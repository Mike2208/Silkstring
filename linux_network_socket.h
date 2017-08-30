#ifndef LINUX_NETWORK_SOCKET_H
#define LINUX_NETWORK_SOCKET_H

/*! \file linux_network_socket.h
 *  \brief Header for NetworkSocket class
 */

// Only for linux sockets
#ifdef linux

#include "network_connection.h"
#include "typesafe_class.h"

#include <string>
#include <unistd.h>
#include <netinet/in.h>

/*!
 *  \brief Namespace for NetworkSocket class
 */
namespace linux_network_socket
{
	/*!
	 *	\brief Network Connection Interface
	 */
	using network_connection::NetworkConnection;

	/*!
	 *	\brief Vector for reading and writing
	 */
	using network_connection::byte_vector_t;
	using network_connection::vector_size_t;

	using network_connection::connection_side_t;
	using network_connection::CLIENT_SIDE;
	using network_connection::SERVER_SIDE;

	using std::string;


	/*!	\class socket_id_t
	 *	\brief Type for Socket IDs
	 */
	using socket_id_type = int;
	class socket_id_t
	{
		public:
			constexpr explicit socket_id_t(socket_id_type SocketID) : _SocketID(SocketID) {}

			// Add equality and comparison operators
			EQUALITY_OPERATORS(socket_id_t, _SocketID)
			COMPARISON_OPERATORS(socket_id_t, _SocketID)

			GET_OPERATOR(socket_id_type, _SocketID)

		private:
			socket_id_type _SocketID;
	};

	/*!
	 * \brief ID 0
	 */
	const socket_id_t SOCKET_NULL_ID = socket_id_t(0);

	/*!
	 * \brief ID for ERROR
	 */
	const socket_id_t SOCKET_ERR_ID = socket_id_t(-1);

	/*!
	 * \brief Default length of one read in bytes
	 */
	const vector_size_t DEFAULT_READ_LENGTH = 2048;

	/*!	\class port_t
	 *	\brief Type for Ports
	 */
	using port_type = u_int16_t;
	class port_t
	{
		public:
			constexpr explicit port_t(port_type PortNum) : _PortNum(PortNum) {}

			// Add equality and comparison operators
			EQUALITY_OPERATORS(port_t, _PortNum)
			COMPARISON_OPERATORS(port_t, _PortNum)

			GET_OPERATOR(port_type, _PortNum)

		private:
			port_type _PortNum;
	};

	enum ip_version
	{
		IPV4 = 1,
		IPV6
	};

	class TestIPAddress;

	class ip_addr_t
	{
		public:
			/*!
			 * \brief Constructor for IPv4
			 * \param IPv4_Address Address for IPv4
			 * \param PortNum Port Number
			 */
			ip_addr_t(struct in_addr IPv4_Address, port_t PortNum) noexcept;

			/*!
			 * \brief Constructor for IPv6
			 * \param IPv6_Address Address for IPv6
			 * \param PortNum Port Number
			 */
			ip_addr_t(struct in6_addr IPv6_Address, port_t PortNum) noexcept;

			/*!
			 * \brief ImportFromString
			 * \param IPAddress String with IP address
			 * \param Version IPv4 or IPv6
			 * \param PortNum Port Number
			 */
			static ip_addr_t ImportFromString(const string &IPAddress, ip_version Version, port_t PortNum);

			/*!
			 * \brief ImportFromString
			 * \param IPAddress String with IP address
			 * \param Version IPv4 or IPv6
			 * \param PortNum Port Number as string
			 */
			static ip_addr_t ImportFromString(const string &IPAddress, ip_version Version, string PortNum);

			/*!
			 * \brief Import from String
			 * \param IPAddress Address including port number
			 */
			static ip_addr_t ImportFromString(const string &IPAddress);

			bool operator==(const ip_addr_t &S) const;
			bool operator!=(const ip_addr_t &S) const;

			/*!
			 * \brief Converts IP address to string
			 * \return Returns IP address as string
			 */
			string AddrToString() const;

			/*!
			 * \brief Converts port number to string
			 * \return Returns string
			 */
			string PortToString() const noexcept;

			/*!
			 * \brief Converts address and port to string
			 * \return
			 */
			string ToString() const;

			/*!
			 * \brief Allow for access as sockaddr type
			 */
			operator const struct sockaddr &() const noexcept;

			ip_version GetIPVersion() const noexcept;

		private:
			struct sockaddr _Address;

			/*!
			 * \brief States whether data is IPv4 or IPv6
			 */
			ip_version _IPVersion;

			static bool CompareSockAddr(const sockaddr_in *Addr1, const sockaddr_in *Addr2) noexcept;
			static bool CompareSockAddr(const sockaddr_in6 *Addr1, const sockaddr_in6 *Addr2) noexcept;

			/*!
			 * \brief Constructor
			 * \param IPAddress String with IP address
			 * \param Version Either IPV4 or IPV6
			 * \param PortNum Port number
			 */
			ip_addr_t(const string &IPAddress, ip_version Version, port_t PortNum);

			/*!
			 * \brief Constructor
			 * \param IPAddress String with IP address
			 * \param Version Either IPV4 or IPV6
			 * \param PortNum Port number as string
			 */
			ip_addr_t(const string &IPAddress, ip_version Version, string PortNum);

			/*!
			 * \brief Constructo
			 * \param IPAddress Address with port number
			 */
			ip_addr_t(const string &IPAddress);

			friend class TestIPAddress;
	};

	/*!
	 * \brief Available socket protocols
	 */
	enum protocol_t
	{
		PROTOCOL_TCP, PROTOCOL_UDP
	};

	/*!	\class server_connection_queue_t
	 *	\brief Size of server connection queue
	 */
	using server_connection_queue_type = unsigned int;
	class server_connection_queue_t
	{
		public:
			explicit constexpr server_connection_queue_t(server_connection_queue_type Value) : _Value(Value) {}

			GET_OPERATOR(server_connection_queue_type, _Value)

		private:
			server_connection_queue_type _Value;
	};

	class NetworkSocketTCPClient;
	class NetworkSocketTCPServer;

	/*!
	 * \brief Class for accessing a socket
	 */
	class NetworkSocket : public NetworkConnection
	{
		public:
			/*!
			 *	\brief Constructor
			 *	\param SocketID ID of socket
			 */
			NetworkSocket(socket_id_t SocketID) noexcept;

			NetworkSocket(const NetworkSocket &S) = delete;
			NetworkSocket &operator=(const NetworkSocket &S) = delete;

			NetworkSocket(NetworkSocket &&S) noexcept;
			NetworkSocket &operator=(NetworkSocket &&S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~NetworkSocket() noexcept;

			/*!
			 * \brief Read Read data from socket
			 * \param NetOutput Storage for output from network
			 * \return Returns amount of bytes read
			 */
			virtual vector_size_t Read(byte_t *NetOutput, size_t OutputSize) = 0;

			/*!
			 * \brief Write to socket
			 * \param NetInput Storage for data to write
			 * \return Returns amount of bytes read
			 */
			virtual vector_size_t Write(const byte_t *NetInput, size_t InputSize) = 0;

			/*!
			 * \brief Closes socket
			 */
			void Close();

			bool IsReadDataAvailable();

			/*!
			 * \brief Returns ID of this socket
			 * \return Returns ID of this socket
			 */
			socket_id_t GetSocketID() const noexcept;

		protected:
			/*!
			 * \brief ID of socket to use
			 */
			socket_id_t _SocketID = SOCKET_ERR_ID;

			virtual NetworkConnectionUniquePtr Move() = 0;

		private:
	};

	class TestNetworkTCPSocket;

	/*!
	 *	\brief Class for accessing a socket using the TCP protocol
	 */
	class NetworkTCPSocket : public NetworkSocket
	{
		public:
			/*!
			 *	\brief Constructor
			 *	\param SocketID ID of socket
			 *	\param ClientOrServerSide Set connection to client or server-side
			 */
			NetworkTCPSocket(socket_id_t SocketID, connection_side_t ClientOrServerSide) noexcept;

			NetworkTCPSocket(const NetworkTCPSocket &S) = delete;
			NetworkTCPSocket &operator=(const NetworkTCPSocket &S) = delete;
			NetworkTCPSocket(NetworkTCPSocket &&S) noexcept = default;
			NetworkTCPSocket &operator=(NetworkTCPSocket &&S) noexcept = default;

			/*!
			 *	\brief Destructor
			 */
			~NetworkTCPSocket() noexcept;

			/*!
			 * \brief Read Read data from socket
			 * \param NetOutput Storage for output from network
			 */
			size_t Read(byte_t *NetOutput, size_t OutputSize);

			/*!
			 * \brief Write to socket
			 * \param NetInput Storage for data to write
			 */
			size_t Write(const byte_t *NetInput, size_t InputSize);

			/*!
			 * \brief Returns whether socket is server or client side
			 * \return Returns whether socket is server or client side
			 */
			connection_side_t GetConnectionSide() const;

		protected:
			virtual NetworkConnectionUniquePtr Move();

		private:
			/*!
			 * \brief Stores whether this connection is client- or server-side
			 */
			connection_side_t _ConnectionSide = CLIENT_SIDE;

			friend class TestNetworkTCPSocket;
	};

	/*!
	 * \brief Class for creating and accessing a socket as a client
	 */
	class NetworkSocketTCPClient : public NetworkTCPSocket
	{
		public:
			/*!
			 *	\brief Start a connection
			 *	\param ServerAddress Address of server to connect to
			 */
			static NetworkSocketTCPClient StartConnection(const ip_addr_t &ServerAddress);

			/*!
			 * \brief Initiate Connection to server
			 * \param ServerAddress
			 */
			bool Connect(const ip_addr_t &ServerAddress);

		private:

			/*!
			 * \brief Constructor
			 */
			NetworkSocketTCPClient();

			/*!
			 * \brief Constructor
			 */
			NetworkSocketTCPClient(const ip_addr_t &ServerAddress);
	};

	/*!
	 * \brief Class for accessing a socket as a server
	 */
	class NetworkSocketTCPServer
	{
		public:
			/*!
			 *	\brief Start a new server
			 *	\param BindAddress Address that server should bind to
			 *	\param TCPorUDP Use TCP (true) or UDP (false)
			 *	\param QueueSize How many client connection attempts are stored between calls to Accept
			 */
			static NetworkSocketTCPServer StartServer(const ip_addr_t &BindAddress, server_connection_queue_t QueueSize);

			// Delete copy constructors
			NetworkSocketTCPServer(const NetworkSocketTCPServer &S) = delete;
			NetworkSocketTCPServer &operator=(const NetworkSocketTCPServer &S) = delete;

			// Move constructors
			NetworkSocketTCPServer(NetworkSocketTCPServer &&S) noexcept;
			NetworkSocketTCPServer &operator=(NetworkSocketTCPServer &&S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~NetworkSocketTCPServer() noexcept;

			/*!
			 * \brief Waits for a client to attempt to connect
			 * \return Returns the new connection
			 */
			NetworkTCPSocket AcceptConnection();

		private:
			/*!
			 * \brief ID of socket to use
			 */
			socket_id_t _SocketID;

			/*!
			 * \brief close socket connection
			 */
			void Close();

			/*!
			 *	\brief Constructor
			 *	\param BindAddress Address that server should bind to
			 *	\param TCPorUDP Use TCP (true) or UDP (false)
			 *	\param QueueSize How many client connection attempts are stored between calls to Accept
			 */
			NetworkSocketTCPServer(const ip_addr_t &BindAddress, server_connection_queue_t QueueSize);
	};
} // namespace linux_network_socket

namespace network_socket
{
	using namespace linux_network_socket;
}

#endif // linux

#endif // LINUX_NETWORK_SOCKET_H
