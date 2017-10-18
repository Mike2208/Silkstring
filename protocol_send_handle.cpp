#include "protocol_send_handle.h"

namespace protocol_send_handle
{
	void ProtocolSendHandle::SendData(protocol_vector_t &Data)
	{
		return this->SendDataHandle(Data);
	}

	ProtocolModuleSendHandle::ProtocolModuleSendHandle(ProtocolSendHandle *Sender)
		: _Sender(Sender)
	{}

	void ProtocolModuleSendHandle::SendData(module_message_connection_t DataHeaderName, protocol_vector_t &Data)
	{
		assert(this->_Sender != nullptr);

		if(this->_Sender != nullptr)
		{
			auto curPos = Data.CurPos-Data.begin();

			// Make room for header
			Data.insert(Data.begin(), sizeof(protocol_header_t));

			// Adjust CurPos iterator
			Data.CurPos = Data.begin()+curPos+sizeof(protocol_header_t);

			// Write header to memory
			ProtocolModuleSendHandle::PrependHeaderNoResize(DataHeaderName, Data);

			this->_Sender->SendData(Data);
		}
	}

	void ProtocolModuleSendHandle::SendData(module_message_connection_t DataHeaderName, protocol_vector_t &&Data)
	{
		auto tmpData = std::move(Data);
		this->SendData(DataHeaderName, tmpData);
	}

	ProtocolSendHandle *ProtocolModuleSendHandle::GetSender()
	{
		return this->_Sender;
	}

	const ProtocolSendHandle *ProtocolModuleSendHandle::GetSender() const
	{
		return this->_Sender;
	}

	void ProtocolModuleSendHandle::PrependHeaderNoResize(module_message_connection_t HeaderName, protocol_vector_t &Data)
	{
		reinterpret_cast<protocol_header_t &>(Data.front()) = protocol_header_t(module_message_connection_t::GetHeaderName(HeaderName), Data.size());
	}
}
