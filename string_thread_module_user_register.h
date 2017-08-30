#ifndef STRING_THREAD_MODULE_USER_REGISTER_H
#define STRING_THREAD_MODULE_USER_REGISTER_H

/*! \file string_thread_module_user_register.h
 *  \brief Header for StringThreadModuleUserRegister class
 */


#include "string_thread_message.h"
#include "string_thread.h"
#include "string_thread.h"
#include "string_thread_memory.h"

/*!
 *  \brief Namespace for StringThreadModuleUserRegister class
 */
namespace string_thread_module_user_register
{
	using std::shared_ptr;

	using string_thread_messages::thread_multi_module_message_t;
	using string_thread_messages::thread_multi_module_t;
	using string_thread_memory::StringThreadMemory;

	using string_thread_messages::registration_message_t;

	using string_thread::StringThreadSharedPtr;

	using string_thread_messages::MessageDataNum;
	using string_thread_messages::MessageTypeNum;
	using string_thread_messages::StringThreadID;
	using string_thread_messages::message_t;

	/*!
	 * \brief Module that registers new users
	 */
	class StringThreadModuleUserRegister : public thread_multi_module_t
	{
		public:
			using shared_ptr_t = shared_ptr<StringThreadModuleUserRegister>;

			/*!
			 * 	\brief Constructor
			 */
			StringThreadModuleUserRegister(const StringThreadSharedPtr &Manager);

			/*!
			 * \brief Registers the given module with StringThread. Also links it to the correct senders
			 */
			static void RegisterWithStringUserManager(const shared_ptr_t &RegisterModule,  const StringThreadSharedPtr &Manager);

		private:

			/*!
			 * \brief Memory used by this thread
			 */
			StringThreadMemory *_Memory;

			void HandleMessage(thread_multi_module_message_t &Message);
	};
} // namespace string_thread_module_user_register


#endif // STRING_USER_REGISTER_MODULE_H
