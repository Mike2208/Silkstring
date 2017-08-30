#include "network_dummy_connection.h"
#include "error_exception.h"
#include <memory.h>
#include <memory>

namespace network_dummy_connection
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	NetworkDummyConnection::NetworkDummyConnection()
	{}

	NetworkDummyConnection::NetworkDummyConnection(NetworkDummyConnection &&S)
	{
		S._LockData.lock();
		this->_LockData.lock();

		if(S._Partner != nullptr)
		{
			auto *const pPartner = S._Partner;
			if(	pPartner->_Partner	== &S &&
				pPartner			!= &S)
			{
				// Point partner to new position if it is currently pointing at S
				pPartner->_LockData.lock();

				pPartner->_Partner = this;

				pPartner->_LockData.unlock();
			}
		}

		this->_Partner = S._Partner;

		this->_ConnectionSide = S._ConnectionSide;
		this->_ReceivedData = std::move(S._ReceivedData);

		S._LockData.unlock();
		this->_LockData.unlock();
	}

	void NetworkDummyConnection::ConnectTwoDummies(NetworkDummyConnection &Server, NetworkDummyConnection &Client)
	{
		Server._ConnectionSide = SERVER_SIDE;
		Server.SetPartner(&Client);

		Client._ConnectionSide = CLIENT_SIDE;
		Client.SetPartner(&Server);
	}

	void NetworkDummyConnection::SetPartner(NetworkDummyConnection *Partner)
	{
		this->_LockData.lock();

		this->_Partner = Partner;

		this->_LockData.unlock();
	}

	void NetworkDummyConnection::Close()
	{
		this->_LockData.lock();

		this->_Partner = nullptr;

		this->_LockData.unlock();
	}

	bool NetworkDummyConnection::IsReadDataAvailable()
	{
		return !this->_ReceivedData.empty();
	}

	size_t NetworkDummyConnection::Read(byte_t *NetOutput, size_t OutputSize)
	{
		size_t readSize;

		this->_LockData.lock();

		// Look for smaller value
		readSize = OutputSize > this->_ReceivedData.size() ? this->_ReceivedData.size() : OutputSize;

		if(readSize > 0)
		{
			// Copy the requested information
			memcpy(NetOutput, &(this->_ReceivedData.front()), readSize);

			// Erase data from vector
			this->_ReceivedData.erase(this->_ReceivedData.begin(), this->_ReceivedData.begin() + readSize);

#ifdef DEBUG
			std::cout << "Received " << readSize << " bytes\n";
#endif
		}

		this->_LockData.unlock();

		return readSize;
	}

	size_t NetworkDummyConnection::Write(const byte_t *NetInput, size_t InputSize)
	{
		this->_LockData.lock();

		// Check if partner is set
		if(this->_Partner != nullptr)
		{
			this->_Partner->_LockData.lock();

			auto &receiveVector = this->_Partner->_ReceivedData;

			// Make room for new data
			const auto oldPos = receiveVector.size();
			receiveVector.resize(oldPos + InputSize);

			// Copy data
			auto *const pOldPos = &receiveVector.at(oldPos);
			memcpy(pOldPos, NetInput, InputSize);

#ifdef DEBUG
			std::cout << "Sent " << InputSize << " bytes: ";
			for(size_t i = 0; i< InputSize*sizeof(byte_t)/sizeof(char); ++i)
			{
				std::cout << (reinterpret_cast<const char*>(NetInput))[i];
			}

			std::cout << "\n";

			std::cout.flush();
#endif

			this->_Partner->_LockData.unlock();
		}
		else
		{
			this->_LockData.unlock();

			throw Exception(ERROR_NUM, "ERROR NetworkDummyConnection::Write(): No partner set to receive data\n");
		}

		this->_LockData.unlock();

		return InputSize;
	}

	NetworkDummyConnection::NetworkConnectionUniquePtr NetworkDummyConnection::Move()
	{
		return NetworkConnectionUniquePtr(new NetworkDummyConnection(std::move(*this)));
	}
}

namespace network_dummy_connection
{
	class TestNetworkDummyConnection
	{
		public:
			static bool Testing();
	};

	bool TestNetworkDummyConnection::Testing()
	{
		try
		{
			NetworkDummyConnection testServer, testClient;
			NetworkDummyConnection::ConnectTwoDummies(testServer, testClient);

			byte_vector_t testData{5,4};

			if(testClient.IsReadDataAvailable())
				return 0;

			if(testServer.Write(testData.data(), testData.size()) != testData.size())
				return 0;

			if(!testClient.IsReadDataAvailable())
				return 0;

			byte_vector_t testRead;
			testRead.resize(testData.size());
			if(testClient.Read(testRead.data(), testRead.size()) != testData.size())
				return 0;

			auto testMove = testServer.Move();

			return 1;
		}
		catch(Exception &)
		{
			return 0;
		}
	}
}
