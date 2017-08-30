#ifndef TLS_CONNECTION_H
#define TLS_CONNECTION_H

/*! \file tls_connection.h
 *  \brief Header for TLSConnection class
 */

#include "network_connection.h"
#include "error_exception.h"
#include "crypto_x509_certificate_chain.h"
#include "crypto_x509_private_key.h"
#include "secure_connection.h"
#include "crypto_tls_session.h"
#include "crypto_tls_certificate_credentials.h"

#include <gnutls/gnutls.h>
#include <functional>

/*!
 *  \brief Namespace for TLSConnection class
 */
namespace tls_connection
{
	using crypto_x509_private_key::X509PrivateKey;
	using crypto_x509_certificate_chain::X509CertificateChain;

	using crypto_tls_session::TLSSession;

	using crypto_tls_certificate_credentials::TLSCertificateCredentials;

	using network_connection::byte_vector_t;
	using network_connection::vector_size_t;

	using std::function;

	using network_connection::connection_side_t;
	using network_connection::NetworkConnectionUniquePtr;

	using secure_connection::SecureConnection;

	/*!
	 * \brief Position of handshake function hook
	 */
	enum tls_handshake_post_t
	{
		TLS_HOOK_PRE = GNUTLS_HOOK_PRE,
		TLS_HOOK_POST = GNUTLS_HOOK_POST,
		TLS_HOOK_BOTH = GNUTLS_HOOK_BOTH
	};

	/*!
	 * \brief The TLSConnection class
	 */
	class TLSConnection : public SecureConnection
	{
			/*!
			 *	\brief Handshake Functions
			 */
			using hook_fcn_type = int(gnutls_session_t, unsigned int htype, unsigned post, unsigned int incoming, const gnutls_datum_t *msg);

		public:

			/*!
			 *	\brief Handshake Functions
			 */
			using hook_fcn_t = function<hook_fcn_type>;

			/*!
			 * \brief Starts a new TLS connection with the given NetworkConnection. Requires that the TLSCredentials are set separately
			 */
			static TLSConnection SetupNetworkConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide);

			/*!
			 * \brief Starts a new TLS connection with all necessary information
			 */
			static TLSConnection SetupTLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide, TLSCertificateCredentials &&CertificateCredentials);

			TLSConnection(const TLSConnection &) = delete;
			TLSConnection &operator=(const TLSConnection &) = delete;

			TLSConnection(TLSConnection &&) noexcept;
			TLSConnection &operator=(TLSConnection &&) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~TLSConnection() noexcept;

			/*!
			 * \brief Perform TLS handshake
			 * \return Returns whether handshake succeeded
			 */
			bool Handshake();

			/*!
			 * \brief Read from network connection
			 * \param Output Vector where data is stored
			 * \return Returns amount of bytes read
			 */
			size_t Read(byte_t *NetOutput, size_t OutputSize);

			/*!
			 * \brief Write to network connection
			 * \param Input Vector where data is stored
			 * \return Returns amount of bytes written
			 */
			size_t Write(const byte_t *NetInput, size_t InputSize);

			/*!
			 * \brief Close connection again
			 */
			void Close();

			bool IsReadDataAvailable();

			/*!
			 * \brief Access to TLS credentials
			 */
			const TLSCertificateCredentials &GetTLSCredentials() const;

			/*!
			 * \brief Access to TLS credentials
			 */
			TLSCertificateCredentials &GetTLSCredentials();

			/*!
			 * \brief Releases TLSCertificateCredentials
			 */
			TLSCertificateCredentials &&ReleaseCredentials();

		protected:			
			/*!
			 * \brief Set the Trust Chain for authentication of other side
			 */
			void AddTrustChain(X509CertificateChain &&TrustChain);

			/*!
			 * \brief Set underlying Connection
			 */
			void SetConnection(NetworkConnectionUniquePtr &&Connection);

			/*!
			 * \brief Set Certificate Chain and key that this side uses for authentication
			 */
			void AddCertChainAndKey(X509CertificateChain &&CertChain, X509PrivateKey &&PrivateKey);

			/*!
			 * \brief Set a function that hooks into the handshake procedure
			 * \param HandshakeStep Step to which HookFcn will be hooked
			 * \param Post Should HookFcn be executed before or after HandshakeStep
			 * \param HookFcn The function that will be performed during handshake
			 */
			void SetHandshakeFunction(gnutls_handshake_description_t HandshakeStep, tls_handshake_post_t Post, const hook_fcn_t HookFcn) noexcept;

			/*!
			 * \brief Unsecured connection to add TLS encryption to
			 */
			NetworkConnectionUniquePtr _ConnectionPtr;

			virtual NetworkConnectionUniquePtr Move();

		private:
			/*!
			 * \brief TLS session data
			 */
			TLSSession _Session;

			/*!
			 * \brief Credentials for this TLS connection
			 */
			TLSCertificateCredentials _Credentials;

			/*!
			 * \brief Basic session setup
			 */
			void SessionSetup();

			/*!
			 * \brief Tell _Session to use _Credentials
			 */
			void AssociateSessionAndCredentials();

			/*!
			 * \brief Set underlying Connection
			 */
			void SetConnection() noexcept;

			/*!
			 * \brief Push Function for TLS session
			 * \param WriteData Data to write
			 * \param WriteSize Maximum Size of data to write
			 * \return Returns written size or -1 for errors
			 */
			ssize_t PushFcn(const byte_t *WriteData, size_t WriteSize);
			//ssize_t PushVecFcn(const giovec_t *iov, int iovcnt);

			/*!
			 * \brief Pull Function for TLS session
			 * \param ReadData Data to read
			 * \param ReadSize Maximum Size of data to read
			 * \return Returns read size or -1 for errors
			 */
			ssize_t PullFcn(byte_t *ReadData, size_t ReadSize);

			/*!
			 * \brief Waits for a certain time
			 * \param Milliseconds Wait Time in ms
			 * \return Returns 0 in timeout
			 */
			int PullWaitTimeFcn(unsigned int MilliSeconds);

			/*!
			 * \brief Constructor
			 * \param Connection Connection to use for sending/receiving data
			 * \param ConnectionSide Is this client or server
			 */
			TLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide);

			/*!
			 * \brief Constructor
			 * \param Connection Connection to use for sending/receiving data
			 * \param ConnectionSide Is this client or server
			 * \param CertificateCredentials Credentials for TLS connection
			 */
			TLSConnection(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide, TLSCertificateCredentials &&CertificateCredentials);

			/*!
			 * \brief Constructor
			 * \param Connection Connection to use for sending/receiving data
			 * \param TrustChain CA chain to be used for verification
			 * \param CertChain Certificate chain that is used for authentication
			 * \param PrivateKey Private Key that corresponds to _CertChains first entry
			 * \param ConnectionSide Is this the server or client
			 * \param PerformHandshake Should the handshake be performed by the constructor?
			 *
			 * TLSConnection will take ownership of the tcp connection, all chains and the password. This is done because gnutls will free certs and key
			 */
			TLSConnection(NetworkConnectionUniquePtr &&Connection, X509CertificateChain &&TrustChain, X509CertificateChain &&CertChain, X509PrivateKey &&PrivateKey, connection_side_t ConnectionSide, bool PerformHandshake = 1);
	};
}
// ~tls_connection


#endif // TLS_CONNECTION_H
