#include "stdafx.h"
#include "LogReader.h"
#include "StateMachine.h"

#include <process.h>

CLogReader::CLogReader()
	: m_hThread(NULL)
	, m_hThreadIsReadyEvent(::CreateEvent(NULL, FALSE, FALSE, NULL))
	, m_hThreadFoundLineEvent(::CreateEvent(NULL, FALSE, FALSE, NULL))
	, m_hFindNextLineEvent(::CreateEvent(NULL, FALSE, FALSE, NULL))
	, m_hStopThreadEvent(::CreateEvent(NULL, FALSE, FALSE, NULL))
	, m_errorCode(EC_NO_ERROR)
	, m_lastFoundLineNum(-1)
{
	InitializeCriticalSection(&m_criticalSection);
}

CLogReader::~CLogReader()
{
	Close();
	DeleteCriticalSection(&m_criticalSection);
}

// открытие файла, false - ошибка
bool CLogReader::Open(const char* pFileName)
{
	if(!IsFileAvailable(pFileName)) {
		SetLastErrorCode(EC_OPEN_FILE);
		return false;
	}

	{
		CAutoLockCS autoLock(&m_criticalSection);
		String2CharArray(pFileName, m_FileName);
	}

	if( !m_hThreadIsReadyEvent.isValid()
		|| !m_hThreadFoundLineEvent.isValid()
		|| !m_hFindNextLineEvent.isValid()
		|| !m_hStopThreadEvent.isValid() )
	{
		SetLastErrorCode(EC_INTERNAL_ERROR_1);
		return false;
	}

	Close();

	m_lastFoundLineNum = -1;
	
	if(m_hThread==NULL) {
		unsigned threadID;
		m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &CLogReader::GetNextLineThread, this, 0, &threadID ));
		if(IsValidHandle(m_hThread)) {
			const HANDLE events[] = {m_hThreadIsReadyEvent.get(), m_hThread};
			DWORD res = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
			if (res == WAIT_OBJECT_0 + 1) /*thread exited, error*/
			{
				CloseHandle(m_hThread);
				m_hThread = NULL;
				return false;
			}
		}
		else {
			SetLastErrorCode(EC_INTERNAL_ERROR_2);
			return false;
		}
	}

	return true;
}

// закрытие 
void CLogReader::Close()
{
	if (IsValidHandle(m_hThread))
	{
		SetEvent(m_hStopThreadEvent.get());
		if(WaitForSingleObject(m_hThread, 500)==WAIT_TIMEOUT) {
			assert(NULL); // something go wrong
			TerminateThread(m_hThread, 1);
		}

		CloseHandle( m_hThread );
		m_hThread = NULL;
	}
}

bool CLogReader::SetFilter(const char *filter)
{
	CAutoLockCS autoLock(&m_criticalSection);
	String2CharArray(filter, m_Filter);
	if(m_Filter.GetSize()<=0)
		SetLastErrorCode(EC_FILTER_NOT_SET);
	return m_Filter.GetSize() > 0;
}

// запрос очередной найденной строки, 
// buf - буфер, bufsize - максимальная длина
// false - конец файла или ошибка
bool CLogReader::GetNextLine(char *buf, const int bufsize, LONGLONG& lineNum)
{
	if (buf == NULL || bufsize == 0) {
		SetLastErrorCode(EC_INCORRECT_BUFFER);
		return false;
	}

	if (m_Filter.GetSize()==0) {
		SetLastErrorCode(EC_FILTER_NOT_SET);
		return false;
	}

	if( !IsValidHandle(m_hThread)
		|| !m_hThreadIsReadyEvent.isValid()
		|| !m_hThreadFoundLineEvent.isValid()
		|| !m_hFindNextLineEvent.isValid()
		|| !m_hStopThreadEvent.isValid() )
	{
		SetLastErrorCode(EC_INTERNAL_ERROR_1);
		return false;
	}

	// зажигаем событие поиска
	SetEvent(m_hFindNextLineEvent.get());

	const HANDLE events[] = { 
		m_hThreadFoundLineEvent.get(), 
		m_hThread
	};
	// ждем результат
	DWORD res = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE);
	if (res == WAIT_OBJECT_0) // нашли
	{
		CAutoLockCS autoLock(&m_criticalSection);
		if(m_lastFoundLine.GetSize()<(UINT)bufsize) {
			strncpy_s(buf, bufsize, (char*)m_lastFoundLine.Data(), _TRUNCATE);
			lineNum = m_lastFoundLineNum;
			return true;
		}
		else {
			SetLastErrorCode(EC_BUFFER_TOO_SMALL);
			return false;
		}
	}

	return false;
}

unsigned __stdcall CLogReader::GetNextLineThread(void* pArguments)
{
	CLogReader* pLogReader = reinterpret_cast<CLogReader*>(pArguments);

	CharArray fileName;
	pLogReader->GetFileName(fileName);

	CHandleWrapper fileHandle(::CreateFileA(
		fileName.Data()
		, GENERIC_READ
		, FILE_SHARE_READ
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL)
		);

	if (!fileHandle.isValid()) {
		pLogReader->SetLastErrorCode(EC_OPEN_FILE);
		return 1;
	}

	const LONGLONG llFileSize = GetSizeOfFile(fileHandle.get());
	if (llFileSize == -1LL) {			
		pLogReader->SetLastErrorCode(EC_GET_FILE_SIZE);
		return 1;
	}
	if (llFileSize == 0LL) {			
		pLogReader->SetLastErrorCode(EC_EMPTY_FILE);
		return 1;
	}

	CHandleWrapper mapHandle(::CreateFileMappingA(
		fileHandle.get()
		, NULL
		, PAGE_READONLY
		, 0
		, 0
		, NULL)
		);

	if (!mapHandle.isValid()) {
		pLogReader->SetLastErrorCode(EC_CREATE_FILE_MAPPING);
		return 1;
	}

	// сообщаем о готовности к поиску
	SetEvent(pLogReader->m_hThreadIsReadyEvent.get());

	const HANDLE events[] = { 
		pLogReader->m_hFindNextLineEvent.get(), 
		pLogReader->m_hStopThreadEvent.get() 
	};

	// ждем продолжения поиска или события выхода
	bool bContinue = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE) == WAIT_OBJECT_0;
	if (bContinue)
	{
		CharArray filter;
		pLogReader->GetFilter(filter);
		CStateMachine stateMachine(reinterpret_cast<const char*>(filter.Data()), false);

		const DWORD dwGranularity = GetSystemGranularity();
		DWORD uBytesToRead = (DWORD)__min(dwGranularity, llFileSize);
		LONGLONG llTotalBytesRead = 0;
		int nLineBufferSize = BUFSIZ; // динамический размер выделенной памяти под pLineBuffer
		int nLineBufferLen = 0; // текущая длина строки pLineBuffer
		char* pLineBuffer = static_cast<char*>(malloc((nLineBufferSize + 1) * sizeof(char)));
		LONGLONG nLine = 0;

		bool bNoError = true;
		LONGLONG llStartPos = 0;
		while(bContinue && llStartPos < llFileSize && bNoError) {

			// получаем порцию данных
			LPVOID lpData = ::MapViewOfFile(
				mapHandle.get()
				, FILE_MAP_READ
				, (DWORD)((llStartPos) >> 32) & 0xffffffff
				, (DWORD)(llStartPos & 0xFFFFFFFF)
				, uBytesToRead
			);

			if(lpData) 
			{
				const char* pChar = static_cast<char*>(lpData);
				bool bEndOfLine = false;
				int nBlockLen = 0;
				const char* pBlockStartAddr = static_cast<char*>(lpData);

				for (DWORD curPos = 0; curPos < uBytesToRead; ++curPos, ++pChar, ++llTotalBytesRead)
				{
					if (*pChar != '\r' && *pChar != '\n') {
						++nBlockLen;
						if (pBlockStartAddr == 0)
							pBlockStartAddr = pChar;
					}

					bEndOfLine = *pChar == '\n' || llTotalBytesRead + 1 == llFileSize;
					if (bEndOfLine || (curPos+1)==uBytesToRead/*конец данных в MapView*/)
					{
						// копируем блок длины nBlockLen из отображаемого представления в наш строковый буфер
						if (nBlockLen>0)
						{
							// если в pLineBuffer недостаточно памяти, увеличиваем ее
							if (nLineBufferSize < nBlockLen) {
								nLineBufferSize += nBlockLen * 2;
								char* newBuffer = static_cast<char*>(realloc(pLineBuffer, (nLineBufferSize + 1) * sizeof(char)));
								if (newBuffer == NULL)
								{
									pLogReader->SetLastErrorCode(EC_LINEBUFFER_REALLOC_ERROR);
									bNoError = false;
									break;
								}
								pLineBuffer = newBuffer;
							}
							memcpy(pLineBuffer + nLineBufferLen, pBlockStartAddr, nBlockLen);
							nLineBufferLen += nBlockLen;
							pLineBuffer[nLineBufferLen] = 0;
							nBlockLen = 0;
						}

						// конец строки - приступаем к поиску
						if (bEndOfLine) 
						{
							if(stateMachine.Match(pLineBuffer))
							{
								pLogReader->SetLastFoundLine(pLineBuffer, nLine);
								SetEvent(pLogReader->m_hThreadFoundLineEvent.get());

								// ждем продолжения поиска или события выхода
								bContinue = WaitForMultipleObjects(sizeof(events) / sizeof(HANDLE), events, FALSE, INFINITE) == WAIT_OBJECT_0;
								if (!bContinue)
									break;
							}

							// зачищаем буферные данные
							pLineBuffer[0] = 0;
							nLineBufferLen = 0;
							pBlockStartAddr = 0;
							
							++nLine;
						}
					}
				} // for (DWORD curPos ...

				::UnmapViewOfFile(lpData);
			}
			else
			{
				pLogReader->SetLastErrorCode(EC_GET_MAP_VIEW_ERROR);
				bNoError = false;
				break;
			}

			llStartPos += dwGranularity;

			if (llStartPos < llFileSize && llFileSize - llStartPos < dwGranularity)
				uBytesToRead = (DWORD)(llFileSize - llStartPos);

		} // while(bContinue ...

		free(pLineBuffer);
	}

	return 0;
}