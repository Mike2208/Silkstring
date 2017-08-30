#ifndef USER_CLI_THREAD_H
#define USER_CLI_THREAD_H

/*! \file user_cli_thread.h
 *  \brief Header for UserCLIThread class
 */


#include "user_cli_thread_messages.h"

/*!
 *  \brief Namespace for UserCLIThread class
 */
namespace user_cli_thread
{
	using namespace user_cli_thread_messages;

	/*!
	 * \brief Thread to manage CLI messages
	 */
	class UserCLIThread : public thread_multi_module_manager_t
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			UserCLIThread();

		private:
	};
} // namespace user_cli_thread


#endif // USER_CLI_THREAD_H
