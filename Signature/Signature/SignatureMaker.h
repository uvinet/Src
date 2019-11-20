/***********************************************************************************************//**
*     \file SignatureMaker.h.
*
*     \brief Declares the signature maker class
****************************************************************************************************/

#pragma once

#include "pch.h"

/***********************************************************************************************//**
*     \struct HandleWrapper
*
*     \brief HANDLE RAII-wrapper
****************************************************************************************************/
struct HandleWrapper
{
	HandleWrapper(){}

	HandleWrapper(HANDLE h) : h_(h) {}

	~HandleWrapper() {
		if (IsValid())
			::CloseHandle(h_);
	}

	HANDLE Get() const { return h_; }

	bool IsValid() const { return h_ != NULL && h_ != INVALID_HANDLE_VALUE; }

	operator HANDLE () const
	{
		return Get();
	}

	void Attach(const HANDLE h) {
		_ASSERT(!h_);
		h_ = h;
	}

	void operator=(const HANDLE h) { Attach(h); }
	explicit operator bool() const { return IsValid(); }

	// noncopyable
	HandleWrapper(const HandleWrapper&) = delete;
	void operator=(const HandleWrapper&) = delete;

private:
	HANDLE h_ = NULL;
};

inline DWORD allocationGranularity()
{
	static DWORD granularity = [&] {
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		_ASSERT(si.dwAllocationGranularity);
		return si.dwAllocationGranularity;
	}();

	return granularity;
}

/***********************************************************************************************//**
*     \class CSignatureMaker
*
*     \brief A signature maker class
****************************************************************************************************/
class CSignatureMaker
{
public:
	CSignatureMaker(unsigned long lBlockSize = DefaultBlockSize())
		: m_lBlockSize(lBlockSize)
		, m_hash_queue(new CTaskQueue<JobPtr>(maxHashThreadsNumber() * 10)) // max queue size
		, m_flush_queue(new CTaskQueue<JobPtr>)
	{
		if (lBlockSize == 0 || lBlockSize > MaxBlockSize()) {
			const std::string msg = "Invalid block size (max block size " + std::to_string(MaxBlockSize()) + " bytes)";
			throw std::invalid_argument(msg);
		}
	}

	~CSignatureMaker()
	{}

	// make signature file
	void Make(const std::string& sourceFileName, const std::string& outputFileName);

	// max reasonable size
	unsigned long static MaxBlockSize()
	{
		static unsigned long maxSize = static_cast<unsigned long>(std::vector<byte>().max_size() / 4);
		return maxSize;
	}

	unsigned long static DefaultBlockSize()
	{
		static unsigned long defaultSize = 1024 * 1024; // 1Mb
		return defaultSize;
	}

private:
	const unsigned long m_lBlockSize;

	// simple job struct
	struct Job
	{
		const std::uintmax_t index;
		std::vector<byte> data;

		Job(std::uintmax_t i) : index(i) {}
	};

	// thread pool class for hash_thread_func
	class HashThreadPool
	{
	public:
		~HashThreadPool() {
			_ASSERT(threads_vector.empty());
		}

		void Create(CSignatureMaker* psm, const int cThreads) {
			_ASSERT(cThreads);
			for (int nThread = 0; nThread < cThreads; nThread++)
				threads_vector.emplace_back(hash_thread_func, psm);
		}

		void JoinAll() {
			for (auto& t : threads_vector)
				t.join();
			threads_vector.clear();
		}

	private:
		std::vector<std::thread> threads_vector;
	};

	// Source file wrapper. Opens specified file, creates file mapping, holds handles while alive
	class MapFileDescriptor
	{
	public:
		MapFileDescriptor() {}

		void Open(const std::string fileName);

		HANDLE Handle()		const { return handle.Get(); }
		HANDLE MapHandle()	const { return mapHandle.Get(); }
		LONGLONG FileSize()	const { return fileSize; }

	private:
		HandleWrapper handle;
		HandleWrapper mapHandle;
		LONGLONG fileSize;
	};

	using JobPtr = std::shared_ptr<Job>;
	using JobQueue = typename CThreadSafeQueue<JobPtr>;

// members
private:
	JobQueue m_hash_queue;
	JobQueue m_flush_queue;
	std::atomic<bool> m_hasException = false;
	std::exception_ptr m_exception;
	std::mutex m_exception_mutex;

	bool hasException() { return m_hasException; }

	// sets exception (used for emergency queue stop)
	void setException(const std::exception& e)
	{
		std::lock_guard<std::mutex> lock(m_exception_mutex);
		if (!m_hasException) {
			m_hasException = true;
			m_exception = std::make_exception_ptr(e);
			notify_exception_all_queues();
		}
	}

	// notifies exception to all queue threads (used for emergency queue stop)
	void notify_exception_all_queues()
	{
		_ASSERT(hasException());
		auto queues = { &m_hash_queue, &m_flush_queue };
		for (auto q : queues)
			q->FinishForced();
	}
	
	// removes all from queue (used for emergency queue stop)
	void remove_all_jobs()
	{
		_ASSERT(hasException());
		auto queues = { &m_hash_queue, &m_flush_queue };
		for (auto q : queues)
			while (!q->GetQueue()->IsEmpty())
				q->GetQueue()->Pop();
	}

	// returns max number of concurrent threads
	static unsigned int maxHashThreadsNumber() {
		static unsigned int number = std::max<unsigned int>(std::thread::hardware_concurrency() - 2/*main + flush thread*/, 1u);
		return number;
	}

	// adjusts threads count in thread pool by specified file size
	unsigned int calcHashThreadsCount(const LONGLONG fileSize) const {
		return std::min<unsigned int>(static_cast<unsigned int>(fileSize / m_lBlockSize + 1), maxHashThreadsNumber());
	}

	// calc hash thread function
	static void hash_thread_func(CSignatureMaker* psm);
	// write result thread function
	static void flush_thread_func(CSignatureMaker* psm, const HANDLE hOutputFile);
};