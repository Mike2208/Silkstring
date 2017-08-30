#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

/*! \file network_connection.h
 *  \brief Header for NetworkConnection class
 */

#include "vector_t.h"
#include "typesafe_class.h"
#include <limits>
#include <string>
#include <memory>

/*!
 *  \brief Namespace for NetworkConnection class
 */
namespace network_connection
{
	/*!
	 *	\brief Type for single byte
	 */
	using byte_t = vector_t::byte_t;

	/*!
	 *	\brief Vector type for class NetworkConnection
	 */
	using vector_t::vector_t;

	/*!
	 *	\brief Vector for read/write operations
	 */
	using byte_vector_t = vector_t<byte_t>;

	/*!
	 *	\brief Type for size of vector
	 */
	using vector_size_t = byte_vector_t::size_type;

	using std::string;
	using std::unique_ptr;

	/*!
	 * \brief Is this client or server socket?
	 */
	enum connection_side_t
	{
		CLIENT_SIDE,
		SERVER_SIDE
	};

	using timeout_ms_type = float;
	class timeout_ms_t
	{
		public:
			constexpr explicit timeout_ms_t(timeout_ms_type Time) : _Value(Time) {}

			ARITHMETIC_OPERATORS(timeout_ms_t, _Value)

			GET_OPERATOR(timeout_ms_type, _Value)

			EQUALITY_OPERATORS(timeout_ms_t, _Value)
			COMPARISON_OPERATORS(timeout_ms_t, _Value)

		private:
			timeout_ms_type _Value;
	};

	/*!
	 * \brief Length to read at once (1 so as not to read too much)
	 */
	constexpr size_t STRING_READ_LENGTH = 1;

	/*!
	 * \brief Wait time between string reads
	 */
	constexpr timeout_ms_t STRING_WAIT_TIME_MS(10);

	/*!
	 * \brief Timeout after which string read is cancelled
	 */
	constexpr timeout_ms_t STRING_TIMEOUT_MS(500);

	/*!
	 * \brief Contains information regarding network
	 */
	struct ConnectionInformation
	{
	};

	class TestNetworkConnection;

	/*!
	 * \brief Interface for connections to networks
	 */
	class NetworkConnection
	{
		public:
			using byte_t = network_connection::byte_t;
			using NetworkConnectionUniquePtr = unique_ptr<NetworkConnection>;

			/*!
			 * \brief Constructor
			 */
			NetworkConnection();

			NetworkConnection(const NetworkConnection &S) = delete;
			NetworkConnection &operator=(const NetworkConnection &S) = delete;

			NetworkConnection(NetworkConnection &&S) noexcept = default;
			NetworkConnection &operator=(NetworkConnection &&S) noexcept = default;

			/*!
			 * \brief Destructor
			 */
			virtual ~NetworkConnection() = 0;

			/*!
			 * \brief Close connection again
			 */
			virtual void Close() = 0;

			/*!
			 * \brief Returns whether data was received that hasn't been read yet
			 */
			virtual bool IsReadDataAvailable() = 0;

			/*!
			 * \brief Read from network connection
			 * \param NetOutput Pointer to where data is stored
			 * \return Returns amount of bytes read from network
			 */
			virtual size_t Read(byte_t *NetOutput, size_t OutputSize) = 0;

			/*!
			 * \brief Write to network connection
			 * \param NetInput Pointer to where data is stored
			 * \return Returns amount of bytes written to network
			 */
			virtual size_t Write(const byte_t *NetInput, size_t InputSize) = 0;

			/*!
			 * \brief Reads from network connection
			 * \param OutputSize Amount of bytes to read from network output
			 * \return Returns byte vector with data
			 */
			byte_vector_t Read(size_t OutputSize);

			/*!
			 * \brief Read from network connection. Stop after Timeout time
			 * \param NetOutput Pointer to where data is stored
			 * \return Returns amount of bytes read from network
			 */
			size_t ReadTimeout(byte_t *NetOutput, size_t OutputSize, timeout_ms_t Timeout = STRING_TIMEOUT_MS);

			/*!
			 * \brief Write to network connection. Stop after Timeout time
			 * \param NetInput Pointer to where data is stored
			 * \return Returns amount of bytes written to network
			 */
			size_t WriteTimeout(const byte_t *NetInput, size_t InputSize, timeout_ms_t Timeout = STRING_TIMEOUT_MS);

			/*!
			 * \brief Read function
			 * \param NetOutput class that should be read from network
			 */
			template<class T>
			T Read();

			/*!
			 * \brief Write function
			 * \param NetInput class that should be written to network
			 */
			template<class T>
			void Write(const T &);

			/*!
			 * \brief Reads String from network
			 * \param Timeout Time before ReadString is terminated (if 0 or negative, it will continue indefinetly)
			 */
			string ReadString(timeout_ms_t Timeout = STRING_TIMEOUT_MS);

			/*!
			 * \brief Writes String to network
			 * \param Timeout Time before ReadString is terminated (if 0 or negative, it will continue indefinetly)
			 */
			void WriteString(const string &S, timeout_ms_t Timeout = STRING_TIMEOUT_MS);

			/*!
			 * \brief Moves ownership of this Network connection
			 * \return Returns pointer to new owner
			 */
			NetworkConnectionUniquePtr MoveOwnership();

		protected:

			virtual NetworkConnectionUniquePtr Move() = 0;

		private:

			friend class TestNetworkConnection;
	};

	using NetworkConnectionUniquePtr = NetworkConnection::NetworkConnectionUniquePtr;

	/*!
	 * \brief Returns current clock time with millisecond accuracy
	 */
	extern timeout_ms_t GetCurrentClockMS();

	template<class T>
	T NetworkConnection::Read()
	{
		T tmpData;

		// Read data from network
		this->Read(reinterpret_cast<byte_t*>(&tmpData), sizeof(T));

		return tmpData;
	}

	template<class T>
	void NetworkConnection::Write(const T &NetOutput)
	{
		// Write data to network
		this->Write(reinterpret_cast<const byte_t*>(&NetOutput), sizeof(T));
	}
} // namespace network_connection


#endif // NETWORK_CONNECTION_H
