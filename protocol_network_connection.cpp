#include "protocol_network_connection.h"
#include "error_exception.h"
#include <array>

namespace protocol_network_connection
{
	using error_exception::Exception;
	using error_exception::ERROR_NUM;

	using std::array;

	using network_connection::byte_t;

	const protocol_header_t ProtocolNetworkConnection::_HeaderData = protocol_header_t();

	ProtocolNetworkConnection::ProtocolNetworkConnection(NetworkConnectionUniquePtr &&Connection, protocol_thread_id_t ConnectionID)
		: _Connection(std::move(Connection)),
		  _ID(ConnectionID)
	{}

	ProtocolNetworkConnection::~ProtocolNetworkConnection()
	{}

	protocol_vector_t ProtocolNetworkConnection::ReceiveData()
	{
		// Set Read length if no header was found yet
		if(this->_ReadBuffer.size() < this->_HeaderData.ProtocolHeader.size())
			this->_ReadLength = this->_HeaderData.ProtocolHeader.size() - this->_ReadBuffer.size();

		// Try to read header
		this->AttemptRead();

		// Return empty vector if not enough data was received yet
		if(this->_ReadBuffer.size() < this->_HeaderData.ProtocolHeader.size())
			return protocol_vector_t();

		// Check header
		auto bufferIterator = this->_ReadBuffer.begin();
		auto headerIterator = this->_HeaderData.ProtocolHeader.begin();

		// Find start
		while(*bufferIterator != *headerIterator)
		{
			if(++bufferIterator == this->_ReadBuffer.end())
			{
				// If start not found, erase buffer as data is useless
				this->_ReadBuffer.clear();
				this->_ReadLength = this->_HeaderData.ProtocolHeader.size();

				return protocol_vector_t();
			}
		}

		// Check rest of header
		bufferIterator++;
		headerIterator++;
		while(headerIterator != this->_HeaderData.ProtocolHeader.end())
		{
			if(*(bufferIterator++) != *(headerIterator++))
			{
				throw Exception(ERROR_NUM, "ERROR ProtocolNetworkConnection::ReceiveData(): Invalid protocol header\n");
			}
		}

		// Find legth of data
		string dataSizeString;
		while(1)
		{
			// Try to read next byte if end reached
			if(bufferIterator == this->_ReadBuffer.end())
			{
				this->_ReadLength = 1;
				if(this->AttemptRead() < 1)
					return protocol_vector_t();

				// Reorient iterator as vector data may have been moved
				bufferIterator = this->_ReadBuffer.end() - 1;
			}

			const auto tmpData = *bufferIterator;

			bufferIterator++;

			if(tmpData == this->_HeaderData.ProtocolDataLengthEnd)
				break;

			if(!std::isdigit(static_cast<unsigned char>(tmpData)))
				throw Exception(ERROR_NUM, "ERROR ProtocolNetworkConnection::ReceiveData(): Invalid symbol found in data length\n");

			dataSizeString.push_back(tmpData);
		}

		if(bufferIterator == this->_ReadBuffer.end())
		{
			this->_ReadLength = 1;
			if(this->AttemptRead() < 1)
				return protocol_vector_t();

			// Reorient iterator as vector data may have been moved by read
			bufferIterator = this->_ReadBuffer.end() - 1;
		}

		auto dataSize = std::stoi(dataSizeString, nullptr, 10);

		// Find start of data
		if(*bufferIterator != this->_HeaderData.ProtocolDataStart)
		{
			throw Exception(ERROR_NUM, "ERROR ProtocolNetworkConnection::ReceiveData(): Couldn't find start of data\n");
		}

		bufferIterator++;

		// Check whether all data was read
		if(this->_ReadBuffer.end()-bufferIterator < dataSize+1)
		{
			// If not, try to read rest
			this->_ReadLength = dataSize + 1 - (this->_ReadBuffer.end()-bufferIterator);

			const auto curIteratorPos = bufferIterator - this->_ReadBuffer.begin();
			this->AttemptRead();
			bufferIterator = this->_ReadBuffer.begin() + curIteratorPos;

			// Wait if not all data was received
			if(this->_ReadLength > 0)
				return protocol_vector_t();
		}

		// Read data
		protocol_vector_t retData(bufferIterator, bufferIterator + dataSize);

		bufferIterator += dataSize;

		if(*bufferIterator != this->_HeaderData.ProtocolDataEnd)
		{
			throw Exception(ERROR_NUM, "ERROR ProtocolNetworkConnection::ReceiveData(): Couldn't find end of data\n");
		}

		// Restart reading]
		this->_ReadLength = this->_HeaderData.ProtocolHeader.size();
		this->_ReadBuffer.clear();

		return retData;
	}

	void ProtocolNetworkConnection::SendData(const protocol_vector_t &Data)
	{
		// Generate header
		string header(this->_HeaderData.ProtocolHeader.begin(), this->_HeaderData.ProtocolHeader.end());
		const auto packetSize = std::to_string(Data.size());
		if(packetSize.empty())
			header.append("0");
		else
			header.append(packetSize);
		header.append(1, this->_HeaderData.ProtocolDataLengthEnd);
		header.append(1, this->_HeaderData.ProtocolDataStart);

		// Send header string without ending '\0'
		this->_Connection->WriteTimeout(reinterpret_cast<const byte_t *>(header.c_str()), header.length());

		// Send data
		this->_Connection->WriteTimeout(&Data.front(), Data.size());

		// Send end char
		this->_Connection->Write<decltype(this->_HeaderData.ProtocolDataEnd)>(this->_HeaderData.ProtocolDataEnd);
	}

	void ProtocolNetworkConnection::SetConnection(NetworkConnectionUniquePtr &&Connection)
	{
		this->_Connection = std::move(Connection);
	}

	NetworkConnectionUniquePtr &&ProtocolNetworkConnection::ReleaseOwnership(bool UnregisterID)
	{
		// Unregister ID
		if(UnregisterID)
		{
			if(this->_ID != ConnectionIDInvalid)
			{
				//this->;

				this->_ID = ConnectionIDInvalid;
			}
		}

		return std::move(this->_Connection);
	}

	protocol_thread_id_t ProtocolNetworkConnection::GetID() const
	{
		return this->_ID;
	}

	bool ProtocolNetworkConnection::IsReadDataAvailable()
	{
		return this->_Connection->IsReadDataAvailable();
	}

	size_t ProtocolNetworkConnection::AttemptRead()
	{
		size_t readSize = 0;

		if(this->_ReadLength > 0)
		{
			const auto oldSize = this->_ReadBuffer.size();

			this->_ReadBuffer.resize(this->_ReadBuffer.size()+this->_ReadLength);

			readSize = this->_Connection->Read(&(this->_ReadBuffer.at(oldSize)), this->_ReadLength);

			this->_ReadBuffer.resize(oldSize + readSize);

			this->_ReadLength -= readSize;
		}

		return readSize;
	}

	void ProtocolNetworkConnection::SendDataHandle(protocol_vector_t &Data)
	{
		this->_Connection->WriteTimeout(Data.data(), Data.size());
	}
}
