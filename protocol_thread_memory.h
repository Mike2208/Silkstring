#ifndef PROTOCOL_THREAD_MEMORY_H
#define PROTOCOL_THREAD_MEMORY_H

/*! \file protocol_thread_memory.h
 *  \brief Header for ProtocolThreadMemory class
 */


#include "protocol_messages.h"
#include "protocol_network_connection.h"
#include "network_connection.h"
#include "string_user_id.h"
#include "protocol_send_handle.h"
#include "global_message_queue_thread.h"
#include "string_thread_message.h"

/*!
 *  \brief Namespace for ProtocolThreadMemory class
 */
namespace protocol_thread_memory
{
	using std::map;

	using protocol_network_connection::ProtocolNetworkConnection;

	using protocol_messages::thread_multi_module_manager_t;

	using protocol_send_handle::ProtocolModuleSendHandle;

	using network_connection::connection_side_t;
	using network_connection::SERVER_SIDE;
	using network_connection::CLIENT_SIDE;

	using protocol_messages::identifier_t;
	using protocol_messages::protocol_state_t;
	using protocol_messages::PROTOCOL_UNDEFINED;

	using string_user_id::StringUserID;

	using protocol_messages::identifier_t;
	using protocol_messages::protocol_header_name_t;
	//using string_thread_messages::request_user_shared_ptr_t;
	//using string_thread_messages::request_user_admin_shared_ptr_t;

	using global_message_queue_thread::GlobalMessageQueueThread;

	using protocol_error_counter_t = uint16_t;

	struct protocol_header_module_map_t : public map<protocol_header_name_t, identifier_t>
	{
		using map_t = map<protocol_header_name_t, identifier_t>;

		bool AlreadyRegistered(map_t::key_type HeadeName, map_t::mapped_type ModuleID) const;
		bool AlreadyRegistered(map_t::value_type Pair) const;
	};

	/*!
	 * \brief Memory owned by a single protocol thread
	 */
	struct ProtocolThreadMemory
	{
		/*!
		 * \brief IF of this thread
		 */
		const identifier_t::thread_id_t ProtocolThreadID;

		/*!
		 * \brief Global Message Queue
		 */
		GlobalMessageQueueThread		*GlobalQueue = nullptr;

		/*!
		 * \brief Pointer to this threads message queue
		 */
		thread_multi_module_manager_t	*ProtocolThreadQueue = nullptr;

		/*!
		 * \brief Used to send data to peer
		 */
		ProtocolModuleSendHandle		SendHandle;

		/*!
		 * \brief Is this connection acting as server or client?
		 */
		connection_side_t ConnectionSide = CLIENT_SIDE;

		/*!
		 * \brief Maximum handshake attempts to try
		 */
		size_t MaximumTLSHandshakeAttempts = 5;

		/*!
		 * \brief Manages Which headers send information to which modules
		 */
		protocol_header_module_map_t	HeaderModuleMap = protocol_header_module_map_t();

		/*!
		 * \brief State of the connection
		 */
		protocol_state_t State = PROTOCOL_UNDEFINED;

		/*!
		 * \brief Peers connection state
		 */
		protocol_state_t PeerState = PROTOCOL_UNDEFINED;

		/*!
		 * \brief ID of own user certificate
		 */
		StringUserID OwnConnectionCertificateID = StringUserID();

		/*!
		 * \brief ID of peers user certificate
		 */
		StringUserID PeerConnectionCertificateID = StringUserID();

//		/*!
//		 * \brief Own connection certificate
//		 */
//		request_user_admin_shared_ptr_t		OwnConnectionCertificate = nullptr;

//		/*!
//		 * \brief Peer connection certificate data
//		 */
//		request_user_shared_ptr_t			PeerConnectionCertificate = nullptr;

		/*!
		 * \brief Counts the amount of errors that have occured since the protocol started
		 */
		protocol_error_counter_t ErrorCounter = 0;
	};
} // namespace protocol_thread_memory


#endif // PROTOCOL_THREAD_MEMORY_H
