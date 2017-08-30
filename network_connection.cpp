#include "network_connection.h"
#include "vector_t.h"
#include "error_exception.h"

namespace network_connection
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	using read_buffer_t = vector_t<char>;

	timeout_ms_t GetCurrentClockMS()
	{
		constexpr auto toMS = static_cast<timeout_ms_type>(CLOCKS_PER_SEC)/1000;
		return timeout_ms_t(static_cast<timeout_ms_type>(clock())/toMS);
	}

	NetworkConnection::NetworkConnection() = default;

	NetworkConnection::~NetworkConnection()
	{}

	byte_vector_t NetworkConnection::Read(size_t OutputSize)
	{
		byte_vector_t retVal(OutputSize);

		this->Read(retVal.data(), OutputSize);

		return retVal;
	}

	size_t NetworkConnection::ReadTimeout(byte_t *NetOutput, size_t OutputSize, timeout_ms_t Timeout)
	{
		auto endTime = GetCurrentClockMS()+Timeout;

		byte_t *pCurPos = NetOutput;
		size_t totalReadSize = 0;

		// Continue reading until size_t is encountered
		do
		{
			auto curReadSize = this->Read(pCurPos, OutputSize - totalReadSize);

			pCurPos += curReadSize;
			totalReadSize += curReadSize;
		}
		while(totalReadSize < OutputSize && (Timeout <= timeout_ms_t(0) || GetCurrentClockMS() < endTime));

		return totalReadSize;
	}

	size_t NetworkConnection::WriteTimeout(const byte_t *NetInput, size_t InputSize, timeout_ms_t Timeout)
	{
		auto endTime = GetCurrentClockMS()+Timeout;

		const byte_t *pCurPos = NetInput;
		size_t totalWriteSize = 0;

		// Continue reading until size_t is encountered
		do
		{
			auto curWriteSize = this->Write(NetInput, InputSize - totalWriteSize);

			pCurPos += curWriteSize;
			totalWriteSize += curWriteSize;
		}
		while(totalWriteSize < InputSize && (Timeout <= timeout_ms_t(0) || GetCurrentClockMS() < endTime));

		return totalWriteSize;
	}

	string NetworkConnection::ReadString(timeout_ms_t Timeout)
	{
		string retString;
		read_buffer_t tmpRead(STRING_READ_LENGTH);

		auto endTime = GetCurrentClockMS()+Timeout;
		bool endFound = false;

		// Continue reading until a '\0' is encountered
		do
		{
			auto readSize = this->Read(reinterpret_cast<byte_t*>(&tmpRead.front()), tmpRead.capacity());

			// Wait for read
			if(readSize == 0)
			{
				// Sleep for a while before retrying
				constexpr struct timespec waitTime{0, static_cast<long>(STRING_WAIT_TIME_MS.get()*1000000)};
				nanosleep(&waitTime, nullptr);

				continue;
			}

			for(size_t i = 0; i < readSize; ++i)
			{
				const char curChar = tmpRead.at(i);

				// Append read to string
				retString += curChar;

				// Check if string has reached end
				if(curChar == '\0')
				{
					endFound = true;
					break;
				}
			}
			if(endFound)
				break;
		}
		while(Timeout <= timeout_ms_t(0) || GetCurrentClockMS() < endTime);

		if(!endFound)
			throw Exception(ERROR_NUM, "ERROR NetworkConnection::ReadString(): Failed to finish reading string. Connection timed out\n");

		return retString;
	}

	void NetworkConnection::WriteString(const string &S, timeout_ms_t Timeout)
	{
		auto endTime = GetCurrentClockMS()+Timeout;

		// Continue writing data until either finished or time is up
		size_t dataToWrite = S.length()+1;
		do
		{
			dataToWrite -= this->Write(reinterpret_cast<const byte_t*>(S.data()), dataToWrite);

			if(Timeout > timeout_ms_t(0) && GetCurrentClockMS() >= endTime)
				throw Exception(ERROR_NUM, "ERROR NetworkConnection::WriteString(): Failed to finish writing string. Connection timed out\n");
		}
		while(dataToWrite > 0);
	}

	NetworkConnectionUniquePtr NetworkConnection::MoveOwnership()
	{
		return this->Move();
	}
}

#include "network_dummy_connection.h"

namespace network_connection
{
	using network_dummy_connection::NetworkDummyConnection;

	class TestNetworkConnection
	{
		public:
			static bool Testing();
	};

	bool TestNetworkConnection::Testing()
	{
		try
		{
			NetworkDummyConnection testServerDummy, testClientDummy;
			NetworkDummyConnection::ConnectTwoDummies(testServerDummy, testClientDummy);

			auto &testServer = static_cast<NetworkConnection&>(testServerDummy);
			auto &testClient = static_cast<NetworkConnection&>(testClientDummy);

			// byte_vector_t Read(size_t OutputSize)
			const byte_vector_t testData{5,6};
			testServerDummy.Write(testData.data(), testData.size());

			auto testRead = testClient.Read(testData.size());
			if(testRead.size() != testData.size())
				return 0;

			if(testRead.at(0) != 5 || testRead.at(1) != 6)
				return 0;

			// size_t ReadTimeout(byte_t *NetOutput, size_t OutputSize, timeout_ms_t Timeout = STRING_TIMEOUT_MS)
			testServerDummy.Write(testData.data(), testData.size());
			if(testClient.ReadTimeout(testRead.data(), testRead.size(), timeout_ms_t(5000)) != testData.size())
				return 0;

			if(testRead.at(0) != 5 || testRead.at(1) != 6)
				return 0;

			// size_t WriteTimeout(const byte_t *NetInput, size_t InputSize, timeout_ms_t Timeout = STRING_TIMEOUT_MS)
			testServer.WriteTimeout(testData.data(), testData.size(), timeout_ms_t(5000));
			if(testClient.Read(testRead.data(), testRead.size() != testData.size()))
				return 0;

			if(testRead.at(0) != 5 || testRead.at(1) != 6)
				return 0;

			// template<class T>
			// T Read();
			testServer.Write(testData.data(), 1);

			const byte_t testReadByte = testClient.Read<byte_t>();
			if(testReadByte != testData.at(0))
				return 0;

			// template<class T>
			// void Write(const T &);
			const float testWriteData = 4.3f;
			testServer.Write<float>(testWriteData);

			if(testClient.Read<float>() != testWriteData)
				return 0;

			// void WriteString(const string &S, timeout_ms_t Timeout = STRING_TIMEOUT_MS);
			// string ReadString(timeout_ms_t Timeout = STRING_TIMEOUT_MS)
			const string testWriteString = "testWrite";
			testServer.WriteString(testWriteString);
			const auto testReadString = testClient.ReadString();

			if(testReadString.compare(testWriteString) != 0)
				return 0;

			// NetworkConnectionUniquePtr MoveOwnership()
			auto testMove = testServer.MoveOwnership();

			return 1;
		}
		catch(Exception&)
		{
			return 0;
		}
	}
}
