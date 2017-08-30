#ifndef CRYPTO_X509_PRIVATE_KEY_H
#define CRYPTO_X509_PRIVATE_KEY_H

/*! \file crypto_x509_private_key.h
 *  \brief Header for X509PrivateKey class
 */


#include "typesafe_class.h"
#include "crypto_header.h"
#include "crypto_x509_id.h"
#include <gnutls/x509.h>
#include <string>
//#include <nlohmann/json.hpp>

/*!
 *  \brief Namespace for X509PrivateKey class
 */
namespace crypto_x509_private_key
{
	using std::string;

	using crypto_header::DefIDHashAlgorithm;

	using crypto_x509_id::X509ID;

	using key_bits_type = unsigned int;
	class key_bits_t
	{
		public:
			explicit constexpr key_bits_t(key_bits_type Value) : _Value(Value)
			{}

			GET_OPERATOR(key_bits_type, _Value)

		private:
			key_bits_type _Value;
	};

	const gnutls_pk_algorithm_t DefAlgorithmType = GNUTLS_PK_RSA;
	const key_bits_t DefKeyBits(gnutls_sec_param_to_pk_bits(DefAlgorithmType, GNUTLS_SEC_PARAM_HIGH));

	const string JSONX509PrivateKey;

	class TestX509PrivateKey;
	class TestX509PrivateKeyID;

	/*!
	 * \brief The X509PrivateKey class
	 */
	class X509PrivateKey
	{
		public:
			/*!
			 * 	\brief Constructor. Create new private key
			 */
			X509PrivateKey();

			/*!
			 * \brief Constructor
			 * \param ImportPEMKey
			 */
			static X509PrivateKey ImportFromPEMString(const string &ImportPEMKey);

			/*!
			 * \brief Generate a key
			 * \param AlgorithmType Key Type (default is RSA)
			 * \param NumBits Key Size
			 */
			static X509PrivateKey GenerateKey(gnutls_pk_algorithm_t AlgorithmType = DefAlgorithmType, key_bits_t NumBits = DefKeyBits);

			// Copy Constructors
			X509PrivateKey(const X509PrivateKey &S);
			X509PrivateKey &operator=(const X509PrivateKey &S);

			// Move Constructor
			X509PrivateKey(X509PrivateKey &&S) noexcept;
			X509PrivateKey &operator=(X509PrivateKey && S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~X509PrivateKey() noexcept;

			/*!
			 * \brief Exports key to PEM string
			 * \return Returns PEM string
			 */
			string ExportToPEMString() const;

			/*!
			 * \brief Saves key to PEM file
			 * \param FileName File Name  to export to
			 */
			void ExportToPEMFile(const string &FileName);

			/*!
			 * \brief Returns pointer to private key data
			 */
			operator gnutls_x509_privkey_t() noexcept;

			/*!
			 * \brief Returns pointer to private key data
			 */
			operator const gnutls_x509_privkey_t&() const noexcept;

			gnutls_x509_privkey_t ReleaseOwnership() noexcept;

		private:
			/*!
			 * \brief Pointer to private key data
			 */
			gnutls_x509_privkey_t _KeyData = nullptr;

			/*!
			 * \brief Free memory
			 */
			void Deinitialize() noexcept;

			/*!
			 * \brief Allocate memory
			 */
			void Initialize();

			/*!
			 * \brief Copy Key
			 */
			void Copy(const X509PrivateKey &S);
	};

	class X509PrivateKeyID : public X509ID
	{
		public:
			X509PrivateKeyID() = default;

			static X509PrivateKeyID ImportFromPrivateKey(X509PrivateKey &PrivateKey, gnutls_keyid_flags_t Algorithm = DefIDHashAlgorithm);

			/*!
			 * \brief Constructor
			 * \param HexString ID as string
			 */
			static X509PrivateKeyID ImportFromHexString(const string &HexString);

		private:

			X509PrivateKeyID(X509ID &&ID);
	};
} // namespace crypto_x509_private_key

// JSON Serialization
//namespace nlohmann
//{
//	template <>
//	struct adl_serializer<crypto_x509_private_key::X509PrivateKey>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static crypto_x509_private_key::X509PrivateKey from_json(const json &J)
//		{
//			return crypto_x509_private_key::X509PrivateKey(J.at(crypto_x509_private_key::JSONX509PrivateKey).get<crypto_x509_private_key::string>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const crypto_x509_private_key::X509PrivateKey &T)
//		{
//			J = json{ crypto_x509_private_key::JSONX509PrivateKey, T.ExportToPEMString() };
//		}
//	};
//}


#endif // CRYPTO_X509_PRIVATE_KEY_H
