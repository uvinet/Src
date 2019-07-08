/**
Реализация CLogReader. Класс для чтения лог-файлов и поиска строк в них
*/

#include <windows.h>
#include <assert.h>

#include "helpers.h"

#pragma once

typedef enum {
	EC_NO_ERROR,
	EC_OPEN_FILE,
	EC_GET_FILE_SIZE,
	EC_EMPTY_FILE,
	EC_CREATE_FILE_MAPPING,
	EC_GET_MAP_VIEW_ERROR,
	EC_LINEBUFFER_REALLOC_ERROR,
	EC_FILTER_NOT_SET,
	EC_INCORRECT_BUFFER,
	EC_BUFFER_TOO_SMALL,
	EC_INTERNAL_ERROR_1, // action events not created
	EC_INTERNAL_ERROR_2, // _beginthreadex error
} ERROR_CODES;

// Вывод текстового сообщения об ошибке по ее коду
inline void PrintError(ERROR_CODES error)
{
	switch(error)
	{
	case EC_OPEN_FILE:
		printf_s("Can not open specified file, check file name\n");
		break;
	case EC_GET_FILE_SIZE:
		printf_s("Error getting file size\n");
		break;

	case EC_EMPTY_FILE:
		printf_s("Specified file is empty\n");
		break;

	case EC_CREATE_FILE_MAPPING:
		printf_s("Could not create map for a specified file.\n");
		break;

	case EC_GET_MAP_VIEW_ERROR:
		printf_s("Could not create view for a specified file.\n");
		break;
	case EC_LINEBUFFER_REALLOC_ERROR:
		printf_s("Could not allocate memor5y for line buffer (out of memory)\n");
		break;

	case EC_BUFFER_TOO_SMALL:
		printf_s("Buffer too small\n");
		break;

	case EC_INCORRECT_BUFFER:
		printf_s("Incorrect buffer string\n");
		break;

	case EC_FILTER_NOT_SET:
		printf_s("Filter not set\n");
		break;

	default:
		printf_s("No error or unknown error\n");
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// CLogReader 
// Класс, предназначенный для чтения текстовых лог-файлов и поиска строк по шаблону с использованием спец. символов:
// '*' - последовательность любых символов неограниченной длины
// '? - один любой символ
// Принцип работы:
// Open(...) создает рабочий поток GetNextLineThread, который после удачного открытия файла переходит в ожидание события очередного поиска.
// SetFilter задает строку поиска
// GetNextLine зажигает событие очередного поиска m_hFindNextLineEvent и переходит в ожидание результата поиска.
// Close() зажигает событие выхода из потока и закрытие файла (вызывается из деструктора)
// Для контроля доступа к общим ресурсам используется критическая секция
#if (_MSC_VER < 1900)
class CLogReader : public virtual CInheritLocker // чтобы исключить возможность создавать производные классы, используем виртуальное наследование
#else
class CLogReader final // начиная с C++11 можно так
#endif //(_MSC_VER < 1900)
{
public:
	CLogReader();
	~CLogReader();

	// открытие файла, false - ошибка
	bool Open(const char* pFileName);

	// закрытие файла
	void Close();

	// установка фильтра строк, false - ошибка
	bool SetFilter(const char *filter);

	// получение следующей строки, удовлетворяющей условю поиска 
	// buf - буфер, bufsize - максимальная длина буфера
	// false - конец файла или ошибка
	bool GetNextLine(char *buf, const int bufsize, LONGLONG& lineNum);

	// получение кода последней ошибки
	ERROR_CODES GetLastErrorCode() { 
		CAutoLockCS autoLock(&m_criticalSection);
		return m_errorCode;
	}

private:
	CharArray m_FileName;
	CharArray m_Filter;
	CharArray m_lastFoundLine;
	LONGLONG m_lastFoundLineNum;
	ERROR_CODES m_errorCode;
	CRITICAL_SECTION m_criticalSection;

	HANDLE m_hThread; // хэндл рабочего потока
	CHandleWrapper
		m_hThreadIsReadyEvent, // Событие от потока: поток создан и готов к поиску)
		m_hThreadFoundLineEvent, // Событие от потока: найдено совпадение
		m_hFindNextLineEvent, // событие устанавливается в основном потоке. Запустить очередной поиск
		m_hStopThreadEvent; // событие устанавливается в основном потоке. Завершить рабочий поток

	void GetFilter(CharArray& ca) {
		CAutoLockCS autoLock(&m_criticalSection);
		ca = m_Filter;
	}

	void GetFileName(CharArray& ca){
		CAutoLockCS autoLock(&m_criticalSection);
		ca = m_FileName;
	}

	void SetLastFoundLine(const char *pLine, LONGLONG llLineNum) { 
		CAutoLockCS autoLock(&m_criticalSection);
		String2CharArray(pLine, m_lastFoundLine);
		m_lastFoundLineNum = llLineNum;
	}

	void SetLastErrorCode(ERROR_CODES err) { 
		CAutoLockCS autoLock(&m_criticalSection);
		m_errorCode = err; 
	}

	// потоковая функция поиска
	static unsigned __stdcall GetNextLineThread(void* pArguments);
};
// CLogReader 
//////////////////////////////////////////////////////////////////////////


class CLogReader2 : public CLogReader
{
public:

};