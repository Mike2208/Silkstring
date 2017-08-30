#ifndef USER_IO_MESSAGES_H
#define USER_IO_MESSAGES_H

#include "string_user_id.h"
#include "silkstring_message.h"

namespace user_io_messages
{
	using std::string;
	using std::mutex;

	using silkstring_message::thread_multi_module_manager_t;
	using silkstring_message::thread_multi_module_t;
	using silkstring_message::thread_multi_module_shared_ptr_t;
	using silkstring_message::thread_multi_module_message_t;
	using silkstring_message::identifier_t;
	using silkstring_message::message_ptr;
	using silkstring_message::message_t;

	using silkstring_message::id_vector_t;

	using silkstring_message::message_t;
	using silkstring_message::identifier_t;
	using silkstring_message::message_id_thread_struct_t;

	using silkstring_message::StartQueueID;
	using silkstring_message::DefaultMessageType;
	using silkstring_message::StartModuleID;

	using string_user_id::StringUserID;

	static constexpr identifier_t::thread_id_t UserIOThreadID = 0;
	static constexpr identifier_t::queue_id_t UserIOQueueID = StartQueueID + 2;

	// Initialization Module ----------------------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t UserIOInitializationModuleID = StartModuleID;

	static constexpr message_t::message_type_t UserIOInitializationMessageType = DefaultMessageType;
	struct initialization_message_t : public message_id_thread_struct_t<UserIOQueueID, UserIOInitializationModuleID, UserIOThreadID, UserIOInitializationMessageType, initialization_message_t>
	{
		/*!
		 * \brief argc from main()
		 */
		int argc;

		/*!
		 * \brief argv from main()
		 */
		const char **argv;
	};
	// ~Initialization Module ---------------------------------------------------------------------------------------------

	// CLI Read Request Module --------------------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t UserIOCLIReadModuleID = StartModuleID + 1;

	static constexpr message_t::message_type_t UserIOCLIReadMessageType = DefaultMessageType + 0;
	/*!
	 * \brief Parses the given string
	 */
	struct cli_read_message_t : public message_id_thread_struct_t<UserIOQueueID, UserIOCLIReadModuleID, UserIOThreadID, UserIOCLIReadMessageType, cli_read_message_t>
	{
		string InputString;

		cli_read_message_t(string _InputString)
			: InputString(_InputString)
		{}
	};
	// ~CLI Read Request Module -------------------------------------------------------------------------------------------

	// Registration Module ------------------------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t UserIORegistrationModuleID = StartModuleID + 2;
	static constexpr identifier_t UserIORegistrationID = identifier_t(UserIOQueueID, UserIORegistrationModuleID, UserIOThreadID);


	static constexpr message_t::message_type_t UserIORegistrationRequestMessageType = DefaultMessageType;
	/*!
	 * \brief Struct for sending out a registration request
	 */
	struct registration_request_message_t : public message_id_thread_struct_t<UserIOQueueID, UserIORegistrationModuleID, UserIOThreadID, UserIORegistrationRequestMessageType, registration_request_message_t>
	{
		/*!
		 * \brief ID of user that requested registration
		 */
		StringUserID NewUserID;

		/*!
		 * \brief Extra Information that was sent along
		 */
		string ExtraInfo;

		registration_request_message_t(StringUserID _NewUserID, string _ExtraInfo = "")
			: NewUserID(_NewUserID), ExtraInfo(_ExtraInfo)
		{}
	};

	static constexpr message_t::message_type_t UserIORegistrationAnswerMessageType = UserIORegistrationRequestMessageType + 3;
	/*!
	 * \brief Struct for sending out a registration answer
	 */
	struct registration_answer_message_t : public message_id_thread_struct_t<UserIOQueueID, UserIORegistrationModuleID, UserIOThreadID, UserIORegistrationAnswerMessageType, registration_answer_message_t>
	{
		/*!
		 * \brief ID of user that requested registration
		 */
		StringUserID NewUserID;

		/*!
		 * \brief Was the request granted?
		 */
		bool RequestGranted;

		registration_answer_message_t(StringUserID _NewUserID, bool _RequestGranted)
			: NewUserID(_NewUserID), RequestGranted(_RequestGranted)
		{}
	};
	// ~Registration Module -----------------------------------------------------------------------------------------------
}

#endif // USER_IO_MESSAGES_H
