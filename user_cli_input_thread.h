#ifndef USER_CLI_INPUT_THREAD_H
#define USER_CLI_INPUT_THREAD_H

/*! \file user_cli_input_thread.h
 *  \brief Header for UserCLIInputThread class
 */


#include "thread_function.h"
#include "global_message_queue_thread.h"

#include "user_cli_thread_module_parser.h"

/*!
 *  \brief Namespace for UserCLIInputThread class
 */
namespace user_cli_input_thread
{
	using std::string;
	using std::atomic;

	using user_cli_thread_module_parser::cin;

	using user_cli_thread_module_parser::UserIOCLIReadModuleID;

	using namespace user_cli_thread_messages;

	using thread_function::ThreadFunction;

	using global_message_queue_thread::GlobalMessageQueueThread;

	class UserCLIInputThread;

	using read_fcn_t = void(GlobalMessageQueueThread *, UserCLIInputThread *);
	using thread_fcn_t = ThreadFunction<read_fcn_t, void, GlobalMessageQueueThread *, UserCLIInputThread *>;

	static constexpr identifier_t::queue_id_t	UserCLIInputQueueID = StartQueueID + 90;
	static constexpr identifier_t::module_id_t	UserCLIInputModuleID = StartModuleID;
	static constexpr identifier_t::thread_id_t	UserCLIInputThreadID = DefaultThreadID;

	/*!
	 * \brief Reads CLI input in a separate thread so that main thread isn't blocked
	 */
	class UserCLIInputThread : protected thread_fcn_t
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			UserCLIInputThread(GlobalMessageQueueThread *GlobalMessageThread);

			~UserCLIInputThread();

			/*!
			 * \brief Sets a flag to stop the thread
			 */
			void StopThread();

			/*!
			 * \brief Reads a line and sends it as message when complete
			 * \param Queue Queue that the input will be sent to
			 */
			static void ReadAndProcessInput(GlobalMessageQueueThread *Queue);

		private:

			/*!
			 * \brief All read input is sent to this thread
			 */
			GlobalMessageQueueThread *_GlobalMessageThread;

			atomic<bool> _StopThread;

			/*!
			 * \brief Function for reading a line and sending it as message when complete
			 * \param Queue Queue that the input will be sent to
			 * \param Thread Pointer to this class
			 */
			static void ReadFcn(GlobalMessageQueueThread *Queue, UserCLIInputThread *Thread);
	};
} // namespace user_cli_input_thread


#endif // USER_CLI_INPUT_THREAD_H
