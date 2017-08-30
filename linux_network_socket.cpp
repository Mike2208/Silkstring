#include "linux_network_socket.h"
#include "error_exception.h"

#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>

namespace linux_network_socket
{
	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;
	using error_exception::ERROR_NONE;

	ip_addr_t::ip_addr_t(struct in_addr IPv4_Address, port_t PortNum) noexcept : _IPVersion(IPV4)
	{
		// Set struct to use IPv4
		reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_addr = IPv4_Address;
		reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_port = htons(PortNum.get());
		reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_family = AF_INET;
	}

	ip_addr_t::ip_addr_t(struct in6_addr IPv6_Address, port_t PortNum) noexcept : _IPVersion(IPV6)
	{
		// Set struct to use IPv6
		reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_addr = IPv6_Address;
		reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_port = htons(PortNum.get());
		reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_family = AF_INET6;
	}

	ip_addr_t ip_addr_t::ImportFromString(const string &IPAddress, ip_version Version, port_t PortNum)
	{
		return ip_addr_t(IPAddress, Version, PortNum);
	}

	ip_addr_t ip_addr_t::ImportFromString(const string &IPAddress, ip_version Version, string PortNum)
	{
		return ip_addr_t(IPAddress, Version, PortNum);
	}

	ip_addr_t ip_addr_t::ImportFromString(const string &IPAddress)
	{
		return ip_addr_t(IPAddress);
	}

	bool ip_addr_t::operator==(const ip_addr_t &S) const
	{
		if(this->_IPVersion == IPV4)
		{
			if(S._IPVersion != IPV4)
				return 0;

			return ip_addr_t::CompareSockAddr(reinterpret_cast<const sockaddr_in*>(&(this->_Address)), reinterpret_cast<const sockaddr_in*>(&(S._Address)));
		}
		else if(this->_IPVersion == IPV6)
		{
			if(S._IPVersion != IPV6)
				return 0;

			return ip_addr_t::CompareSockAddr(reinterpret_cast<const sockaddr_in6*>(&(this->_Address)), reinterpret_cast<const sockaddr_in6*>(&(S._Address)));
		}

		return 0;
	}

	bool ip_addr_t::operator!=(const ip_addr_t &S) const
	{
		return !(*this == S);
	}

	/*!
	 * \brief Maximum Size for address string
	 */
	const size_t MaxSize = 10*1024;

	string ip_addr_t::AddrToString() const
	{
		int ipType;
		const void *pAddr;
		if(this->_IPVersion == IPV4)
		{
			ipType = AF_INET;
			pAddr = &(reinterpret_cast<const struct sockaddr_in*>(&this->_Address)->sin_addr);
		}
		else
		{
			ipType = AF_INET6;
			pAddr = &(reinterpret_cast<const struct sockaddr_in6*>(&this->_Address)->sin6_addr);
		}

		string addrString;
		size_t bufferSize = 1024;
		const char *pErr = nullptr;
		do
		{
			addrString.resize(bufferSize);
			pErr = inet_ntop(ipType, pAddr, &(addrString.front()), addrString.size());
			if(pErr == nullptr)
			{
				// If buffer too small, retry with bigger size
				if(errno == ENOSPC)
				{
					bufferSize += 1024;
					if(bufferSize < MaxSize)
						continue;
				}

				throw Exception( error_t(errno), "ERROR ip_addr_t::ToString(): Failed to convert to string\n");
			}
		}
		while(pErr == nullptr);

		return addrString;
	}

	string ip_addr_t::PortToString() const noexcept
	{
		if(this->_IPVersion == IPV4)
			return std::to_string(ntohs((reinterpret_cast<const struct sockaddr_in*>(&this->_Address)->sin_port)));
		else
			return std::to_string(ntohs((reinterpret_cast<const struct sockaddr_in6*>(&this->_Address)->sin6_port)));
	}

	string ip_addr_t::ToString() const
	{
		return this->AddrToString() + ":" + this->PortToString();
	}

	ip_addr_t::operator const struct sockaddr &() const noexcept
	{
		return this->_Address;
	}

	ip_version ip_addr_t::GetIPVersion() const noexcept
	{
		return this->_IPVersion;
	}

	bool ip_addr_t::CompareSockAddr(const sockaddr_in *Addr1, const sockaddr_in *Addr2) noexcept
	{
		if(memcmp(Addr1, Addr2, sizeof(sockaddr_in)) == 0)
			return 1;

		return 0;
	}

	bool ip_addr_t::CompareSockAddr(const sockaddr_in6 *Addr1, const sockaddr_in6 *Addr2) noexcept
	{
		if(memcmp(Addr1, Addr2, sizeof(sockaddr_in6)) == 0)
			return 1;

		return 0;
	}

	ip_addr_t::ip_addr_t(const string &IPAddress, ip_version Version, port_t PortNum)
		: _IPVersion( Version )
	{
		if(IPAddress.empty())
			throw Exception(ERROR_NUM, "ERROR ip_addr_t::ip_addr_t(): No IP address specified\n");

		void *pAddr;
		sa_family_t ipFamily;
		if(this->_IPVersion == IPV4)
		{
			ipFamily = AF_INET;
			pAddr = &(reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_addr);

			reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_port = htons(PortNum.get());
			reinterpret_cast<struct sockaddr_in*>(&this->_Address)->sin_family = AF_INET;
		}
		else
		{
			ipFamily = AF_INET6;
			pAddr = &(reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_addr);

			reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_port = htons(PortNum.get());
			reinterpret_cast<struct sockaddr_in6*>(&this->_Address)->sin6_family = AF_INET6;
		}

		error_t err(inet_pton(ipFamily, &(IPAddress.at(0)), pAddr));
		if(err < ERROR_NONE)
			throw Exception(err, "ERROR ip_addr_t::ip_addr_t(): Invalid IP address\n");
	}

	ip_addr_t::ip_addr_t(const string &IPAddress, ip_version Version, string PortNum)
		: ip_addr_t(IPAddress, Version, port_t(std::stoi(PortNum)))
	{}

	ip_addr_t::ip_addr_t(const string &IPAddress)
	{
		struct addrinfo hints;
		struct addrinfo *result;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /* Any protocol */

		// Get infot from string
		auto err = getaddrinfo(IPAddress.c_str(), nullptr, &hints, &result);
		if(err < 0)
			throw Exception(error_t(err), "ERROR ip_addr_t(const string &): Failed to get address from string\n");

		this->_Address = *result->ai_addr;
		if(result->ai_family == AF_INET)
			this->_IPVersion = IPV4;
		else
			this->_IPVersion = IPV6;

		freeaddrinfo(result);
	}

	NetworkSocket::NetworkSocket(socket_id_t SocketID) noexcept
		: _SocketID(SocketID)
	{}

	NetworkSocket::NetworkSocket(NetworkSocket &&S) noexcept
		: _SocketID(std::move(S._SocketID))
	{
		S._SocketID = SOCKET_ERR_ID;
	}

	NetworkSocket &NetworkSocket::operator=(NetworkSocket &&S) noexcept
	{
		try{ this->Close(); }
		catch(Exception e) { Exception(e.GetErrorNumber(), "WARNING NetworkSocket::operator=(NetworkSocket &&S): Close failed\n"); }

		this->_SocketID = std::move(S._SocketID);
		S._SocketID = SOCKET_ERR_ID;

		return *this;
	}

	NetworkSocket::~NetworkSocket() noexcept
	{
		try
		{
			this->Close();
		}
		catch(Exception e)
		{
			Exception(e.GetErrorNumber(), "ERROR NetworkSocket::~NetworkSocket(): Failed to close socket\n");
		}
	}

	void NetworkSocket::Close()
	{
		if(this->_SocketID >= SOCKET_NULL_ID)
		{
			auto err = shutdown(this->_SocketID.get(), SHUT_RDWR);

			err = close(this->_SocketID.get());
			if(err != 0)
				throw Exception(ERROR_NUM, "ERROR NetworkSocket::Close(): Failed to close socket\n");

			this->_SocketID = SOCKET_ERR_ID;
		}
	}

	bool NetworkSocket::IsReadDataAvailable()
	{
		int count;
		ioctl(this->_SocketID.get(), FIONREAD, &count);

		if(count > 0)
			return true;

		return false;
	}

	socket_id_t NetworkSocket::GetSocketID() const noexcept
	{
		return this->_SocketID;
	}

	NetworkTCPSocket::NetworkTCPSocket(socket_id_t SocketID, connection_side_t ConnectionSide) noexcept : NetworkSocket(SocketID), _ConnectionSide(ConnectionSide)
	{}

	size_t NetworkTCPSocket::Read(byte_t *NetOutput, size_t OutputSize)
	{
		const auto err = read(this->_SocketID.get(), NetOutput, OutputSize);

		if(err < 0)
		{
			throw Exception(ERROR_NUM, "ERROR: NetworkTCPSocket::Read(): Read failed\n");
		}

		return vector_size_t(err);
	}

	size_t NetworkTCPSocket::Write(const byte_t *NetInput, size_t InputSize)
	{
		// Write data
		const auto err = write(this->_SocketID.get(), NetInput, InputSize);

		if(err < 0)
		{
			throw Exception(ERROR_NUM, "ERROR: NetworkTCPSocket::Write(): Write failed\n");
		}

		return vector_size_t(err);
	}

	connection_side_t NetworkTCPSocket::GetConnectionSide() const
	{
		return this->_ConnectionSide;
	}

	NetworkTCPSocket::NetworkConnectionUniquePtr NetworkTCPSocket::Move()
	{
		return NetworkConnectionUniquePtr(new NetworkTCPSocket(std::move(*this)));
	}

	NetworkTCPSocket::~NetworkTCPSocket() noexcept
	{
		try
		{
			this->Close();
		}
		catch (Exception)
		{
			Exception(ERROR_NUM, "ERROR NetworkSocket::~NetworkSocket(): Failed to close socket\n");
		}
	}

	// Create socket from given data
	socket_id_t CreateSocket(const ip_addr_t &ServerAddress, protocol_t TCPorUDP) noexcept
	{
		const auto sockFamily = (ServerAddress.GetIPVersion() == IPV6 ? AF_INET6 : AF_INET);
		const auto sockType = (TCPorUDP == PROTOCOL_TCP ? SOCK_STREAM : SOCK_DGRAM);

		return socket_id_t(socket(sockFamily, sockType, 0));
	}

	NetworkSocketTCPClient NetworkSocketTCPClient::StartConnection(const ip_addr_t &ServerAddress)
	{
		return NetworkSocketTCPClient(ServerAddress);
	}

	bool NetworkSocketTCPClient::Connect(const ip_addr_t &ServerAddress)
	{
		if(this->GetSocketID() < SOCKET_NULL_ID)
			static_cast<NetworkTCPSocket&>(*this) = NetworkTCPSocket(CreateSocket(ServerAddress, PROTOCOL_TCP), CLIENT_SIDE);

		const sockaddr &sockServerAddress = ServerAddress;
		auto err = connect(this->_SocketID.get(), &sockServerAddress, sizeof(sockaddr));
		if(err != 0)
		{
			//throw Exception(ERROR_NUM, "ERROR NetworkSocketClient::NetworkSocketClient(): Couldn't conect to server\n");
			return 0;
		}

		return 1;
	}

	NetworkSocketTCPClient::NetworkSocketTCPClient()
		: NetworkTCPSocket(SOCKET_ERR_ID, CLIENT_SIDE)
	{}

	NetworkSocketTCPClient::NetworkSocketTCPClient(const ip_addr_t &ServerAddress)
		: NetworkTCPSocket(CreateSocket(ServerAddress, PROTOCOL_TCP), CLIENT_SIDE)
	{
		// Check socket
		if(this->_SocketID < SOCKET_NULL_ID)
		{
			throw Exception(ERROR_NUM, "ERROR NetworkSocketClient::NetworkSocketClient(): Couldn't create socket\n");
		}

		// Connect to server
		this->Connect(ServerAddress);
	}

	NetworkSocketTCPServer NetworkSocketTCPServer::StartServer(const ip_addr_t &BindAddress, server_connection_queue_t QueueSize)
	{
		return NetworkSocketTCPServer(BindAddress, QueueSize);
	}

	NetworkSocketTCPServer::NetworkSocketTCPServer(NetworkSocketTCPServer &&S) noexcept : _SocketID(std::move(S._SocketID))
	{
		S._SocketID = SOCKET_ERR_ID;
	}

	NetworkSocketTCPServer &NetworkSocketTCPServer::operator=(NetworkSocketTCPServer &&S) noexcept
	{
		try{ this->Close(); }
		catch(Exception)
		{}

		this->_SocketID = S._SocketID;
		S._SocketID = SOCKET_NULL_ID;

		return *this;
	}

	NetworkSocketTCPServer::~NetworkSocketTCPServer() noexcept
	{
		try{ this->Close(); }
		catch(Exception)
		{}
	}

	NetworkTCPSocket NetworkSocketTCPServer::AcceptConnection()
	{
		// Accept connection and create new socket for it
		socket_id_t newSocket(accept(this->_SocketID.get(), nullptr, 0));

		if(newSocket < SOCKET_NULL_ID)
		{
			throw Exception(ERROR_NUM, "ERROR NetworkSocketServer::AcceptConnection(): Couldn't accept new connection\n");
		}

		return NetworkTCPSocket(newSocket, SERVER_SIDE);
	}

	void NetworkSocketTCPServer::Close()
	{
		if(this->_SocketID >= SOCKET_NULL_ID)
		{
			auto err = close(this->_SocketID.get());
			if(err < 0)
				throw Exception(error_t(err), "ERROR NetworkSocketTCPServer::Close(): Failed to close socket\n");

			this->_SocketID = SOCKET_ERR_ID;
		}
	}

	NetworkSocketTCPServer::NetworkSocketTCPServer(const ip_addr_t &BindAddress, server_connection_queue_t QueueSize) : _SocketID(CreateSocket(BindAddress, PROTOCOL_TCP))
	{
		// Check socket
		if(this->_SocketID < SOCKET_NULL_ID)
		{
			throw Exception(ERROR_NUM, "ERROR NetworkSocketServer::NetworkSocketServer(): Couldn't create socket\n");
		}

		// Bind server to BindAddress
		const sockaddr &bindAddress = BindAddress;
		auto err = bind(this->_SocketID.get(), &bindAddress, sizeof(sockaddr));

		if(err != 0)
		{
			throw Exception(ERROR_NUM, "ERROR NetworkSocketServer::NetworkSocketServer(): Couldn't bind server to address\n");
		}

		// Listen for client connections
		err = listen(this->_SocketID.get(), QueueSize.get());

		if(err < 0)
		{
			throw Exception(error_t(err), "ERROR NetworkSocketServer::NetworkSocketServer(): Couldn't listen for clients\n");
		}
	}
}

#include "thread_function.h"
#include "thread_queued.h"

namespace linux_network_socket
{
	using thread_function::ThreadFunction;
	using thread_queued::SleepForMs;

	class TestIPAddress
	{
		public:
			static bool Testing();
	};

	bool TestIPAddress::Testing()
	{
		try
		{
			const string testString("127.0.0.1:5632");

			ip_addr_t testAddress1 = ip_addr_t::ImportFromString("127.0.0.1", IPV4, "5632");
			ip_addr_t testAddress2 = ip_addr_t::ImportFromString("127.0.0.1", IPV4, port_t(5632));
			ip_addr_t testAddress3 = ip_addr_t::ImportFromString(testString);

			// Test comparison
			if(testAddress1 != testAddress1 ||
					!(testAddress1 == testAddress1))
				return 0;

			// Test Constructors
			if(testAddress1 != testAddress2 ||
					testAddress1 != testAddress3 ||
					testAddress2 != testAddress3)
				return 0;

			// Test String conversion
			if(testString.compare(testAddress3.ToString()) != 0)
				return 0;

			// Test IPv6
			const string testStringv6("[::1]:5632");

			ip_addr_t testAddressIPv6 = ip_addr_t::ImportFromString(testString);
			if(testAddressIPv6.ToString().compare(testString) != 0)
				return 0;

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}

	class TestNetworkSocketTCPSocket
	{
		public:
			static bool Testing();
	};

	bool TestRead(NetworkSocket &Socket, const byte_vector_t &ReadCompare)
	{
		byte_vector_t readData;
		readData.resize(ReadCompare.size());

		if(Socket.ReadTimeout(readData.data(), ReadCompare.size(), network_connection::timeout_ms_t(5000)) != ReadCompare.size())
			return 0;

		for(byte_vector_t::const_iterator compIterator = ReadCompare.begin(), datIterator = readData.begin();
			datIterator != readData.end();
			++datIterator)
		{
			if(*compIterator != *datIterator)
				return 0;
		}

		return 1;
	}

	bool ServerTest()
	{
		try
		{
			NetworkSocketTCPServer testServer = NetworkSocketTCPServer::StartServer(
															ip_addr_t::ImportFromString("127.0.0.1:5328"),
															server_connection_queue_t(5));

			auto testServerConnection = testServer.AcceptConnection();

			if(testServerConnection.GetConnectionSide() != SERVER_SIDE)
				return 0;

			// Test Write
			const byte_vector_t testVector{5,10};
			if(testServerConnection.Write(testVector.data(), testVector.size()) != testVector.size())
				return 0;

			// Test Read
			return TestRead(testServerConnection, testVector);
		}
		catch(Exception&)
		{
			return 0;
		}
	}
	using test_thread_fcn_t = ThreadFunction<decltype(ServerTest), bool>;

	bool TestNetworkSocketTCPSocket::Testing()
	{
		try
		{
			test_thread_fcn_t ServerThread(ServerTest);

			NetworkSocketTCPClient clientConnection = NetworkSocketTCPClient::StartConnection(ip_addr_t::ImportFromString("127.0.0.1:5328"));

			// Test Read
			const byte_vector_t testVector{5,10};
			if(TestRead(clientConnection, testVector) == 0)
				return 0;

			// Test Write
			if(clientConnection.Write(testVector.data(), testVector.size()) != testVector.size())
				return 0;

			// Analyze server results
			return ServerThread.GetResult();
		}
		catch(Exception&)
		{
			return 0;
		}
	}
}
