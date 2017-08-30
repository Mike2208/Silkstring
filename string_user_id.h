#ifndef STRING_USER_ID_H
#define STRING_USER_ID_H

/*! \file string_user_id.h
 *  \brief Header for StringUserID class
 */


#include "crypto_x509_certificate.h"
#include <nlohmann/json.hpp>
#include <memory>

/*!
 *  \brief Namespace for StringUserID class
 */
namespace string_user_id
{
	using crypto_x509_certificate::X509PublicKeyID;

	using std::string;
	using std::shared_ptr;

	const string JSONStringUserID = "StringUserID";

	/*!
	 * \brief ID of string users
	 */
	class StringUserID : public X509PublicKeyID
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			StringUserID(const X509PublicKeyID &_StringUserID);

			StringUserID() = default;

			explicit StringUserID(const string &_StringUserID);

			bool operator==(const StringUserID &S) const noexcept;
			bool operator!=(const StringUserID &S) const noexcept;

		private:
	};

	using StringUserIDSharedPtr = shared_ptr<StringUserID>;
} // namespace string_user_id

// JSON Serialization
namespace nlohmann
{
	template <>
	struct adl_serializer<string_user_id::StringUserID>
	{
		// note: the return type is no longer 'void', and the method only takes
		// one argument
		static string_user_id::StringUserID from_json(const json &J)
		{
			return string_user_id::StringUserID(J.at(string_user_id::JSONStringUserID).get<crypto_x509_certificate::string>());
		}

		// Here's the catch! You must provide a to_json method! Otherwise you
		// will not be able to convert move_only_type to json, since you fully
		// specialized adl_serializer on that type
		static void to_json(json &J, const string_user_id::StringUserID &T)
		{
			J = json{ string_user_id::JSONStringUserID, T.GetID() };
		}
	};
}

#endif // STRING_USER_ID_H
