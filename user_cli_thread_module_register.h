#ifndef USER_CLI_THREAD_MODULE_REGISTER_H
#define USER_CLI_THREAD_MODULE_REGISTER_H

/*! \file user_cli_register.h
 *  \brief Header for UserCLIRegister class
 */


#include "user_cli_thread_messages.h"
#include "vector_t.h"

/*!
 *  \brief Namespace for UserCLIRegister class
 */
namespace user_cli_thread_module_register
{
	using std::array;
	using std::shared_ptr;

	using namespace user_cli_thread_messages;


	struct registration_request_data_t : public registration_request_message_t
	{
		using id_vector_t = silkstring_message::id_vector_t;

		/*!
		 * \brief Modules that issued the request
		 */
		id_vector_t ReturnModules;

		registration_request_data_t(const registration_request_message_t &Message, identifier_t ReturnModule);
	};

	/*!
	 * \brief The UserCLIRegister class
	 */
	class UserCLIThreadModuleRegister : public thread_multi_module_t
	{
		public:
			using registration_request_vector_t = vector_t::vector_type<registration_request_data_t>;

			/*!
			 *	\brief Constructor
			 */
			UserCLIThreadModuleRegister();

			const registration_request_vector_t &GetRegistrationRequests() const;

		private:

			static void HandleRegistrationRequest(UserCLIThreadModuleRegister *Module, msg_struct_t &Message);
			static void HandleRegistrationAnswer(UserCLIThreadModuleRegister *Module, msg_struct_t &Message);

			/*!
			 * \brief Stores all registration requests
			 */
			registration_request_vector_t _RegistrationRequests;

			void HandleMessage(msg_struct_t &Parameters);
	};

	using UserCLIRegisterSharedPtr = shared_ptr<UserCLIThreadModuleRegister>;
} // namespace user_cli_thread_module_register


#endif // USER_CLI_THREAD_MODULE_REGISTER_H
