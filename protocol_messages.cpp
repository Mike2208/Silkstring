#include "protocol_messages.h"

namespace protocol_messages
{
	bool module_message_connection_t::IsMessageType(const thread_multi_module_message_t &Message) const
	{
		const auto &MessageType = Message.Get<MessageTypeNum>();

		if(MessageType.MessageType != this->MessageType)
			return false;

		if(MessageType.IsSenderMessageType())
			return (Message.Get<MessageSenderIDNum>().ModuleID == this->ModuleID);
		else
			return (Message.Get<MessageReceiverIDNum>().ModuleID == this->ModuleID);
	}

	protocol_header_name_t module_message_connection_t::GetHeaderName(const thread_multi_module_message_t &Message)
	{
		const auto						&MessageType = Message.Get<MessageTypeNum>();
		const identifier_t::module_id_t &ModuleID = (MessageType.IsSenderMessageType() ? Message.Get<MessageSenderIDNum>().ModuleID :
																	Message.Get<MessageReceiverIDNum>().ModuleID);

		return module_message_connection_t::GetHeaderName(ModuleID, MessageType.MessageType);
	}

	protocol_header_name_t module_message_connection_t::GetHeaderName(module_message_connection_t Data)
	{
		return module_message_connection_t::GetHeaderName(Data.ModuleID, Data.MessageType);
	}

	protocol_header_name_t module_message_connection_t::GetHeaderName(identifier_t::module_id_t ModuleID, message_t::message_type_t MessageType)
	{
		const auto element = module_message_connection_t::ModuleHeaderMap.find(module_message_connection_t(ModuleID, MessageType));
		if(element == module_message_connection_t::ModuleHeaderMap.end())
			return EmptyModuleName;
		else
			return element->second;
	}

	module_message_connection_t::module_message_connection_t(identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, protocol_header_name_t HeaderName)
		: module_message_connection_t(_ModuleID, _MessageType)
	{
		assert(ModuleHeaderMap.find(*this) != ModuleHeaderMap.end());

		ModuleHeaderMap.emplace(*this, HeaderName);
	}

	bool module_message_connection_t::operator==(const module_message_connection_t &S) const
	{
		return (this->MessageType == S.MessageType && this->ModuleID == S.ModuleID);
	}

	bool module_message_connection_t::operator<(const module_message_connection_t &S) const
	{
		if(this->ModuleID < S.ModuleID)
			return true;
		else if(this->ModuleID == S.ModuleID)
		{
			if(this->MessageType < S.MessageType)
				return true;
			else
				return false;
		}
		else
			return false;
	}

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
