#ifndef USER_CERT_ID_H
#define USER_CERT_ID_H

/*! \file user_cert_id.h
 *  \brief Header for UserCertID class
 */


#include "crypto_header.h"
#include "crypto_x509_certificate.h"
#include "vector_t.h"

//#include <nlohmann/json.hpp>
#include <string>

/*!
 *  \brief Namespace for UserCertID class
 */
namespace user_cert_id
{
	using crypto_x509_certificate::X509Certificate;
	using crypto_x509_certificate::X509CertificateID;

	using std::string;

	//using vector_t::byte_vector_t;

	using vector_t::vector_type;

//	using nlohmann::json;

	const string JSONUserID = "UserID";
	const string JSONGroupID = "GroupID";
	const string JSONGroupUserID = "GroupUserID";

	/*!
	 * \brief The UserCertID class
	 */
	class UserCertID : public X509CertificateID
	{
		public:
			UserCertID() = default;

			static UserCertID ImportFromCertificate(const X509Certificate &Cert);

			static UserCertID ImportFromIDArray(id_array_t &&ID) noexcept;

		private:

			explicit UserCertID(X509CertificateID &&ID) noexcept;
	};

	using UserCertIDVector = vector_t::vector_t<UserCertID>;
} // namespace user_cert_id


// JSON Serialization
//namespace nlohmann
//{
//	template <>
//	struct adl_serializer<user_cert_id::UserCertID>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static user_cert_id::UserCertID from_json(const json &J)
//		{
//			user_cert_id::X509CertificateID tmpID = J.at(user_cert_id::JSONUserID);
//			return user_cert_id::UserCertID(std::move(tmpID));
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const user_cert_id::UserCertID &T)
//		{
//			json tmpJSON = static_cast<user_cert_id::X509CertificateID>(T);
//			J = json{ user_cert_id::JSONUserID, tmpJSON };
//		}
//	};
//}

#endif // USER_CERT_ID_H
