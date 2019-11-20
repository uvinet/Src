/***********************************************************************************************//**
*     \file SignatureMaker.cpp.
*
*     \brief Implements the signature maker class
****************************************************************************************************/

#include "pch.h"
#include "SignatureMaker.h"
#include "hash.h"
#include <map>

void print_read_progress(int nPercentage) {
	printf("\rFile read progress: %3d%%", nPercentage);
}

/***********************************************************************************************//**
*     \fn void CSignatureMaker::Make(const std::string& sourceFileName, const std::string& outputFileName)
*
*     \brief Makes signature from sourceFileName, writes it to outputFileName. Throws
*     std::exception on error
*
*     \param sourceFileName source file name.
*     \param outputFileName output file name.
****************************************************************************************************/
void CSignatureMaker::Make(const std::string& sourceFileName, const std::string& outputFileName)
{
	if (_stricmp(sourceFileName.c_str(), outputFileName.c_str()) == 0)
		throw std::invalid_argument("Source file name equals to output file name\n");

	// Print hint in case of a small buffer value
	{
		if (m_lBlockSize < allocationGranularity())
			printf("Tip: setting block size more or equal %d bytes will improve velocity\n", allocationGranularity());
	}

	m_hasException = false;

	// open output
	HandleWrapper outputFileHandle = ::CreateFileA(
		outputFileName.c_str(),
		GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!outputFileHandle)
		throw std::system_error(GetLastError(), std::system_category(), "Open output file");

	// prepare source
	MapFileDescriptor sourceDesc;
	sourceDesc.Open(sourceFileName); // throws exception

	LONGLONG llBytesToRead = std::min<LONGLONG>(allocationGranularity(), sourceDesc.FileSize());
	LONGLONG llTotalBytesHasRead = 0;
	
	// Start hash processing threads
	HashThreadPool hashThreadPool;
	std::thread flush_thread;
	try {
		hashThreadPool.Create(this, calcHashThreadsCount(sourceDesc.FileSize()));
		flush_thread = std::thread(flush_thread_func, this, outputFileHandle.Get());
	}
	catch (const std::exception& e)
	{
#if _DEBUG
		printf("%s\n", e.what());
#endif
		throw std::exception("Error while starting processing threads");
	}

	auto check_exception = [&]() {
		if (hasException()) 
		{
			notify_exception_all_queues();
			try {
				hashThreadPool.JoinAll();
				flush_thread.join();
			}
			catch(...) {} // no matter, already got one
			remove_all_jobs();
			std::rethrow_exception(m_exception);
		}
	};

	// job blank
	JobPtr pJob;
	std::uintmax_t current_job_index = 0;

	// start read
	int nPercentage = 0;
	print_read_progress(nPercentage);
	LONGLONG llStartPos = 0;
	while (!hasException() && !m_hash_queue.IsFinished() && llStartPos < sourceDesc.FileSize())
	{
		LPVOID lpData = ::MapViewOfFile(
			sourceDesc.MapHandle(),
			FILE_MAP_READ,
			(DWORD)((llStartPos) >> 32) & 0xffffffff,
			(DWORD)(llStartPos & 0xFFFFFFFF),
			static_cast<SIZE_T>(llBytesToRead)
		);

		if (lpData == nullptr) {
			setException(std::system_error(GetLastError(), std::system_category(), "Mapping source file"));
			break;
		}

		const byte* pBytes = static_cast<byte*>(lpData);

		LONGLONG cRest = llBytesToRead;
		while (cRest) {

			LONGLONG cInsert = cRest;

			try
			{
				// create new job if it not created yet
				if(!pJob) {
					pJob = std::make_shared<Job>(current_job_index++);
					_ASSERT(pJob);
					pJob->data.reserve(m_lBlockSize);				
				}

				const LONGLONG startPos = llBytesToRead - cRest;
				if (pJob->data.size() + cInsert > m_lBlockSize)
					cInsert = m_lBlockSize - pJob->data.size();

				pJob->data.insert(pJob->data.end(), &pBytes[startPos], &pBytes[startPos + cInsert]);

				llTotalBytesHasRead += cInsert;

				_ASSERT(pJob->data.size() <= m_lBlockSize);

				if (pJob->data.size() == m_lBlockSize || llTotalBytesHasRead == sourceDesc.FileSize()) {
					
					// add zeros to the end of block
					/*	DISABLED - reduces speed on large blocks
					if (pJob->data.size() < m_lBlockSize)
						pJob->data.insert(pJob->data.end(), m_lBlockSize - pJob->data.size(), 0);
					*/
				
					// add to queue
					m_hash_queue.Push(std::move(pJob));
				}
			}
			catch (const JobQueue::QueueFinished&)
			{
				// Finish signal
				check_exception(); // exception in other thread?
				break;
			}
			catch (const std::exception& e) {
				setException(e);
				break;
			}

			// show progress
			int curPercentage = (int)(100 * ((double)llTotalBytesHasRead / sourceDesc.FileSize()));
			if (curPercentage - nPercentage >= 1) {
				nPercentage = curPercentage;
				print_read_progress(nPercentage);
			}

			cRest -= cInsert;
		}

		::UnmapViewOfFile(lpData);

		llStartPos += allocationGranularity();

		if (llStartPos < sourceDesc.FileSize() && sourceDesc.FileSize() - llStartPos < allocationGranularity())
			llBytesToRead = (DWORD)(sourceDesc.FileSize() - llStartPos);
	}
	
	printf("\nProcessing...");
	{
		// notify file reading is done and wait hash threads
		m_hash_queue.Finish();
		hashThreadPool.JoinAll();

		// notify hash is done and wait flush thread
		m_flush_queue.Finish();
		flush_thread.join();

		// any exception while while waiting?
		check_exception();
	}
	printf(" Done.\n");
}

/***********************************************************************************************//**
*     \fn void CSignatureMaker::hash_thread_func(CSignatureMaker* psm)
*
*     \brief Hash thread function
*
*     \param psm CSignatureMaker pointer
****************************************************************************************************/
void CSignatureMaker::hash_thread_func(CSignatureMaker* psm)
{
	_ASSERT(psm);

	while (!psm->hasException() && !psm->m_hash_queue.IsFinished())
	{
		try
		{
			// get from hash queue
			JobPtr pJob = psm->m_hash_queue.Pop();
			
			// calc hash
			unsigned int crc_val = CRC::toCRC32(pJob->data);

			// set hash
			pJob->data.clear();
			byte* byte_begin = reinterpret_cast<byte*>(&crc_val);
			std::copy(byte_begin, byte_begin + sizeof(crc_val), std::back_inserter(pJob->data));

			// put to flush queue 
			psm->m_flush_queue.Push(std::move(pJob));
		}
		catch (const JobQueue::QueueFinished&)
		{
			// Finish signal
		}
		catch (const std::exception& e)
		{
			psm->setException(e);
		}
	}
}

/***********************************************************************************************//**
*     \fn void CSignatureMaker::flush_thread_func(CSignatureMaker* psm, const HANDLE hOutputFile)
*
*     \brief Thread function. Flushes result to specified file 
*
*     \param	psm			CSignatureMaker pointer
*     \param	hOutputFile The output file handle
****************************************************************************************************/
void CSignatureMaker::flush_thread_func(CSignatureMaker* psm, const HANDLE hOutputFile)
{
	_ASSERT(psm);
	_ASSERT(hOutputFile);

	// write to file in read order
	std::uintmax_t current_job_index = 0;
	std::map<std::uintmax_t, JobPtr, std::less<std::uintmax_t>> delayed_jobs;
	
	while (!psm->hasException() && !psm->m_flush_queue.IsFinished())
	{
		try
		{
			JobPtr pJob = psm->m_flush_queue.Pop();
			JobPtr pJobToWrite;
			
			if (pJob->index == current_job_index)
				pJobToWrite = std::move(pJob);
			else // delay job
				delayed_jobs.insert({ pJob->index, std::move(pJob) });

			while(pJobToWrite) 
			{
				BOOL bWriteOk = WriteFile(hOutputFile, pJobToWrite->data.data(), static_cast<DWORD>(pJobToWrite->data.size()), NULL, NULL);
				
				 // finally delete job created in main thread
				pJobToWrite.reset();

				if (!bWriteOk)
					throw std::system_error(GetLastError(), std::system_category(), "WriteFile");

				++current_job_index;

				auto it = delayed_jobs.find(current_job_index);
				if (it != delayed_jobs.end()) {
					pJobToWrite = std::move(it->second);
					delayed_jobs.erase(it);
				}
			}
		}
		catch (const JobQueue::QueueFinished&)
		{
			// Finish signal
		}
		catch (const std::exception& e)
		{
			psm->setException(e);
		}
	}
}

/***********************************************************************************************//**
*     \fn void CSignatureMaker::MapFileDescriptor::Open(const std::string fileName)
*
*     \brief Opens specified file, creates file descriptions
*
*     \param fileName Target file name
****************************************************************************************************/
void CSignatureMaker::MapFileDescriptor::Open(const std::string fileName)
{
	_ASSERT(!handle);
	_ASSERT(!mapHandle);
	handle = ::CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!handle)
		throw std::system_error(GetLastError(), std::system_category(), "Open source file");
	
	fileSize = 0ll;
	{
		LARGE_INTEGER size;
		if (!::GetFileSizeEx(handle, &size))
			throw std::system_error(GetLastError(), std::system_category());
		fileSize = size.QuadPart;
		if (fileSize == 0ll)
			throw std::invalid_argument("Source file is empty\n");
	}

	mapHandle = ::CreateFileMappingA(handle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!mapHandle)
		throw std::system_error(GetLastError(), std::system_category(), "Open source file");
}