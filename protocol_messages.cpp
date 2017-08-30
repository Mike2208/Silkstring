#include "protocol_messages.h"

namespace protocol_messages
{
	connection_state_change_distribution_t::connection_state_change_distribution_t(protocol_state_t _UpdatedState)
		: UpdatedState(_UpdatedState)
	{}

	certificate_tls_credentials_answer_t::certificate_tls_credentials_answer_t(TLSCertificateCredentials &&_Credentials)
		: Credentials(std::move(_Credentials))
	{}

	id_verification_request_t::id_verification_request_t(StringUserID _IDToVerify)
		: IDToVerify(_IDToVerify)
	{}

	received_data_t::received_data_t(protocol_header_name_t _DataType, protocol_vector_t _ReceivedData)
		: DataType(_DataType), ReceivedData(_ReceivedData)
	{}

	peer_data_t::peer_data_t(protocol_vector_t &&_PeerData)
		: PeerData(std::move(_PeerData))
	{}

	tls_handshake_completed_t::tls_handshake_completed_t(bool _Success)
		: Success(_Success)
	{}

	tls_received_authentication_id_t::tls_received_authentication_id_t(StringUserID &&_PeerID)
		: PeerID(std::move(_PeerID))
	{}
}
