#ifndef USER_CLI_THREAD_MESSAGES_H
#define USER_CLI_THREAD_MESSAGES_H

/*! \file user_cli_thread_messages.h
 *  \brief Header for user class
 */


#include "silkstring_message.h"
#include "user_io_messages.h"

/*!
 *  \brief Namespace for user class
 */
namespace user_cli_thread_messages
{
	using namespace silkstring_message;

	using namespace user_io_messages;

	static constexpr identifier_t::thread_id_t UserCLIThreadID = DefaultThreadID + 1;
} // namespace user_cli_thread_messages


#endif // USER_CLI_THREAD_MESSAGES_H
