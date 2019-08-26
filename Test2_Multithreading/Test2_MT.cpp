// Test2_MT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <atomic>

// количество потоков, дающих доступ
const int maxGrantPermissionThreads = 3;
// количество потоков, принимающих доступ
const int maxWaitForPermissionThreads = 3;
// количество циклов в функциях, дающих доступ
const int maxCyclesNum = 10;

// та самая
void UsePermission()
{
	static std::atomic<int> callsCounter = 0;
	callsCounter++;
	printf("->UsePermission is running (call num %d)\n", callsCounter.load());
	Sleep(2000);
}

void sleepRandomTime()
{
	Sleep(rand() % 4001 + 1000);
}

// Singleton
class CPermission
{
public:

	static CPermission& GetInstance()
	{
		static CPermission instance = CPermission();
		return instance;
	}

	~CPermission()
	{
		DeleteCriticalSection(&m_cs);
	}

	void Permit()
	{
		CLocker lock(&m_cs);
		if(!m_bPermitted)
			m_bPermitted = true;
	}

	bool TryTake()
	{
		CLocker lock(&m_cs);
		if (m_bPermitted && !m_isLocked) {
			m_isLocked = true;
			return true;
		}
		return false;
	}

	void Reset()
	{
		CLocker lock(&m_cs);
		if(m_isLocked)
			m_bPermitted = m_isLocked = false;
	}

	void Finish()
	{
		CLocker lock(&m_cs);
		if (!m_bFinished) {
			printf("Finishing...\n");
			m_bFinished = true;
		}
	}

	bool IsFinished()
	{
		CLocker lock(&m_cs);
		return m_bFinished;
	}

protected:
	class CLocker
	{
	private:
		CRITICAL_SECTION* const m_pCS;

	public:
		CLocker(CRITICAL_SECTION* pcs) : m_pCS(pcs) { EnterCriticalSection(m_pCS); }
		~CLocker()									{ LeaveCriticalSection(m_pCS); }
	};

private:
	CPermission()
	{
		InitializeCriticalSection(&m_cs);
	}
	
	CRITICAL_SECTION m_cs;
	bool m_bPermitted = false;
	bool m_isLocked = false;
	bool m_bFinished = false;
};

class CThreadCollector
{
public:
	~CThreadCollector()
	{
		close();
	}

	void Add(HANDLE hThread)
	{
		m_threadArr.push_back(hThread);
	}

private:
	std::vector<HANDLE> m_threadArr;

	void close()
	{
		for(HANDLE h : m_threadArr) {
			WaitForSingleObject(h, INFINITE);
			CloseHandle(h);
		}
		m_threadArr.clear();
	}
};

// выдает разрешение
DWORD WINAPI GrantPermissionProc(const LPVOID lpParam)
{
	// для отладки 
	static std::atomic<int> st_nameCounter = 1;
	const int nameCounter = st_nameCounter.fetch_add(1);

	printf("GrantPermissionProc %d started\n", nameCounter);

	CPermission* pPermission = static_cast<CPermission*>(lpParam);

	// для теста - "бесконечный цикл" ограничен maxCyclesNum
	for(int nCycle = 0; nCycle < maxCyclesNum; ++nCycle) 
	{
		printf("GrantPermissionProc %d is sleeping. Cycle %d\n", nameCounter, nCycle);
		sleepRandomTime();
		pPermission->Permit();
	}

	// завершаем ждущие потоки
	pPermission->Finish();

	printf("GrantPermissionProc %d exited\n", nameCounter);
	ExitThread(0);
}

// ждет разрешение, вызывает UsePermission
DWORD WINAPI WaitForPermissionProc(const LPVOID lpParam)
{
	// для отладки
	static std::atomic<int> st_nameCounter = 1;
	const int nameCounter = st_nameCounter.fetch_add(1);

	printf("WaitForPermissionProc %d started\n", nameCounter);

	CPermission* pPermission =  static_cast<CPermission*>(lpParam);

	while (!pPermission->IsFinished()) {
		printf("WaitForPermissionProc %d is sleeping\n", nameCounter);
		sleepRandomTime();
		if (pPermission->TryTake()) {
			printf("WaitForPermissionProc %d lock\n", nameCounter);
			UsePermission();
			printf("WaitForPermissionProc %d unlock\n", nameCounter);
			pPermission->Reset();
		}
	}

	printf("WaitForPermissionProc %d exited\n", nameCounter);
	ExitThread(0);
}

int main()
{
	{
		CThreadCollector threadCollector;

		for (int nThreads = 0; nThreads < maxGrantPermissionThreads; ++nThreads)
			threadCollector.Add(
				CreateThread(NULL, 0, &GrantPermissionProc, &CPermission::GetInstance(), 0, NULL)
			);

		for (int nThreads = 0; nThreads < maxWaitForPermissionThreads; ++nThreads)
			threadCollector.Add(
				CreateThread(NULL, 0, &WaitForPermissionProc, &CPermission::GetInstance(), 0, NULL)
			);
	}
	
	printf("Press any key...");
	_getch();

    return 0;
}