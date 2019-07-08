/**
Вспомогательные классы и функции
*/

#pragma once

inline bool IsValidHandle(HANDLE hHandle)
{
	return hHandle != NULL && hHandle != INVALID_HANDLE_VALUE;
}

// RAII-обертка над дескриптором (HANDLE)
class CHandleWrapper
{
public:
	explicit CHandleWrapper(HANDLE h) : m_hHandle(h)
	{}

	~CHandleWrapper()
	{
		close();
	}

	HANDLE get() const { return m_hHandle; }

	bool isValid() const { return IsValidHandle(m_hHandle); }

	void close()
	{
		if (isValid()) {
			::CloseHandle(m_hHandle);
			m_hHandle = NULL;
		}
	}

#if (_MSC_VER >= 1900)
	// non copyable
	CHandleWrapper(const CHandleWrapper&) = delete;
	void operator=(const CHandleWrapper&) = delete;
#endif

private:
	HANDLE m_hHandle;
};

inline bool IsFileAvailable(LPCSTR lpFileName)
{
	if(lpFileName!=NULL)
	{
		return CHandleWrapper(
			::CreateFileA(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, NULL)
			).isValid();
	}

	return false;
}

// RAII-обертка для критической секции
class CAutoLockCS
{
private:
	LPCRITICAL_SECTION const m_lpCS;

public:
	CAutoLockCS(LPCRITICAL_SECTION lpcs) : m_lpCS(lpcs) { 
		if(m_lpCS)
			EnterCriticalSection(m_lpCS);
	}
	~CAutoLockCS() { 
		if(m_lpCS)
			LeaveCriticalSection(m_lpCS);
	}
};

#if (_MSC_VER < 1900)
// Класс, запрещающий наследование в производных классах
// Использование: 
// применить виртуальное наследование
// добавить класс в friend
class CInheritLocker
{
private:
	CInheritLocker() {}

	friend class CLogReader;
};
#endif //(_MSC_VER < 1900)

// Шаблонный класс простейшего динамического массива
template<class TElement>
class CSimpleDynArray
{
public:
	CSimpleDynArray() 
		: m_pArray(NULL)
		, m_size(0)
		, m_allocatedSize(0)
	{
	}

	CSimpleDynArray(const CSimpleDynArray<TElement>& src) 
	{
		Copy(src, *this);
	}

	virtual ~CSimpleDynArray()
	{
		if (m_pArray) {
			free(m_pArray);
			m_pArray = NULL;
		}
	}

	bool Add(const TElement &element)
	{
		if (m_allocatedSize == 0) // lazy initialization
			Init();

		assert(m_allocatedSize);

		if ((m_size + 1) > m_allocatedSize)
		{
			UINT newAllocatedSize = m_allocatedSize * 2;
			TElement* newArray = (TElement*)realloc(m_pArray, sizeof(TElement)*newAllocatedSize);
			if (newArray == NULL)
			{
				printf_s("CSimpleDynArray::Add realloc error\n");
				return false;
			}

			m_allocatedSize = newAllocatedSize;
			m_pArray = newArray;
		}

		m_size++;
		m_pArray[m_size - 1] = element;
		return true;
	}

	UINT GetSize() const { return m_size; }

	void Clear() { Init(); }
	void ClearFast() { m_size = 0; }

	const TElement* const Data() const { return m_pArray; }

	static void Copy(const CSimpleDynArray<TElement>& src, CSimpleDynArray<TElement>& dst)
	{
		dst.Clear();
		for(UINT n=0; n<src.GetSize(); ++n)
			dst.Add(src[n]);
	}

	CSimpleDynArray& operator=(const CSimpleDynArray& src) { Copy(src, *this); return *this; }
	const TElement& operator [] (UINT n) const { assert(n<GetSize()); return m_pArray[n]; }

private:
	enum { INITIAL_SIZE = MAX_PATH };
	TElement* m_pArray;
	UINT m_size;
	UINT m_allocatedSize;

	void Init()
	{
		m_size = 0;
		m_allocatedSize = INITIAL_SIZE;
		if (m_pArray != NULL) {
			free(m_pArray);
			m_pArray = NULL;
		}

		TElement* newArray = (TElement*)calloc(m_allocatedSize, sizeof(TElement));
		if (newArray != NULL)
			m_pArray = newArray;
	}
};

// Символьный динамический массив
typedef CSimpleDynArray<char> CharArray;

// преобразователь из строки в символьный массив
inline void String2CharArray(const char* pStr, CharArray& rCharArray)
{
	assert(pStr);
	if(pStr)
	{
		rCharArray.Clear();
		const char* pChar = pStr;
		for (;; ++pChar) {
			rCharArray.Add(*pChar);
			if(*pChar=='\0')
				break;
		}
	}
}

// преобразователь символьного массива в строку
// возвращаемый указатель на строку должным быть освобожден функцией free()
inline char* CharArray2String(CharArray& rCharArray)
{
	char* pRetStr = NULL;
	const UINT size = rCharArray.GetSize();
	if(size>0) {
		pRetStr = static_cast<char*>(malloc(size));
		memcpy_s(pRetStr, size, rCharArray.Data(), size);
	}
	return pRetStr;
}

// получение размера файла по его дескритеру 
inline LONGLONG GetSizeOfFile(HANDLE hFile)
{
	LARGE_INTEGER fileSize = {};
	if(hFile && GetFileSizeEx(hFile, &fileSize)) 
		return fileSize.QuadPart;

	return -1LL;
}

// гранулярность памяти в системе
inline DWORD GetSystemGranularity()
{
	static DWORD dwGranularity = 0;
	if(dwGranularity==0) {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		dwGranularity = si.dwAllocationGranularity;
	}
	return dwGranularity;
}
// Вспомогательные классы и функции
//////////////////////////////////////////////////////////////////////////