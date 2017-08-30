#ifndef STRING_CONNECTION_H
#define STRING_CONNECTION_H

/*! \file string_connection.h
 *  \brief Header for StringConnection class
 */


#include "string_user_admin.h"
#include "string_user.h"
#include "string_protocols.h"
#include "string_connection_functions.h"

#include "secure_connection.h"
#include "tls_connection.h"

#include <functional>

/*!
 *  \brief Namespace for StringConnection class
 */
namespace string_connection
{
	using network_socket::NetworkTCPSocket;

	using std::string;

	using tls_connection::TLSConnection;

	using string_protocols::string_protocols_t;
	using string_protocols::STRING_TCP_CONNECTION;
	using string_protocols::STRING_TLS_CONNECTION;

	using string_connection_functions::ConnectionFlagOptions;
	using string_connection_functions::DefConnectionFlags;

	using crypto_x509_certificate::X509CertificateID;

	/*!
	 * \brief State of a StringConnection<STRING_TCP_CONNECTION>
	 */
	enum tcp_state_t
	{
		/*!
		 *	\brief An error was encountered and TCP state was set to error
		 */
		TCP_ERROR_STATE = -1,
		/*!
		 *	\brief Connection was just established
		 */
		TCP_ESTABLISHED = 1,
		/*!
		 *	\brief Register new connection
		 */
		TCP_REGISTER,
		/*!
		 *	\brief IDs of both sides were exchanged
		 */
		TCP_EXCHANGED_ID,
		/*!
		 *	\brief IDs were accepted on both sides
		 */
		TCP_ID_ACCEPTED,
		/*!
		 *	\brief Connection used to exchange data after IDs were accepted
		 */
		TCP_RAW,
		/*!
		 *	\brief ID was declined on this side
		 */
		TCP_ID_DECLINED_LOCAL,
		/*!
		 *	\brief Other side declined ID
		 */
		TCP_ID_DECLINED_REMOTE,
		/*!
		 *	\brief Both sides declined the ID
		 */
		TCP_ID_DECLINED_BOTH
	};

	const string RawTCPState = "RAW";

	/*!	\class StringConnection
	 *	\brief The StringConnection class
	 */
	template<string_protocols_t ConnectionType>
	class StringConnection;

	/*!
	 *	\brief TCP Connection over a string
	 */
	template<>
	class StringConnection<STRING_TCP_CONNECTION> : public NetworkTCPSocket
	{
		public:
			/*!
			 * \brief Constructor
			 */
			StringConnection(NetworkTCPSocket &&Connection) noexcept;

			/*!
			 * \brief Exchange IDs between this side and the other one, also sets _State to TCP_EXCHANGED_ID
			 * \param OwnID ID of this side
			 * \return Returns ID of other side
			 */
			X509CertificateID ExchangeIDs(const X509CertificateID &OwnID);

			/*!
			 * \brief Sends whether the ID was accepted, and
			 * \return
			 */
			bool ExchangeIDValidity(bool IDAccepted);

			/*!
			 * \brief Sets connection state to TCP_RAW
			 */
			void SetToRawTCPConnection();

		private:
			/*!
			 * \brief State of the connection
			 */
			tcp_state_t _State = TCP_ESTABLISHED;

			/*!
			 * \brief Connection Flags used during communication
			 */
			static const ConnectionFlagOptions _ConnectionFlags; //= DefConnectionFlags;
	};

	/*!
	 *	\brief TLS Connection over a string
	 */
	template<>
	class StringConnection<STRING_TLS_CONNECTION> : public TLSConnection
	{
		public:
			/*!
			 * \brief Constructor
			 */
			StringConnection(StringConnection<STRING_TCP_CONNECTION> &&Connection) noexcept;

		private:
	};
} // namespace string_connection


#endif // STRING_CONNECTION_H
