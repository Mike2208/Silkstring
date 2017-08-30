#ifndef STRING_TLS_CONNECTION_H
#define STRING_TLS_CONNECTION_H

/*! \file string_tls_connection.h
 *  \brief Header for StringTLSClientConnection class
 */


#include "tls_connection.h"
#include "network_connection.h"
#include "string_user_admin.h"
#include "typesafe_class.h"
#include "string_thread.h"

#ifdef __linux__
#include "linux_network_socket.h"
#endif

#include <memory>
#include <functional>

/*!
 *  \brief Namespace for StringTLSClientConnection class
 */
namespace string_tls_connection
{
//	using tls_connection::TLSConnection;

//	using network_socket::NetworkSocketTCPClient;

//	using string_manager::StringUserManager;
//	using string_user::StringUser;
//	using string_user_admin::StringUserAdmin;

//	using std::function;
//	using std::bind;
//	using std::string;

//	using std::shared_ptr;

//	using network_connection::timeout_ms_t;

//	using tls_connection::TLS_HOOK_BOTH;
//	using tls_connection::TLS_HOOK_PRE;
//	using tls_connection::TLS_HOOK_POST;

//	const string AcceptString = "OK";
//	const string DenyString = "NOT";
//	const string AbortString = "ABORT";

//	constexpr gnutls_handshake_description_t ClientCARequestPos = GNUTLS_HANDSHAKE_CLIENT_HELLO;
//	constexpr auto ClientCARequestPre = TLS_HOOK_POST;

//	/*!
//	 * \brief Base class for both client- and server-side connection
//	 */
//	class StringTLSBaseConnection : public TLSConnection
//	{
//		public:
//			/*!
//			 * 	\brief Constructor
//			 */
//			StringTLSBaseConnection(NetworkSocketTCPClient &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups, hook_fcn_t HookFunction);

//		protected:
//			/*!
//			 *	\brief Hooks into handshake process and sends possible certificate IDs.
//			 */
//			int SendCertificateInfoHookFcn(gnutls_session_t, unsigned int htype, unsigned post, unsigned int incoming, const gnutls_datum_t *msg);

//			/*!
//			 * \brief Hooks into handshake process and receives possible certificate IDs. If a match is found, report it to other side
//			 */
//			int ReceiveCertificateInfoHookFcn(gnutls_session_t, unsigned int htype, unsigned post, unsigned int incoming, const gnutls_datum_t *msg);

//			hook_fcn_t _HookFunction;

//		private:
//			/*!
//			 * \brief Own data used for authentication on other side
//			 */
//			shared_ptr<const StringUserAdmin> _OwnData;

//			/*!
//			 * \brief Data used to verify other side of connection
//			 */
//			shared_ptr<const StringUser> _ConnectionCerts;

//			/*!
//			 * \brief Group certificates
//			 */
//			shared_ptr<const StringUserManager> _Groups;

//			constexpr static gnutls_handshake_description_t _HookPos = ClientCARequestPos;
//			constexpr static auto _HookPre = ClientCARequestPre;

//			constexpr static auto _WaitTimeMS = network_connection::STRING_TIMEOUT_MS;

//			void SendCertificateInfo();
//			void ReceiveCertificateInfo();
//	};

//	/*!
//	 * \brief The StringTLSClientConnection class
//	 */
//	class StringTLSClientConnection : public StringTLSBaseConnection
//	{
//		public:
//			/*!
//			 * 	\brief Constructor
//			 */
//			StringTLSClientConnection(NetworkSocketTCPClient &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups);

//			StringTLSClientConnection(StringTLSClientConnection &&S) noexcept;
//			StringTLSClientConnection &operator=(StringTLSClientConnection &&S) noexcept;

//		private:
//	};

//	/*!
//	 * \brief The StringTLSServerConnection class
//	 */
//	class StringTLSServerConnection : public StringTLSBaseConnection
//	{
//		public:
//			/*!
//			 * 	\brief Constructor
//			 */
//			StringTLSServerConnection(NetworkSocketTCPClient &&Connection, const shared_ptr<const StringUserAdmin> &OwnUser, const shared_ptr<const StringUser> &ServerUser, const shared_ptr<const StringUserManager> &Groups);

//			StringTLSServerConnection(StringTLSServerConnection &&S) noexcept;
//			StringTLSServerConnection &operator=(StringTLSServerConnection &&S) noexcept;
//		private:
//	};
} // namespace string_tls_connection


#endif // STRING_TLS_CONNECTION_H
