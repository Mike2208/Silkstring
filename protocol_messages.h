#ifndef PROTOCOL_MESSAGES_H
#define PROTOCOL_MESSAGES_H

/*! \file protocol_messages.h
 *  \brief Header for protocol_messages class
 */


#include "silkstring_message.h"
#include "network_connection.h"
#include "string_user_id.h"
#include "crypto_tls_certificate_credentials.h"

#include "protocol_data.h"

/*!
 *  \brief Namespace for protocol_messages class
 */
namespace protocol_messages
{
	using std::map;

	using namespace silkstring_message;

	using crypto_tls_certificate_credentials::TLSCertificateCredentials;

	static constexpr identifier_t::queue_id_t ProtocolQueueID = StartQueueID + 1;

	using protocol_data::protocol_header_name_t;
	using protocol_data::EmptyModuleName;
	using protocol_vector::protocol_vector_t;

	using network_connection::connection_side_t;
	using network_connection::CLIENT_SIDE;
	using network_connection::SERVER_SIDE;

	using string_user_id::StringUserID;

	struct module_message_connection_t
	{
		using map_data_t = map<module_message_connection_t, protocol_header_name_t>;
		static map_data_t ModuleHeaderMap;

		identifier_t::module_id_t ModuleID;
		message_t::message_type_t MessageType;

		bool IsMessageType(const thread_multi_module_message_t &Message) const;

		static protocol_header_name_t GetHeaderName(const thread_multi_module_message_t &Message);
		static protocol_header_name_t GetHeaderName(module_message_connection_t Data);
		static protocol_header_name_t GetHeaderName(identifier_t::module_id_t ModuleID, message_t::message_type_t MessageType);

		module_message_connection_t(identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType, protocol_header_name_t HeaderName);

		bool operator==(const module_message_connection_t &S) const;
		bool operator<(const module_message_connection_t &S) const;

		constexpr module_message_connection_t(identifier_t::module_id_t _ModuleID, message_t::message_type_t _MessageType)
			:	ModuleID(_ModuleID),
				MessageType(_MessageType)
		{}

		private:
	};

	// Connection messages ---------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t ProtocolConnectionModuleID = StartModuleID+0;

	static constexpr message_t::message_type_t ProtocolConnectionModuleReceivedDataMessageType	= DefaultMessageType + 0;
	/*!
	 * \brief Struct for propagating data received from peer to appropriate modules
	 */
	struct received_data_t : public message_id_struct_t<ProtocolQueueID, ProtocolConnectionModuleID, ProtocolConnectionModuleReceivedDataMessageType, received_data_t>
	{
		protocol_header_name_t DataType;
		protocol_vector_t ReceivedData;

		template<class T>
		T *CheckAndParseData(protocol_header_name_t TypeHeaderName)
		{
			if(this->DataType == TypeHeaderName)
			{
				auto tmpIterator = this->ReceivedData.begin();
				return this->ReceivedData.ParseVector<T>(tmpIterator);
			}

			return nullptr;
		}

		template<class T>
		const T *CheckAndParseData(protocol_header_name_t TypeHeaderName) const
		{
			if(this->DataType == TypeHeaderName)
			{
				auto tmpIterator = this->ReceivedData.begin();
				return this->ReceivedData.ParseVector<T>(tmpIterator);
			}

			return nullptr;
		}

		received_data_t(protocol_header_name_t _DataType, protocol_vector_t _ReceivedData);
	};

	static constexpr message_t::message_type_t ProtocolConnectionModuleRequestReceiveMessageType	= DefaultMessageType + 1;
	struct request_receive_t : public message_id_struct_t<ProtocolQueueID, ProtocolConnectionModuleID, ProtocolConnectionModuleRequestReceiveMessageType, request_receive_t>
	{};

	static constexpr message_t::message_type_t ProtocolConnectionModulePeerDataMessageType	= DefaultMessageType + 2;
	struct peer_data_t : public message_id_struct_t<ProtocolQueueID, ProtocolConnectionModuleID, ProtocolConnectionModulePeerDataMessageType, peer_data_t>
	{
		protocol_vector_t PeerData;

		peer_data_t(protocol_vector_t &&_PeerData);
	};
	// ~Connection messages --------------------------------------------------------------------------

	// Connection State messages ---------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t ProtocolConnectionStateModuleID = StartModuleID+1;

	/*!
	 * \brief State of the protocol
	 */
	enum protocol_state_t
	{
		/*!
		 *	\brief Undefined state
		 */
		PROTOCOL_UNDEFINED = -1,

		/*!
		 *	\brief Start state
		 */
		PROTOCOL_STARTED = 0,

		/*!
		 *	\brief Unsecured state
		 */
		PROTOCOL_UNSECURE_CONNECTION_STATE,

		/*!
		 *	\brief State where TLS handshake is being performed
		 */
		PROTOCOL_TLS_HANDSHAKE_STATE,

		/*!
		 *	\brief State after TLS handshake failed 5 times
		 */
		PROTOCOL_TLS_HANDSHAKE_FAILED,

		/*!
		 *	\brief Secure connection established state
		 */
		PROTOCOL_SECURE_CONNECTION_STATE,

		/*!
		 *	\brief End state where the connection is closed
		 */
		PROTOCOL_END_STATE
	};

	static constexpr protocol_header_name_t ProtocolConnectionStateChangeRequestHeader{{{'S', 'T', 'A'}}};	
	static constexpr message_t::message_type_t ProtocolConnectionStateModuleStateChangeRequestMessageType = DefaultMessageType;
	static module_message_connection_t ProtocolChangeStateConnection(ProtocolConnectionStateModuleID, ProtocolConnectionStateModuleStateChangeRequestMessageType, ProtocolConnectionStateChangeRequestHeader);
	struct connection_state_change_request_t : public message_id_struct_t<ProtocolQueueID, ProtocolConnectionStateModuleID, ProtocolConnectionStateModuleStateChangeRequestMessageType, connection_state_change_request_t>
	{
		protocol_state_t NewState;

		connection_state_change_request_t(protocol_state_t _NewState)
			: NewState(_NewState)
		{}
	};

	/*!
	 * \brief Module where state changes are sent to
	 */
	static constexpr identifier_t::module_id_t ProtocolConnectionStateChangeInformerID = StartModuleID+2;
	static constexpr message_t::message_type_t ProtocolConnectionStateChangeInformerMessageType = DefaultMessageType;
	/*!
	 * \brief Used to inform modules of a change in the connection state
	 */
	struct connection_state_change_distribution_t : public message_id_struct_t<ProtocolQueueID, ProtocolConnectionStateChangeInformerID, ProtocolConnectionStateChangeInformerMessageType, connection_state_change_distribution_t>
	{
		protocol_state_t UpdatedState;

		connection_state_change_distribution_t(protocol_state_t _UpdatedState);
	};
	// ~Connection State messages --------------------------------------------------------------------------

	// TLS messages --------------------------------------------------------------------------------
	static constexpr identifier_t::module_id_t ProtocolTLSConnectionModuleID = StartModuleID+3;
	static constexpr message_t::message_type_t ProtocolTLSConnectionReadDataUpdateMessageType = DefaultMessageType + 0;

	static constexpr protocol_header_name_t ProtocolTLSConnectionReadDataUpdateHeaderName{{{'T', 'L', 'S'}}};
	static module_message_connection_t ProtocolTLSConnectionReadDataUpdateConnection(ProtocolTLSConnectionModuleID, ProtocolTLSConnectionReadDataUpdateMessageType, ProtocolTLSConnectionReadDataUpdateHeaderName);
	static constexpr message_t::message_type_t ProtocolTLSConnectionWriteDataUpdatedMessageType = DefaultMessageType + 1;
	/*!
	 * \brief New encrypted data is in write buffer for processing
	 */
	struct tls_write_data_updated_t : public message_id_struct_t<ProtocolQueueID, ProtocolTLSConnectionModuleID, ProtocolTLSConnectionWriteDataUpdatedMessageType, tls_write_data_updated_t>
	{};

	static constexpr message_t::message_type_t ProtocolTLSConnectionInputDataUpdateMessageType = DefaultMessageType + 2;
	/*!
	 * \brief Update Input buffer with data that should be encrypted and sent to peer
	 */
	struct tls_input_data_update_t : public protocol_vector_t, public message_id_struct_t<ProtocolQueueID, ProtocolTLSConnectionModuleID, ProtocolTLSConnectionInputDataUpdateMessageType, tls_input_data_update_t>
	{};

	static constexpr message_t::message_type_t ProtocolTLSConnectionOutputDataUpdatedMessageType = DefaultMessageType + 3;
	/*!
	 * \brief New decrypted data is in buffer
	 */
	struct tls_output_data_updated_t : public message_id_struct_t<ProtocolQueueID, ProtocolTLSConnectionModuleID, ProtocolTLSConnectionOutputDataUpdatedMessageType, tls_output_data_updated_t>
	{};

	static constexpr message_t::message_type_t ProtocolTLSConnectionHandshakeCompletedMessageType = DefaultMessageType + 4;
	struct tls_handshake_completed_t : public message_id_struct_t<ProtocolQueueID, ProtocolTLSConnectionModuleID, ProtocolTLSConnectionHandshakeCompletedMessageType, tls_handshake_completed_t>
	{
		bool Success;

		tls_handshake_completed_t(bool _Success);
	};
	// ~TLS messages -------------------------------------------------------------------------------
} // namespace protocol_messages


#endif // PROTOCOL_MESSAGES_H
