#ifndef STRING_USER_ADMIN_H
#define STRING_USER_ADMIN_H

/*! \file string_user_admin.h
 *  \brief Header for StringUserAdmin class
 */


#include "string_user.h"
#include "secure_connection.h"
#include "crypto_x509_private_key.h"
#include <memory>

/*!
 *  \brief Namespace for StringUserAdmin class
 */
namespace string_user_admin
{
	using namespace string_user;

	using std::unique_ptr;

	using secure_connection::SecureConnection;
	using crypto_x509_private_key::X509PrivateKey;

	using user_cert_id::UserCertID;

	enum CertificateUsage
	{
		USAGE_ANY = 0
	};

	/*!
	 * \brief User certificate with corresponding private key
	 */
	class UserCertAdmin : public UserCert
	{
		public:
			UserCertAdmin(UserCert &&CertificateData, X509CertificateRequest &&Request, X509PrivateKey &&PrivateKey);

			/*!
			 * \brief Generates a New UserCertAdmin that only contains a subject field
			 * \param SubjectField String that will be in subject field
			 * \param ExpirationTime Time certificate expires (set to 0 for no expiry)
			 * \return Returns new UserCertAdmin
			 */
			static UserCertAdmin GenerateNewUserCertAdmin(const string &SubjectField, const time_t ExpirationTime);

			~UserCertAdmin() = default;

			const X509CertificateRequest &GetRequest() const;

			const X509PrivateKey &GetKey() const;

			void CombineWithUserCertAdmin(UserCertAdmin &&ExtraUserCertAdmin);

		private:
			/*!
			 * \brief Request used to create new certificates for this certificate
			 */
			X509CertificateRequest _Request;

			/*!
			 * \brief Private Key of Cert
			 */
			X509PrivateKey _CertificateKey;
	};

	using user_cert_admin_vector_t = vector_t::vector_t<UserCertAdmin>;

	/*!
	 * \brief The StringUserAdmin class
	 */
	class StringUserAdmin : public StringUser
	{
		public:
			static StringUserAdmin GenerateNewAdmin(const string &SubjectField, const time_t ExpirationTime);

			static StringUserAdmin CreateNewAdmin(UserCertAdmin &&NewAdmin);

			static StringUserAdmin ImportUser(StringUser &&User);

			//explicit StringUserAdmin(StringUser &&UserData);

			virtual ~StringUserAdmin() = default;

			/*!
			 * \brief Find a UserCert
			 * \param CertID ID of UserCert
			 * \return Returns pointer to UserCert or nullptr if not found
			 */
			UserCert *FindUserCert(const UserCertID &CertID);

			/*!
			 * \brief Find a UserCert
			 * \param CertID ID of UserCertAdmin
			 * \return Returns pointer to UserCert or nullptr if not found
			 */
			const UserCert *FindUserCert(const UserCertID &CertID) const;

			/*!
			 * \brief Find a UserCertAdmin
			 * \param CertID ID of UserCertAdmin
			 * \return Returns pointer to UserCertAdmin or nullptr if not found
			 */
			UserCertAdmin *FindUserCertAdmin(const UserCertID &CertID);

			/*!
			 * \brief Find a UserCertAdmin
			 * \param CertID ID of UserCertAdmin
			 * \return Returns pointer to UserCertAdmin or nullptr if not found
			 */
			const UserCertAdmin *FindUserCertAdmin(const UserCertID &CertID) const;

			void AddUserCert(UserCert &&NewCert) noexcept;

			/*!
			 * \brief Add new admin certificate
			 * \param NewAdmin
			 */
			void AddUserCertAdmin(UserCertAdmin &&NewAdmin);

			/*!
			 * \brief CombineWithStringUser
			 * \param ExtraStringUserAdmin
			 */
			void CombineWithStringUser(StringUser &&ExtraStringUser);

			/*!
			 * \brief CombineWithStringUserAdmin
			 * \param ExtraStringUserAdmin
			 */
			void CombineWithStringUserAdmin(StringUserAdmin &&ExtraStringUserAdmin);

			user_cert_admin_vector_t &GetAdminCertificates();
			const user_cert_admin_vector_t &GetAdminCertificates() const;

			UserCertIterator GetUserCertBegin() noexcept;
			UserCertIterator GetUserCertEnd() noexcept;

			ConstUserCertIterator GetUserCertBegin() const noexcept;
			ConstUserCertIterator GetUserCertEnd() const noexcept;

		private:
			/*!
			 * \brief User certificates with corresponding private keys
			 */
			user_cert_admin_vector_t _UserCertAdmins;

			/*!
			 * \brief How can this certificate be used
			 */
			CertificateUsage		_UsageOption;

			/*!
			 * \brief FindUserCertIterator
			 */
			user_cert_admin_vector_t::iterator FindUserCertAdminIterator(const UserCertID &ID);

			user_cert_admin_vector_t::const_iterator FindUserCertAdminIterator(const UserCertID &ID) const;

			/*!
			 * \brief Combine UserCert with UserCertAdmin and erase UserCert from _UserCerts
			 * \param AdminCert UserCertAdmin to which to add UserCertIterator
			 * \param UserCertIterator Iterator to UserCert that should be added to AdminCert and deleted from _UserCerts
			 */
			void CombineUserCertAndUserCertAdmin(UserCertAdmin &AdminCert, user_cert_vector_t::iterator UserCertIterator);

			void CombineUserCerts(user_cert_vector_t &&ExtraUserCerts);

			void CombineUserCertAdmins(user_cert_admin_vector_t &&ExtraUserCertAdmins);

			/*!
			 * \brief Constructor
			 */
			StringUserAdmin(UserCertAdmin &&AdminCert);

			/*!
			 * \brief Constructor
			 */
			StringUserAdmin(StringUser &&User);
	};
} // namespace string_user_admin


#endif // STRING_USER_ADMIN_H
