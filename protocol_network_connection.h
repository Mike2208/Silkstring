#ifndef PROTOCOL_NETWORK_CONNECTION_H
#define PROTOCOL_NETWORK_CONNECTION_H

/*! \file protocol_network_connection.h
 *  \brief Header for ProtocolNetworkConnection class
 */


#include "network_connection.h"
#include "protocol_send_handle.h"
#include "protocol_data.h"
#include <array>

/*!
 *  \brief Namespace for ProtocolNetworkConnection class
 */
namespace protocol_network_connection
{
	using network_connection::NetworkConnectionUniquePtr;

	using vector_t::vector_type;

	using protocol_vector::byte_vector_t;
	using protocol_vector::protocol_vector_t;

	using protocol_data::protocol_thread_id_t;
	using protocol_data::ConnectionIDInvalid;
	using protocol_data::protocol_header_name_t;

	using protocol_send_handle::ProtocolSendHandle;

	using std::string;
	using std::array;
	using std::shared_ptr;
	using std::unique_ptr;

	static constexpr unsigned char ProtocolHeaderSize = 4;
	struct protocol_header_t
	{
		array<char, ProtocolHeaderSize> ProtocolHeader{{'D', 'A', 'T', 'A'}};
		char ProtocolDataLengthEnd = ':'; // Necessary for easy string write
		char ProtocolDataStart = '<';
		char ProtocolDataEnd = '>';
	};

	/*!
	 * \brief The ProtocolNetworkConnection class
	 */
	class ProtocolNetworkConnection : public ProtocolSendHandle
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			ProtocolNetworkConnection(NetworkConnectionUniquePtr &&Connection, protocol_thread_id_t ConnectionID);

			ProtocolNetworkConnection(const ProtocolNetworkConnection &S) = delete;
			ProtocolNetworkConnection &operator=(const ProtocolNetworkConnection &S) = delete;

			ProtocolNetworkConnection(ProtocolNetworkConnection &&S) = default;
			ProtocolNetworkConnection &operator=(ProtocolNetworkConnection &&S) = default;

			~ProtocolNetworkConnection();

			/*!
			 * \brief ReceiveData
			 * \return Returns received data
			 */
			protocol_vector_t ReceiveData();

			/*!
			 * \brief Send Data over connection
			 * \param Data
			 */
			void SendData(const protocol_vector_t &Data);

			/*!
			 * \brief Set new Connection
			 * \param Connection to be set
			 */
			void SetConnection(NetworkConnectionUniquePtr &&Connection);

			/*!
			 * \brief Release Ownership of connection
			 * \param UnregisterID Should ID of connection be unregistered
			 */
			NetworkConnectionUniquePtr &&ReleaseOwnership(bool UnregisterID = true);

			/*!
			 * \brief Gets ID of this connection
			 */
			protocol_thread_id_t GetID() const;


			bool IsReadDataAvailable();

		private:

			/*!
			 * \brief Network Connection for protocol to use
			 */
			NetworkConnectionUniquePtr _Connection;

			/*!
			 * \brief Buffer for read data
			 */
			byte_vector_t	_ReadBuffer;

			/*!
			 * \brief How much data must be received before data can be processed
			 */
			size_t			_ReadLength = 0;

			/*!
			 * \brief Connection ID
			 */
			protocol_thread_id_t _ID = ConnectionIDInvalid;

			/*!
			 * \brief Attempts to Read _ReadLength amount of bytes from _Connection to end of _ReadBuffer
			 * \return Returns number of bytes read
			 */
			size_t AttemptRead();

			virtual void SendDataHandle(protocol_vector_t &Data);

			/*!
			 * \brief Header Data
			 */
			static const protocol_header_t _HeaderData;// = protocol_header_t();
	};

	using ProtocolNetworkConnectionSharedPtr = shared_ptr<ProtocolNetworkConnection>;
	using ProtocolNetworkConnectionUniquePtr = unique_ptr<ProtocolNetworkConnection>;

} // namespace protocol_network_connection


#endif // PROTOCOL_NETWORK_CONNECTION_H
