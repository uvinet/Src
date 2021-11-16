	template <typename TValue>
	class TThreadSafeQueue : private std::queue<TValue>
	{
	public:
		/// добавить значение
		void push(TValue& val) {
			const CAutoLock locker(&cs_);
			__super::push(val);
		}
		/// извлечь значение
		bool pop(TValue& val) {
			const CAutoLock locker(&cs_);
			if(!__super::empty()) {
				val = __super::front();
				__super::pop();
				return true;
			}
			return false;
		}
		/// извлечь все значения
		bool popAll(TThreadSafeQueue& queue) {
			const CAutoLock locker(&cs_);
			if(queue.empty() && !__super::empty()) {
				__super::swap(queue);
				return true;
			}
			return false;
		}
		bool front(TValue& val) {
			const CAutoLock locker(&cs_);
			if(!__super::empty()) {
				val = __super::front();
				return true;
			}
			return false;
		}
		int size() { 
			const CAutoLock locker(&cs_);
			return __super::size();
		}
	private:
		CCriticalSection cs_; 	///< объект синхронизации
	};