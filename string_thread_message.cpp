#include "string_thread_message.h"

namespace string_thread_messages
{
	string_user_modification_t::string_user_modification_t(StringUserID &&_StringID, mod_data_t &&_ModificationData)
		: StringID(std::move(_StringID)),
		  ModificationData(std::move(_ModificationData))
	{}

//	user_admin_request_t::user_admin_request_t(user_request_reason_t _RequestReason, request_user_admin_shared_ptr_t _RequestedStringUserAdmin, StringUserID _PeerID)
//		: RequestReason(_RequestReason),
//		  RequestedStringUserAdmin(_RequestedStringUserAdmin),
//		  PeerID(_PeerID)
//	{}

	peer_authentication_data_t::peer_authentication_data_t(StringUserID &&_PeerID)
		: PeerID(std::move(_PeerID))
	{}

	peer_verification_data_t::peer_verification_data_t(StringUserID &&_PeerID)
		: PeerID(std::move(_PeerID))
	{}

	user_storage_request_t::user_storage_request_t(user_storage_ptr_t &&_UserStorage, user_request_data_t &&_RequestData)
		: UserStorage(std::move(_UserStorage)),
		  RequestData(std::move(_RequestData))
	{}

	user_storage_answer_t::user_storage_answer_t(user_storage_ptr_t &&_UserStorage, user_storage_request_t &&_Request, bool _RequestGranted)
		:	UserStorage(std::move(_UserStorage)),
			Request(std::move(_Request)),
			RequestGranted(_RequestGranted)
	{}

	tls_credentials_answer_t::tls_credentials_answer_t(TLSCertificateCredentials &&_Credentials, extra_data_t &&_AnswerData, bool _RequestFound)
		: Credentials(std::move(_Credentials)),
		  AnswerData(std::move(_AnswerData)),
		  RequestFound(_RequestFound)
	{}
}
