#ifndef PROTOCOL_MODULE_CONNECTION_STATE_H
#define PROTOCOL_MODULE_CONNECTION_STATE_H

/*! \file protocol_module_connection_state.h
 *  \brief Header for ProtocolModuleConnectionState class
 */


#include "protocol_messages.h"
#include "protocol_thread_memory.h"
#include "protocol_module_instantiator.h"
#include "protocol_thread.h"

#include "debug_flag.h"

/*!
 *  \brief Namespace for ProtocolModuleConnectionState class
 */
namespace protocol_module_connection_state
{
	using protocol_module_instantiator::ProtocolModuleInstantiator;

	using protocol_thread_memory::ProtocolThreadMemory;

	using silkstring_message::thread_multi_module_fcn_t;

	using protocol_thread::ProtocolThread;

	using namespace protocol_messages;

	using protocol_thread_memory::CLIENT_SIDE;
	using protocol_thread_memory::SERVER_SIDE;

	using protocol_vector::protocol_vector_t;

	/*!
	 * \brief Control the state of the connection
	 */
	class ProtocolModuleConnectionState : public thread_multi_module_t
	{
		public:
			static constexpr decltype(ProtocolConnectionStateChangeRequestHeader) HeaderName = ProtocolConnectionStateChangeRequestHeader;

			/*!
			 * 	\brief Constructor
			 */
			ProtocolModuleConnectionState(ProtocolThreadMemory &Memory);

			void HandleMessage(msg_struct_t &Message);

			/*!
			 * \brief Handles a peer state change
			 * \param NewRequestedState New State set by peer
			 */
			void OnPeerStateChange(protocol_state_t NewRequestedState);

			/*!
			 * \brief Handles an internal state change request
			 * \param SenderID ID of module that requested state change
			 * \param NewRequestedState New state requested
			 */
			void OnInternalStateChangeRequest(identifier_t SenderID, protocol_state_t NewRequestedState);

			/*!
			 * \brief Handle a restart request from this side
			 */
			void OnRestartRequest();

		private:

			ProtocolThreadMemory &_Memory;

			/*!
			 * \brief Sends a message that the state was changed
			 */
			void SendInternalStateChangeMessage();

			/*!
			 * \brief Sends a message to the peer that the state was changed
			 */
			void SendExternalStateChangeMessage();

			/*!
			 * \brief Changes Internal State and notifies other modules
			 */
			void ChangeInternalState(protocol_state_t NewState);
	};


	class ProtocolModuleConnectionStateInstantiator : public ProtocolModuleInstantiator
	{
		public:
			ProtocolModuleConnectionStateInstantiator();

		private:

			thread_multi_module_shared_ptr_t CreateNewInstanceHandle(instantiation_data_t &Instance) const;
	};
} // namespace protocol_module_connection_state


#endif // PROTOCOL_MODULE_CONNECTION_STATE_H
