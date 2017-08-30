#ifndef STRING_USER_H
#define STRING_USER_H

/*! \file string_user.h
 *  \brief Header for StringUser class
 */


#include "user_cert_id.h"
#include "string_connection_option.h"
#include "string_user_id.h"
#include "crypto_x509_certificate.h"
#include "crypto_x509_certificate_chain.h"
#include "string_user_admin_declaration.h"
#include "string_user_admin_declaration.h"

//#include <nlohmann/json.hpp>
#include <memory>

/*!
 *  \brief Namespace for StringUser class
 */
namespace string_user
{
	using crypto_x509_certificate::X509Certificate;
	using crypto_x509_certificate::X509CertificateRequest;
	using crypto_x509_certificate::X509CertificateID;

	using crypto_x509_certificate_chain::X509CertificateChain;

	using std::string;
	using std::unique_ptr;

	//using vector_t::vector_t;

	using string_user_id::StringUserID;

	using user_cert_id::UserCertID;
	using user_cert_id::UserCertIDVector;
	using string_connection_option::StringConnectionOptionVector;

//	using nlohmann::json;
	using string_user_admin::StringUserAdmin;
	using string_user_admin::UserCertAdmin;

	//struct UserCertChain;

	/*!
	 * \brief Certificate of a user
	 */
	class UserCert : public X509Certificate
	{
		public:
			/*!
			 *	\brief Constructor
			 */
			UserCert() = default;

			UserCert(X509Certificate Cert, StringUserID MainStringUserID, const UserCertID Issuer) noexcept;

			UserCert(const UserCert &S) = default;
			UserCert &operator=(const UserCert &S) = default;

			UserCert(UserCert &&S) noexcept = default;
			UserCert &operator=(UserCert &&S) noexcept = default;

			/*!
			 * \brief Get UserCertID
			 */
			operator UserCertID() const;

			/*!
			 *	\brief Returns this UserCert's ID
			 */
			UserCertID GetCertificateID() const;

			/*!
			 * \brief Get the IssuerID of this certificate
			 */
			const UserCertID &GetIssuer() const;

			/*!
			 * \brief Checks if this UserCert has an issuer
			 */
			bool HasIssuer() const;

			/*!
			 * \brief Combine With similar Cert
			 * \param ExtraCert
			 */
			void CombineWithCert(UserCert &&ExtraCert);

			/*!
			 * \brief Get the StringUser this UserCert is assigned to
			 */
			const StringUserID &GetMainStringUserID() const noexcept;

			/*!
			 * \brief Get all Certificates signed by this UserCert
			 */
			const UserCertIDVector &GetSignedCertificateIDs() const noexcept;

		private:

			/*!
			 * \brief Issuer of this certificate
			 */
			UserCertID _IssuerID;

			/*!
			 * \brief ID of main user
			 */
			StringUserID _MainStringUserID;

			/*!
			 * \brief Vector containing IDs of certificates signed by this UserCert
			 */
			UserCertIDVector _SignedCertificateIDs;
	};

//	struct UserCertChain : public X509CertificateChain
//	{
//		UserCertChain() = default;

//		/*!
//		 * \brief Constructor
//		 */
//		UserCertChain(X509CertificateChain &&Chain);

//		/*!
//		 *	\brief Constructor
//		 */
//		template<class ...T>
//		UserCertChain(T ...UserCerts) : X509CertificateChain(std::forward(UserCerts)...)
//		{}
//	};

	/*!
	 *	\brief Vector with user certificates
	 */
	using user_cert_vector_t = vector_t::vector_t<UserCert>;
	using user_cert_admin_vector_t = vector_t::vector_t<UserCertAdmin>;


	//const string JSONStringUser = "StringUser";

	class StringUser;


	class ConstUserCertIterator
	{
		public:
			explicit ConstUserCertIterator(const user_cert_vector_t *UserCerts, const user_cert_admin_vector_t *UserCertAdmins, size_t CurPosInVectors);

			ConstUserCertIterator &operator++();
			ConstUserCertIterator operator++(int);

			ConstUserCertIterator &operator--();
			ConstUserCertIterator operator--(int);

			bool operator==(const ConstUserCertIterator &S) const;
			bool operator!=(const ConstUserCertIterator &S) const;

			const UserCert *base() const;

			const UserCert *operator*(int) const;

			const UserCert *operator->() const;

		private:

			size_t							_CurPosInVectors;

			const user_cert_vector_t		*const _UserCerts;
			const user_cert_admin_vector_t	*const _UserCertAdmins;
	};

	class UserCertIterator
	{
		public:
			explicit UserCertIterator(user_cert_vector_t *UserCerts, user_cert_admin_vector_t *UserCertAdmins, size_t CurPosInVectors);

			UserCertIterator &operator++();
			UserCertIterator operator++(int);

			UserCertIterator &operator--();
			UserCertIterator operator--(int);

			bool operator==(const UserCertIterator &S) const;
			bool operator!=(const UserCertIterator &S) const;

			UserCert *base();
			const UserCert *base() const;

			UserCert *operator*(int);
			const UserCert *operator*(int) const;

			const UserCert *operator->() const;
			UserCert *operator->();

			operator ConstUserCertIterator() const noexcept;

		private:

			size_t						_CurPosInVectors;

			user_cert_vector_t			*const _UserCerts;
			user_cert_admin_vector_t	*const _UserCertAdmins;
	};

	/*!
	 * \brief The StringUser class
	 */
	class StringUser : public StringUserID
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			explicit StringUser(StringUserID &&MainID);

			explicit StringUser(const StringUserID &MainID);

			virtual ~StringUser() = default;

			/*!
			 * \brief Finds the user certificate corresponding to ID
			 * \return Returns the corresponding certificate
			 */
			virtual UserCert *FindUserCert(const UserCertID &ID);

			/*!
			 * \brief Finds the user certificate corresponding to ID
			 * \return Returns the corresponding certificate
			 */
			virtual const UserCert *FindUserCert(const UserCertID &ID) const;

			/*!
			 * \brief Get ID OF StringUser
			 */
			const StringUserID &GetMainID() const noexcept;

			/*!
			 * \brief Add a new certificate to this StringUser
			 * \param NewCert Certificate to add
			 */
			virtual void AddUserCert(UserCert &&NewCert) noexcept;

			/*!
			 * \brief Removes a User Cert
			 */
			void RemoveUserCert(const UserCertID &IDToRemove) noexcept;

			/*!
			 * \brief CombineWithStringUser
			 * \param ExtraStringUser
			 */
			virtual void CombineWithStringUser(StringUser &&ExtraStringUser);

			/*!
			 * \brief Get possible options to connect to user
			 */
			StringConnectionOptionVector &GetConnections() noexcept;

			/*!
			 * \brief Get possible options to connect to user
			 */
			const StringConnectionOptionVector &GetConnections() const noexcept;

			/*!
			 * \brief Get iterator for UserCerts
			 * \return Returns beginning of certs
			 */
			virtual ConstUserCertIterator GetUserCertBegin() const noexcept;

			/*!
			 * \brief Get iterator for UserCerts
			 * \return Returns end of certs
			 */
			virtual ConstUserCertIterator GetUserCertEnd() const noexcept;

			/*!
			 * \brief Get iterator for UserCerts
			 * \return Returns beginning of certs
			 */
			virtual UserCertIterator GetUserCertBegin() noexcept;

			/*!
			 * \brief Get iterator for UserCerts
			 * \return Returns end of certs
			 */
			virtual UserCertIterator GetUserCertEnd() noexcept;

		protected:
			/*!
			 * \brief Certificates with which this user can identify
			 */
			user_cert_vector_t _UserCerts;

			/*!
			 * \brief Storage for all possibilities for connecting to User
			 */
			StringConnectionOptionVector _ConnectionOptions;

			/*!
			 * \brief FindUserCertIterator
			 */
			user_cert_vector_t::iterator FindUserCertIterator(const UserCertID &ID);

			user_cert_vector_t::const_iterator FindUserCertIterator(const UserCertID &ID) const;

			/*!
			 *	\brief Combine UserCerts
			 */
			void CombineUserCerts(user_cert_vector_t &&ExtraCerts);

			friend class string_user_admin::StringUserAdmin;
	};
} // namespace string_user

// JSON Serialization
//namespace nlohmann
//{
//	template <>
//	struct adl_serializer<string_user::StringUser>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static string_user::StringUser from_json(const json &J)
//		{
//			//return string_user::StringUser();
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const string_user::StringUser &T)
//		{
//			json tmpStringID = T.GetMainID();
//			J = json{ string_user::JSONStringUser, {tmpStringID, }};
//		}
//	};
//}
#endif // STRING_USER_H
