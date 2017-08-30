#ifndef CRYPTO_TLS_CERTIFICATE_CREDENTIALS_H
#define CRYPTO_TLS_CERTIFICATE_CREDENTIALS_H

/*! \file crypto_tls_certificate_credentials.h
 *  \brief Header for TLSCertificateCredentials class
 */


#include <gnutls/gnutls.h>
#include "crypto_x509_certificate_chain.h"
#include "crypto_x509_private_key.h"

/*!
 *  \brief Namespace for TLSCertificateCredentials class
 */
namespace crypto_tls_certificate_credentials
{
	using vector_t::vector_t;

	using crypto_x509_private_key::X509PrivateKey;
	using crypto_x509_certificate_chain::X509CertificateChain;

	class TestTLSCertificateCredentials;

	/*!
	 * \brief The TLSCertificateCredentials class
	 */
	class TLSCertificateCredentials
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			TLSCertificateCredentials();

			TLSCertificateCredentials(const TLSCertificateCredentials &S) = delete;
			TLSCertificateCredentials &operator=(const TLSCertificateCredentials &S) = delete;

			TLSCertificateCredentials(TLSCertificateCredentials &&S);
			TLSCertificateCredentials &operator=(TLSCertificateCredentials &&S);

			~TLSCertificateCredentials();

			operator gnutls_certificate_credentials_t();
			operator const gnutls_certificate_credentials_t() const;

			/*!
			 * \brief Add the given Trust Credentials to this storage
			 * \param CertificateChain Chain to add to storage
			 */
			void AddTrustCredentials(X509CertificateChain &&CertificateChain);

			/*!
			 * \brief Add the given Key Credentials to this storage
			 * \param CertificateChain Chain to add to storage
			 * \param PrivateKey Key associated with last element of CertificateChain
			 */
			void AddKeyCredentials(X509CertificateChain &&CertificateChain, X509PrivateKey &&PrivateKey);

			/*!
			 * \brief Removes all credentials stored here
			 */
			void RemoveAllCredentials();

		private:

			gnutls_certificate_credentials_t	_Credentials = nullptr;

			friend class TestTLSCertificateCredentials;
	};
} // namespace crypto_tls_certificate_credentials


#endif // CRYPTO_TLS_CERTIFICATE_CREDENTIALS_H
