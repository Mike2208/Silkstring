#ifndef CRYPTO_X509_CERTIFICATE_CHAIN_H
#define CRYPTO_X509_CERTIFICATE_CHAIN_H

/*! \file crypto_x509_certificate_chain.h
 *  \brief Header for X509CertificateChain class
 */


#include "crypto_x509_certificate.h"
#include "vector_t.h"
#include <string>

/*!
 *  \brief Namespace for X509CertificateChain class
 */
namespace crypto_x509_certificate_chain
{
	using crypto_x509_certificate::X509Certificate;
	using crypto_x509_certificate::X509CertificateID;
	using crypto_x509_certificate::gnutls_x509_crt_t;

	using X509CertificateVector = vector_t::vector_type<X509Certificate>;

	using gnutls_cert_vector_t = vector_t::vector_type<gnutls_x509_crt_t>;

	using std::string;

	const string JSONX509CertificateChain = "X509CertificateChain";

	class TestX509CertificateChain;

	/*!
	 * \brief The X509CertificateChain class
	 */
	class X509CertificateChain
	{
		public:
			X509CertificateChain() = default;

			/*!
			 * 	\brief Constructor
			 */
			template<class... T>
			static X509CertificateChain ImportCerts(T &&...Certificates);

			/*!
			 * \brief Constructor
			 * \param ImportChain
			 */
			static X509CertificateChain ImportFromPEMStringChain(const string &ImportPEMChain);

			X509CertificateChain(const X509CertificateChain &S) = default;
			X509CertificateChain(X509CertificateChain &&S) = default;
			X509CertificateChain &operator=(const X509CertificateChain &S) = default;
			X509CertificateChain &operator=(X509CertificateChain &&S) = default;

			/*!
			 * \brief Gets credentials as a vector
			 * \param MoveOwnership Should ownership be moved to returned vector?
			 * \return Returns vector with credentials
			 */
			gnutls_cert_vector_t GetCertVector(bool MoveOwnership);
			size_t GetNumCerts() const;

			X509CertificateID GetLowestCertID() const;

			string ExportToPEMString() const;

			template<class CertType>
			void AppendElement(CertType &&Cert);

			gnutls_cert_vector_t ReleaseOwnership();

		private:
			/*!
			 * \brief Contains the chain of certificates
			 */
			X509CertificateVector _CertificateChain;

			template<class CertType, class... T>
			void AppendElementChain(CertType &&Cert, T &&...Rest);

			template<class ...T>
			void AppendElementChain(T &&...Rest);

			friend class TestX509CertificateChain;
	};

	template<class... T>
	X509CertificateChain X509CertificateChain::ImportCerts(T &&...Rest)
	{
		X509CertificateChain retVal;

		retVal._CertificateChain.reserve(sizeof...(Rest));
		retVal.AppendElementChain(std::forward<T>(Rest)...);

		return retVal;
	}

	template<class CertType, class... T>
	void X509CertificateChain::AppendElementChain(CertType &&Cert, T &&...Rest)
	{
		static_assert(std::is_same<typename std::remove_const<typename std::remove_reference<CertType>::type>::type, X509Certificate>::value, "ERROR X509CertificateChain::AppendElementChain(): Can only accept X509Certificate as template type\n");

		this->AppendElement(std::forward<CertType>(Cert));
		this->AppendElementChain(std::forward<T>(Rest)...);
	}

	template<class ...T>
	void X509CertificateChain::AppendElementChain(T &&...)
	{}


	template<class CertType>
	void X509CertificateChain::AppendElement(CertType &&Cert)
	{
		static_assert(std::is_same<typename std::remove_const<typename std::remove_reference<CertType>::type>::type, X509Certificate>::value, "ERROR X509CertificateChain::AppendElement(): Can only accept X509Certificate as template type\n");

		this->_CertificateChain.push_back(std::forward<CertType>(Cert));
	}
} // namespace crypto_x509_certificate_chain


// JSON Serialization
//namespace nlohmann
//{
//	template <>
//	struct adl_serializer<crypto_x509_certificate_chain::X509CertificateChain>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static crypto_x509_certificate_chain::X509CertificateChain from_json(const json &J)
//		{
//			return crypto_x509_certificate_chain::X509CertificateChain(J.at(crypto_x509_certificate_chain::JSONX509CertificateChain).get<crypto_x509_certificate_chain::string>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const crypto_x509_certificate_chain::X509CertificateChain &T)
//		{
//			J = json{ crypto_x509_certificate_chain::JSONX509CertificateChain, T.ExportToPEMString() };
//		}
//	};
//}

#endif // CRYPTO_X509_CERTIFICATE_CHAIN_H
