#ifndef PROTOCOL_CONNECTION_MODULE_H
#define PROTOCOL_CONNECTION_MODULE_H

/*! \file protocol_connection_module.h
 *  \brief Header for ProtocolConnectionModule class
 */


#include "protocol_send_handle.h"
#include "protocol_thread_memory.h"
#include "protocol_messages.h"
#include "protocol_network_connection.h"
#include "protocol_module_instantiator.h"

/*!
 *  \brief Namespace for ProtocolConnectionModule class
 */
namespace protocol_connection_module
{
	using protocol_send_handle::ProtocolSendHandle;
	using protocol_network_connection::ProtocolNetworkConnection;
	using protocol_thread_memory::ProtocolThreadMemory;

	using protocol_module_instantiator::ProtocolModuleInstantiator;

	using protocol_data::protocol_header_t;
	using namespace protocol_messages;

	class TestProtocolConnectionModule;

	/*!
	 * \brief Mocule for handling a connection
	 */
	class ProtocolConnectionModule : public thread_multi_module_t, public ProtocolSendHandle
	{
		public:
			static constexpr identifier_t::module_id_t ModuleID = ProtocolConnectionModuleID;

			ProtocolConnectionModule(ProtocolNetworkConnection &Connection, ProtocolThreadMemory &ThreadMemory);

		private:

			void HandleMessage(msg_struct_t &Parameters);

			/*!
			 * \brief Handle a request to receive data
			 */
			bool HandleReceiveRequest();

			/*!
			 * \brief Simulate a receive
			 */
			bool HandlePeerData(peer_data_t &PeerData);

			/*!
			 * \brief Parse Received Data
			 * \return Returns whether the received data could be parsed
			 */
			bool ParseReceivedData(protocol_vector_t &ReceivedData);

			/*!
			 * \brief Network Connection
			 */
			ProtocolNetworkConnection &_Connection;

			/*!
			 * \brief Pointer to thread memory
			 */
			ProtocolThreadMemory &_ThreadMemory;

			void SendDataHandle(protocol_vector_t &Data);

			friend class TestProtocolConnectionModule;
	};

	class ProtocolConnectionModuleInstantiator : public ProtocolModuleInstantiator
	{
		public:
			ProtocolConnectionModuleInstantiator();

		private:
			thread_multi_module_shared_ptr_t CreateNewInstanceHandle(instantiation_data_t &Instance) const;
	};
} // namespace protocol_connection_module


#endif // PROTOCOL_CONNECTION_MODULE_H
