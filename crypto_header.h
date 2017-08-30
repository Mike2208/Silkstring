#ifndef CRYPTO_HEADER_H
#define CRYPTO_HEADER_H

#include <string>
#include <gnutls/x509.h>

namespace crypto_header
{
	/*!
	 * \brief Default hash algorithm for ID generation
	 */
	const gnutls_keyid_flags_t DefIDHashAlgorithm = GNUTLS_KEYID_USE_SHA256;

	/*!
	 * \brief Default size of ID in bytes
	 */
	const size_t DefIDSize = 256/8;
}

#endif // CRYPTO_HEADER_H
