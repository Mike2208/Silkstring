#ifndef CRYPTO_X509_CERTIFICATE_H
#define CRYPTO_X509_CERTIFICATE_H

/*! \file crypto_x509_certificate.h
 *  \brief Header for X509Certificate class
 */


#include <gnutls/x509.h>
#include <string>
//#include <nlohmann/json.hpp>
#include "crypto_header.h"
#include "typesafe_class.h"
#include "crypto_x509_private_key.h"
#include "crypto_x509_id.h"

//namespace crypto_x509_certificate_request
//{
//class X509CertificateRequest;
//}

/*!
 *  \brief Namespace for X509Certificate class
 */
namespace crypto_x509_certificate
{
//	using nlohmann::json;

	using crypto_x509_private_key::X509PrivateKey;

	using crypto_x509_id::X509ID;

	using crypto_header::DefIDHashAlgorithm;

	using gnutls_x509_crt_t = gnutls_x509_crt_t;

	class TestX509Certificate;
	class TestX509PublicKeyID;
	class TestX509CertificateRequest;
	class TestX509CertificateID;


	//using vector_t::byte_vector_t;
	using std::string;

	enum dn_field_element_t
	{
		DN_COUNTRY = 1,
		DN_ORGANIZATION,
		DN_ORGANIZATIONAL_UNIT,
		DN_COMMON_NAME,
		DN_LOCALITY,
		DN_PROVINCE
	};

	enum x509_cert_version_t
	{
		X509_VERSION_1 = 1,
		X509_VERSION_3 = 3
	};

	using serial_type = long;
	class serial_t
	{
		public:
			constexpr explicit serial_t(serial_type Value) : _Value(Value)
			{}

			GET_OPERATOR(serial_type, _Value)

		private:
			serial_type _Value;
	};

	const auto DefHashAlgo = GNUTLS_DIG_SHA256;
	const auto DefSignAlgo = GNUTLS_DIG_SHA256;

	const string JSONX509CertificateID = "ID";
	const string JSONX509Certificate = "Certificate";

	/*!
	 * \brief The X509Certificate class
	 */
	class X509Certificate
	{
		public:
			/*!
			 * \brief Constructor
			 */
			X509Certificate();

			static X509Certificate ImportFromPEMString(const string &ImportPEMString);
			static X509Certificate ImportGnuTLSCertificate(gnutls_x509_crt_t &&GnuTLSCertificate);

			X509Certificate(const X509Certificate &S);
			X509Certificate &operator=(const X509Certificate &S);
			X509Certificate(X509Certificate &&S) noexcept;
			X509Certificate &operator=(X509Certificate &&S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~X509Certificate() noexcept;

			bool operator==(X509Certificate &S);
			bool operator!=(X509Certificate &S);

			/*!
			 * \brief Exports certificate to string in PEM format
			 * \return Returns PEM string
			 */
			string ExportToPEMString() const;

			/*!
			 * \brief Exports certificate to file in PEM format
			 * \param FileName export file
			 */
			void ExportToPEMFile(const string &FileName);

			operator gnutls_x509_crt_t();

			/*!
			 * \brief Ownership is transferred to returned value. MUST BE DESTROYED MANUALLY
			 * \return Returns ownership to credential
			 */
			gnutls_x509_crt_t ReleaseOwnership();

		private:
			/*!
			 * \brief Data storage for certificate
			 */
			gnutls_x509_crt_t _CertificateData = nullptr;

			/*!
			 * \brief Uninitialize memory
			 */
			void Deinitialize() noexcept;

			/*!
			 * \brief Constructor
			 */
			explicit X509Certificate(gnutls_x509_crt_t &&Certificate);

			/*!
			 * \brief Function to copy class
			 * \param S Class to copy
			 */
			void Copy(const X509Certificate &S);

			friend class X509CertificateRequest;

			friend class TestX509Certificate;
	};

	class X509PublicKeyID;

	/*!
	 * \brief The X509CertificateRequest class
	 */
	class X509CertificateRequest
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			X509CertificateRequest();

			/*!
			 * \brief Imports request from PEM String
			 */
			static X509CertificateRequest ImportFromPEMString(const string &PEMString);

			// Copy constructors
			X509CertificateRequest(const X509CertificateRequest &S);
			X509CertificateRequest &operator=(const X509CertificateRequest &S);

			// Move constructors
			X509CertificateRequest(X509CertificateRequest &&S) noexcept;
			X509CertificateRequest &operator=(X509CertificateRequest &&S) noexcept;

			/*!
			 *	\brief Destructor
			 */
			~X509CertificateRequest() noexcept;

			/*!
			 * \brief Set one field of DN
			 * \param DNField Field to set
			 * \param Entry Entry Value
			 */
			void SetDNField(dn_field_element_t DNField, string Entry);

			/*!
			 * \brief Sets the version
			 * \param Version
			 */
			void SetCertVersion(x509_cert_version_t Version);

			/*!
			 * \brief Set the private key and sign the request with it
			 * \param PrivateKey Key to set as private key
			 * \param DigestAlgorithm Hash algorithm to use
			 */
			void SetAndSignWithPrivateKey(X509PrivateKey &PrivateKey, gnutls_digest_algorithm_t DigestAlgorithm = DefHashAlgo);

			/*!
			 * \brief Sign this request
			 * \param IssuerCertificate Certificate to sign this one with
			 * \param IssuerPrivateKey Private Key of issuer
			 * \param Serial Serial number of new certificate
			 * \param ActivationTime Time from when the certificate is valid
			 * \param ExpiryTime Time when the certificate is no longer valid
			 * \param SignAlgorithm Signature algorithm
			 * \return Returns newly signed certificate
			 */
			X509Certificate SignRequest(X509Certificate &IssuerCertificate, X509PrivateKey &IssuerPrivateKey, serial_t Serial, time_t ActivationTime, time_t ExpiryTime, gnutls_digest_algorithm_t SignAlgorithm = DefSignAlgo);

			/*!
			 * \brief Generate a new self signed certificate with the given options
			 * \param PrivateKey Private key of new certificate
			 * \param Serial Serial number of new certificate
			 * \param ActivationTime Time from when the certificate is valid
			 * \param ExpiryTime Time when the certificate is no longer valid
			 * \param SignAlgorithm Signature algorithm
			 * \return Returns new self-signed certificate
			 */
			X509Certificate GenerateSelfSigned(X509PrivateKey &PrivateKey, serial_t Serial, time_t ActivationTime, time_t ExpiryTime, gnutls_digest_algorithm_t SignAlgorithm = DefSignAlgo);

			/*!
			 * \brief Generates a string with the certificate information
			 * \return Returns string
			 */
			string ExportToPEMString();

			/*!
			 * \brief Export to PEM file
			 * \param FileName File Name
			 */
			void ExportToPEMFile(const string &FileName);

		private:
			/*!
			 * \brief Pointer to certificate data
			 */
			gnutls_x509_crq_t _CertRequest = nullptr;

			/*!
			 * \brief Free memory
			 */
			void Deinitialize() noexcept;

			/*!
			 * \brief Allocate memory
			 */
			void Initialize();

			/*!
			 * \brief Copy Request
			 */
			void Copy(const X509CertificateRequest &S);

			/*!
			 * \brief Generate a new self signed certificate with the given options
			 * \param ActivationTime Time from when the certificate is valid
			 * \param ExpiryTime Time when the certificate is no longer valid
			 * \param Serial Serial number of new certificate
			 * \return Returns new self-signed certificate
			 */
			X509Certificate CreateCertificate(serial_t Serial, time_t ActivationTime, time_t ExpiryTime);

			friend class TestX509CertificateRequest;
	};

	class X509CertificateID : public X509ID
	{
		public:
			/*!
			 * \brief Constructor
			 */
			X509CertificateID() = default;

			/*!
			 * \brief Constructor
			 * \param Certificate Certificate to get ID from
			 */
			static X509CertificateID ImportFromCertificate(const X509Certificate &Certificate, gnutls_digest_algorithm_t Algorithm = DefHashAlgo);

			/*!
			 * \brief Constructor
			 * \param HexString String as Hex value
			 */
			static X509CertificateID ImportFromHexString(const string &HexString);

			static X509CertificateID ImportFromIDArray(id_array_t &&IDArray);

			/*!
			 * \brief Constructor
			 * \param StringID ID of certificate as string
			 */
			//explicit X509CertificateID(const id_array_t &StringID) noexcept;

			bool operator==(const X509CertificateID &Other) const noexcept;
			bool operator!=(const X509CertificateID &Other) const noexcept;

		private:

			/*!
			 * \brief Constructor
			 */
			X509CertificateID(X509ID &&ID);

			friend class TestX509CertificateID;
	};

	class X509PublicKeyID : public X509ID
	{
		public:
			/*!
			 * \brief Constructor
			 */
			X509PublicKeyID() = default;

			/*!
			 * \brief Constructor
			 * \param Certificate Certificate to get ID from
			 */
			static X509PublicKeyID ImportFromCertificate(const X509Certificate &Certificate, gnutls_keyid_flags_t Algorithm = DefIDHashAlgorithm);

			/*!
			 * \brief Constructor
			 * \param HexString String as Hex value
			 */
			static X509PublicKeyID ImportFromHexString(const string &HexString);

			/*!
			 * \brief Constructor
			 * \param StringID ID of certificate as string
			 */
			static X509PublicKeyID ImportFromIDArray(id_array_t &&StringID) noexcept;

			bool operator==(const X509PublicKeyID &Other) const noexcept;
			bool operator!=(const X509PublicKeyID &Other) const noexcept;

		private:

			X509PublicKeyID(X509ID &&ID);

			friend class TestX509PublicKeyID;
	};

	static const X509PublicKeyID ErrorID = X509PublicKeyID();

} // namespace crypto_x509_certificate

// JSON Serialization
//namespace nlohmann
//{
//	template <>
//	struct adl_serializer<crypto_x509_certificate::X509CertificateID>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static crypto_x509_certificate::X509CertificateID from_json(const json &J)
//		{
//			return crypto_x509_certificate::X509CertificateID(J.at(crypto_x509_certificate::JSONX509CertificateID).get<crypto_x509_certificate::string>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const crypto_x509_certificate::X509CertificateID &T)
//		{
//			J = json{ crypto_x509_certificate::JSONX509CertificateID, T.ConvertToString() };
//		}
//	};

//	template <>
//	struct adl_serializer<crypto_x509_certificate::X509Certificate>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static crypto_x509_certificate::X509Certificate from_json(const json &J)
//		{
//			return crypto_x509_certificate::X509Certificate(J.at(crypto_x509_certificate::JSONX509Certificate).get<crypto_x509_certificate::string>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const crypto_x509_certificate::X509Certificate &T)
//		{
//			J = json{ crypto_x509_certificate::JSONX509Certificate, T.ExportToPEMString() };
//		}
//	};
//}

#endif // CRYPTO_X509_CERTIFICATE_H
