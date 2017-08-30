#ifndef SECURE_CONNECTION_H
#define SECURE_CONNECTION_H

/*! \file secure_connection.h
 *  \brief Header for SecureConnection class
 */


#include "network_connection.h"

/*!
 *  \brief Namespace for SecureConnection class
 */
namespace secure_connection
{
	using network_connection::NetworkConnection;

	using network_connection::vector_size_t;
	using network_connection::byte_vector_t;

	/*!
	 * \brief The SecureConnection class
	 */
	class SecureConnection : public NetworkConnection
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			SecureConnection() = default;

			SecureConnection(const SecureConnection &) = delete;
			SecureConnection &operator=(const SecureConnection &) = delete;

			SecureConnection(SecureConnection &&) noexcept = default;
			SecureConnection &operator=(SecureConnection &&) noexcept = default;

			/*!
			 * \brief Destructor
			 */
			virtual ~SecureConnection() noexcept = 0;

			virtual bool Handshake() = 0;

			virtual size_t Read(byte_t *NetOutput, size_t OutputSize) = 0;

			virtual size_t Write(const byte_t *NetInput, size_t InputSize) = 0;

			virtual void Close() = 0;

		private:
	};
} // namespace secure_connection


#endif // SECURE_CONNECTION_H
