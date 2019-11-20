/***********************************************************************************************//**
*     \file Queue.h.
*
*     \brief Queues classes
****************************************************************************************************/

#pragma once

/***********************************************************************************************//**
*     \class IQueue
*
*     \brief Describes IQueue base interface. Abstract Class
*
*     \tparam TTaskType Type of the task.
****************************************************************************************************/
template <typename TTaskType>
class IQueue
{
public:
	virtual void Push(TTaskType task) = 0;
	virtual TTaskType Pop() = 0;
	virtual bool IsEmpty() = 0;

	virtual ~IQueue() {}
};

/***********************************************************************************************//**
*     \class CTaskQueue
*
*     \brief Simple task queue template. IQueue interface derived
*
*     \tparam TTask Type of the task.
****************************************************************************************************/
template <typename TTask>
class CTaskQueue : private std::queue<TTask>, public IQueue<TTask>
{
	using TContainer = typename std::queue<TTask>;
	enum { INFINITE_SIZE = -1 };
public:
	CTaskQueue(const std::queue<TTask>::size_type maxSize = INFINITE_SIZE) : m_maxSize(maxSize)
	{}

	~CTaskQueue() override {}

	void Push(TTask task) override {
		if (m_maxSize != INFINITE_SIZE && TContainer::size() == m_maxSize)
			throw std::range_error("Queue limit exceeded");

		TContainer::push(task);
	}

	TTask Pop() override {
		if (IsEmpty())
			throw std::range_error("Queue is empty");

		TTask task = TContainer::front();
		TContainer::pop();
		return task;
	}

	bool IsEmpty() override {
		return TContainer::empty();
	}

private:
	const TContainer::size_type m_maxSize;
};

/***********************************************************************************************//**
*     \class CThreadSafeQueue
*
*     \brief Thread safe queue (delegates IQueue derived class)
*
*     \tparam TTask Type of the task.
****************************************************************************************************/
template <typename TTask>
class CThreadSafeQueue : public IQueue<TTask>
{
	using Queue = typename IQueue<TTask>;
public:
	CThreadSafeQueue(Queue* pQueue) : m_pQueue(pQueue)
	{
		_ASSERT(pQueue);
		if (!pQueue)
			throw std::invalid_argument("CThreadSafeQueue: invalid pQueue pointer");
	}

	CThreadSafeQueue() = delete;

	~CThreadSafeQueue()
	{}

	struct QueueFinished : public std::range_error {
		QueueFinished() : std::range_error("Queue finished") {}
	};

	void Push(TTask pJob) override
	{
		if(IsFinished())
			throw QueueFinished();

		bool bPushed = false;
		while (!bPushed) 
		{
			std::unique_lock<std::mutex> lock(m_guard_mutex);
			try	{
				m_pQueue->Push(pJob);
				bPushed = true;
				m_cv_push.notify_all();
			}
			catch (const std::range_error&) {		
				if (IsFinished())
					throw QueueFinished();
				m_cv_pop.wait(lock); // wait for vacancy in queue
			}
		}
	}

	TTask Pop() override
	{
		if (IsFinished())
			throw QueueFinished();

		bool bPopped = false;
		while (!bPopped || IsFinished())
		{
			std::unique_lock<std::mutex> lock(m_guard_mutex);
			try {
				TTask task = m_pQueue->Pop();
				bPopped = true;
				m_cv_pop.notify_all();
				return task;
			}
			catch (const std::range_error&) {
				if (IsFinished())
					throw QueueFinished();
				m_cv_push.wait(lock);  // wait for new job in queue
			}
		}
		
		return TTask(); // stub
	}

	bool IsEmpty() override
	{
		std::lock_guard<std::mutex> lock(m_guard_mutex);
		return m_pQueue->IsEmpty();
	}

	void Finish()
	{
		if(!IsFinished()) 
		{
			{
				// empty queue
				std::unique_lock<std::mutex> lock(m_guard_mutex);
				while(!m_pQueue->IsEmpty()) {
					// emulate push to cheer up consumers
					m_cv_push.notify_all();
					m_cv_pop.wait(lock);
					if(IsFinished())
						break;
				}
			}

			if(!IsFinished()) // could change on emergency stop
			{
				// signalize finished
				std::lock_guard<std::mutex> lock(m_guard_mutex);
				m_isFinished = true;
				m_cv_push.notify_all();
			}
		}
	}

	/// Set finished forced and notify waiters (used for emergency queue stop)
	void FinishForced()
	{
		m_isFinished = true;
		m_cv_pop.notify_all();
		m_cv_push.notify_all();
	}

	bool IsFinished() 
	{
		// no need to lock
		return m_isFinished;
	}

	Queue* GetQueue() { return m_pQueue.get(); }

private:
	std::unique_ptr<Queue> m_pQueue;
	std::mutex m_guard_mutex;
	std::condition_variable m_cv_pop;
	std::condition_variable m_cv_push;
	std::atomic<bool> m_isFinished = false;
};