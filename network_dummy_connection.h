#ifndef NETWORK_DUMMY_CONNECTION_H
#define NETWORK_DUMMY_CONNECTION_H

/*! \file network_dummy_connection.h
 *  \brief Header for NetworkDummyConnection class
 */


#include "network_connection.h"
#include "debug_flag.h"
#include <mutex>

/*!
 *  \brief Namespace for NetworkDummyConnection class
 */
namespace network_dummy_connection
{
	using std::mutex;
	using std::unique_ptr;

	using network_connection::NetworkConnection;

	using network_connection::byte_vector_t;

	using network_connection::connection_side_t;
	using network_connection::CLIENT_SIDE;
	using network_connection::SERVER_SIDE;

	class TestNetworkDummyConnection;

	/*!
	 * \brief Dummy connection for testing purposes
	 */
	class NetworkDummyConnection : public NetworkConnection
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			NetworkDummyConnection();

			NetworkDummyConnection(const NetworkDummyConnection &S) = delete;
			NetworkDummyConnection(NetworkDummyConnection &&S);

			NetworkDummyConnection &operator=(const NetworkDummyConnection &S) = delete;
			NetworkDummyConnection &operator=(NetworkDummyConnection &&S) = delete;

			/*!
			 * \brief Connect Two Dummies for communication
			 */
			static void ConnectTwoDummies(NetworkDummyConnection &Server, NetworkDummyConnection &Client);

			/*!
			 * \brief Set the connection's partner
			 */
			void SetPartner(NetworkDummyConnection *Partner);

			/*!
			 * \brief Close the connection again
			 */
			void Close();

			bool IsReadDataAvailable();

			size_t Read(byte_t *NetOutput, size_t OutputSize);

			size_t Write(const byte_t *NetInput, size_t InputSize);


			NetworkConnectionUniquePtr Move();

		private:

			connection_side_t		_ConnectionSide;
			NetworkDummyConnection	*_Partner	= nullptr;

			mutex			_LockData;
			byte_vector_t	_ReceivedData;
	};
} // namespace network_dummy_connection


#endif // NETWORK_DUMMY_CONNECTION_H
