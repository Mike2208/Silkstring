#ifndef USER_IO_THREAD_H
#define USER_IO_THREAD_H

/*! \file user_io_thread.h
 *  \brief Header for UserIOThread class
 */


#include "silkstring_message.h"
#include "string_user_id.h"
#include "user_io_messages.h"

/*!
 *  \brief Namespace for UserIOThread class
 */
namespace user_io_thread
{
	using std::string;
	using std::shared_ptr;

	using user_io_messages::thread_multi_module_manager_t;
	using user_io_messages::thread_multi_module_t;
	using user_io_messages::thread_multi_module_shared_ptr_t;
	using user_io_messages::thread_multi_module_message_t;
	using user_io_messages::identifier_t;
	using user_io_messages::message_ptr;
	using user_io_messages::message_t;
	using user_io_messages::StartModuleID;

	using user_thread_t = thread_multi_module_manager_t;
	using user_thread_module_t = thread_multi_module_t;
	using user_thread_message_t = thread_multi_module_message_t;

	using string_user_id::StringUserID;

	using user_io_messages::registration_request_message_t;
	using user_io_messages::UserIORegistrationRequestMessageType;
	using user_io_messages::UserIORegistrationModuleID;

	using user_io_messages::UserIOThreadID;
	using user_io_messages::UserIOQueueID;

	using user_io_messages::message_t;
	using user_io_messages::message_ptr;

	/*!
	 * \brief Thread for User I/O
	 */
	class UserIOThread : public user_thread_t
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			UserIOThread();

			//void Register(const module_shared_ptr_t &ModulePtr, const id_vector_t *SendIDs = nullptr);

//			/*!
//			 * \brief Request the user to verify a Registration
//			 * \param NewUserID ID of the counterpart that requested a registration
//			 * \param ExtraInfo Additional information that was sent along
//			 * \param SendModuleID ID of module that sent request
//			 */
//			void RequestRegistration(StringUserID NewUserID, string ExtraInfo, identifier_t SendModuleID);

			//void PushMessage(identifier_t ReceiverID, identifier_t SenderID, user_io_messages::message_t MessageType, message_ptr &&Message);

		private:
	};

	using UserIOThreadSharedPtr = shared_ptr<UserIOThread>;
} // namespace user_io_thread


#endif // USER_IO_THREAD_H
