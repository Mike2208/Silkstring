#ifndef THREAD_FUNCTION_H
#define THREAD_FUNCTION_H

/*! \file thread_function.h
 *  \brief Header for ThreadFunction class
 */

#include <future>
#include <mutex>

/*!
 *  \brief Namespace for ThreadFunction class
 */
namespace thread_function
{
	using std::future;
	using std::async;
	using std::mutex;

	/*!
	 * \brief The ThreadFunction class
	 * Warning: Cannot take a reference as argument. This is because ThreadFunction uses async to launch the thread, which doesn't take references. Use pointers instead
	 */
	template<class Function, class Return, class ...Args>
	class ThreadFunction : public future<Return>
	{
			using future_t = future<Return>;
		public:

			template<class ...FcnArgs>
			ThreadFunction(Function *ThreadFcn, FcnArgs &&...Parameters)
				: future_t(async(ThreadFcn, std::forward<FcnArgs>(Parameters)...)),
				  _ThreadFcn(ThreadFcn)
			{
				//static_cast<future_t&>(*this) = async(this->_ThreadFcn, std::forward<Args>(Parameters)...);
			}

			ThreadFunction(const ThreadFunction &S) = delete;
			ThreadFunction(ThreadFunction &&S) = default;

			ThreadFunction &operator=(const ThreadFunction &S) = delete;
			ThreadFunction &operator=(ThreadFunction &&S) = default;

			template<class ...FcnArgs>
			void operator()(FcnArgs &&...Parameters)
			{
				// Finish any previous threads
				static_cast<future_t&>(*this).wait();

				static_cast<future_t&>(*this) = async(this->_ThreadFcn, std::forward<FcnArgs>(Parameters)...);
			}

			bool Finished() const
			{
				return static_cast<future_t&>(*this).valid();
			}

			void Wait() const
			{
				static_cast<const future_t&>(*this).wait();
			}

			template<class FcnReturn = Return>
			typename std::enable_if<!std::is_same<Return, void>::value && std::is_same<FcnReturn, Return>::value, Return>::type &&GetResult()
			{
				static_assert(std::is_convertible<FcnReturn, Return>::value, "ERROR ThreadFunction::GetResult(): Parameter must match return function\n");

				// Retrieve value
				return std::move(static_cast<future_t&>(*this).get());
			}

		private:

			Function *const _ThreadFcn;
	};


//	template<class Function, class ...Args>
//	class ThreadFunction<Function, void, Args...> : public thread
//	{
//		public:

//			ThreadFunction(Function *ThreadFcn, Args &&...Parameters)
//				: _ThreadFcn(ThreadFcn)
//			{
//				static_cast<thread&>(*this) = thread(ThreadFunction::ThreadFcn, this->_ThreadFcn, std::ref(this->_Running), std::forward<Args>(Parameters)...);
//			}

//			void operator()(Args &&...Parameters)
//			{
//				// Finish any previous threads
//				static_cast<thread&>(*this).join();

//				static_cast<thread&>(*this) = thread(ThreadFunction::ThreadFcn, this->_ThreadFcn, std::ref(this->_Running), std::forward<Args>(Parameters)...);
//			}

//		private:

//			Function *const _ThreadFcn;

//			/*!
//			 * \brief Mutex that is locked when the thread is running
//			 */
//			mutex _Running;

//			static void ThreadFcn(Function *ThreadFcn, mutex &Lock, Args &&...Parameters)
//			{
//				Lock.lock();

//				ThreadFcn(std::forward<Args>(Parameters)...);

//				Lock.unlock();
//			}
//	};

} // namespace thread_function


#endif // THREAD_FUNCTION_H
