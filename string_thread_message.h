#ifndef STRING_THREAD_MESSAGES_H
#define STRING_THREAD_MESSAGES_H

/*! \file string_thread_messages.h
 *  \brief Header for String Thread messages
 */


#include "silkstring_message.h"

#include "crypto_tls_certificate_credentials.h"
#include "string_user_storage.h"
#include "string_user_id.h"
#include "string_user_admin.h"

namespace string_thread_messages
{
	using std::unique_ptr;
	using std::shared_ptr;
	using std::atomic;

	using vector_t::vector_t;

	using namespace silkstring_message;

	using extra_data_t = SharedDynamicPointer<void>;

	using string_user_id::StringUserID;

	using dynamic_pointer::SharedDynamicPointerTyped;

	using string_user::UserCertID;
	using string_user::StringUser;
	using string_user_admin::StringUserAdmin;
	using string_user_admin::UserCertAdmin;
	using string_user::UserCert;

	using string_user_storage::StringUserStorage;

	using crypto_tls_certificate_credentials::TLSCertificateCredentials;

	using string_user_id_vector = vector_t<StringUserID>;
	using user_cert_id_vector_t = vector_t<UserCertID>;

	struct string_user_id_and_certs
	{
		StringUserID StringID;

		user_cert_id_vector_t	UserCertIDs;
	};

	using string_user_id_and_certs_vector_t = vector_t<string_user_id_and_certs>;

	/*!
	 * \brief ID for StringManager queue
	 */
	static constexpr identifier_t::queue_id_t StringQueueID = StartQueueID + 3;

	static constexpr identifier_t::thread_id_t StringThreadID = DefaultThreadID;

	// General Module --------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t GeneralModuleID = StartModuleID+0;

	// ~General Module -------------------------------------------------------------------------------

	// Register --------------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t RegistrationModuleID = StartModuleID+1;

	static constexpr message_t::message_type_t RegistrationMessageType = DefaultMessageType+0;

	/*!
	 * \brief Struct containing the new ID to register
	 */
	struct registration_message_t : public message_id_thread_struct_t<StringQueueID, RegistrationModuleID, StringThreadID, RegistrationMessageType, registration_message_t>
	{
		StringUserID NewID;
	};
	// ~Register ------------------------------------------------------------------------------------

	// ModificationRequest ----------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t ModificationModuleID = StartModuleID+2;

	static constexpr message_t::message_type_t ModificationModuleModificationMessageType = DefaultMessageType+0;

	enum string_user_modification_request_type_t
	{
		/*!
		 *	\brief Add UserAdminCert to StringUserAdmin. Upgrades StringUser to StringUserAdmin if same StringUserID found
		 * modification_add_user_admin_cert_t
		 */
		STRING_USER_MODIFICATION_ADD_USER_ADMIN_CERT = 0,

		/*!
		 *	\brief Add UserAdmin to StringUser or StringUserAdmin with given StringUserID
		 *	modification_add_user_cert_t
		 */
		STRING_USER_MODIFICATION_ADD_USER_CERT,
	};

	struct modification_add_user_admin_cert_t
	{
		UserCertAdmin NewAdminCert;
	};

	struct modification_add_user_cert_t
	{
		UserCert NewCert;
	};

	struct string_user_modification_t : public message_id_thread_struct_t<StringQueueID, ModificationModuleID, StringThreadID, ModificationModuleModificationMessageType, string_user_modification_t>
	{
		using mod_data_t = SharedDynamicPointerTyped<	modification_add_user_admin_cert_t,
														modification_add_user_cert_t>;

		/*!
		 * \brief ID of string that should be modified
		 */
		StringUserID	StringID;

		/*!
		 * \brief Modification data type determined by string_user_modification_request_type_t
		 */
		mod_data_t		ModificationData;

		string_user_modification_t(StringUserID &&_StringID, mod_data_t &&_ModificationData);
	};

	// ~ModificationRequest ---------------------------------------------------------------------------------

	// UserRequest ----------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t UserRequestModuleID = StartModuleID+3;

	enum user_request_reason_t
	{
		/*!
		 *	\brief The requested TLSCertificateCredentials is for authentication to a peer
		 *	string_user_id_vector
		 */
		PEER_AUTHENTICATION = 0,

		/*!
		 *	\brief The requested TLSCertificateCredentials is for verification of a peer
		 *  string_user_id_and_certs
		 */
		PEER_VERIFICATION
	};

	struct peer_authentication_data_t
	{
		StringUserID PeerID;

		peer_authentication_data_t(StringUserID &&_PeerID);
	};

	struct peer_verification_data_t
	{
		StringUserID PeerID;

		peer_verification_data_t(StringUserID &&_PeerID);
	};

	using user_storage_ptr_t = unique_ptr<StringUserStorage>;

	static constexpr message_t::message_type_t UserStorageRequestMessageType = DefaultMessageType + 2;
	struct user_storage_request_t : public message_id_thread_struct_t<StringQueueID, UserRequestModuleID, StringThreadID, UserStorageRequestMessageType, user_storage_request_t>
	{
		using request_data_t = SharedDynamicPointerTyped<	peer_authentication_data_t,
															peer_verification_data_t>;

		using user_storage_ptr_t = string_thread_messages::user_storage_ptr_t;

		/*!
		 * \brief User Storage to update
		 */
		user_storage_ptr_t UserStorage;

		/*!
		 * \brief Information defining request
		 */
		request_data_t RequestData;

		user_storage_request_t(user_storage_ptr_t &&_UserStorage, request_data_t _RequestData);
	};

	static constexpr message_t::message_type_t UserStorageAnswerMessageType = DefaultMessageType + 3;
	struct user_storage_answer_t : public message_id_thread_struct_t<StringQueueID, UserRequestModuleID, StringThreadID, UserStorageAnswerMessageType, user_storage_answer_t>
	{
		using user_storage_ptr_t = string_thread_messages::user_storage_ptr_t;

		/*!
		 * \brief User Storage to update
		 */
		user_storage_ptr_t UserStorage;


		/*!
		 * \brief Request that was answered
		 */
		user_storage_request_t Request;

		/*!
		 *	\brief Could the request be granted
		 */
		bool	RequestGranted;

		user_storage_answer_t(user_storage_ptr_t &&_UserStorage, user_storage_request_t &&_Request, bool _RequestGranted);
	};

	static constexpr message_t::message_type_t TLSCredentialsRequestMessageType = DefaultMessageType + 4;
	struct tls_credentials_request_t : public message_id_thread_struct_t<StringQueueID, UserRequestModuleID, StringThreadID, TLSCredentialsRequestMessageType, tls_credentials_request_t>
	{
		/*!
		 * \brief Reason for this request, or what will the certificate be used for
		 */
		user_request_reason_t					RequestReason;

		/*!
		 * \brief Extra Data for Request
		 */
		extra_data_t							RequestData;

		/*!
		 * \brief Old Credentials that should be updated
		 */
		unique_ptr<TLSCertificateCredentials>	Credentials;
	};

	static constexpr message_t::message_type_t TLSCredentialsAnswerMessageType = DefaultMessageType + 5;
	struct tls_credentials_answer_t : public message_id_thread_struct_t<StringQueueID, UserRequestModuleID, StringThreadID, TLSCredentialsAnswerMessageType, tls_credentials_answer_t>
	{
		/*!
		 * \brief Credentials usable by a TLS connection
		 */
		TLSCertificateCredentials	Credentials;

		/*!
		 * \brief Extra Answer Data
		 */
		extra_data_t				AnswerData;

		/*!
		 * \brief Was the request granted?
		 */
		bool						RequestFound;

		tls_credentials_answer_t(TLSCertificateCredentials &&_Credentials, extra_data_t &&_AnswerData, bool _RequestFound);
	};
	// ~UserRequest ----------------------------------------------------------------------------

} // namespace string_message


#endif // STRING_THREAD_MESSAGES_H
