#ifndef CRYPTO_TLS_SESSION_H
#define CRYPTO_TLS_SESSION_H

/*! \file crypto_tls_session.h
 *  \brief Header for TLSSession class
 */


#include <gnutls/gnutls.h>

/*!
 *  \brief Namespace for TLSSession class
 */
namespace crypto_tls_session
{
	class TestTLSSession;

	/*!
	 * \brief The TLSSession class
	 */
	class TLSSession
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			TLSSession(gnutls_init_flags_t InitFlags);

			TLSSession(const TLSSession &S);
			TLSSession &operator=(const TLSSession &S);

			TLSSession(TLSSession &&S) noexcept;
			TLSSession &operator=(TLSSession &&S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~TLSSession() noexcept;

			operator gnutls_session_t() const;
			operator gnutls_session_t&();

		private:
			gnutls_session_t _Session = nullptr;

			void Copy(const TLSSession &S);

			void Initialize(gnutls_init_flags_t InitFlags);
			void Deinitialize() noexcept;

			friend class TestTLSSession;
	};
} // namespace crypto_tls_session


#endif // CRYPTO_TLS_SESSION_H
