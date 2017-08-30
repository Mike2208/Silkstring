#ifndef STRING_USER_STORAGE_H
#define STRING_USER_STORAGE_H

/*! \file string_user_storage.h
 *  \brief Header for StringUserStorage class
 */


#include "string_user.h"
#include "string_user_admin.h"

#include "crypto_tls_certificate_credentials.h"

#include "vector_t.h"

/*!
 *  \brief Namespace for StringUserStorage class
 */
namespace string_user_storage
{
	using vector_t::vector_t;

	using crypto_tls_certificate_credentials::TLSCertificateCredentials;

	using crypto_x509_certificate::X509Certificate;
	using crypto_x509_certificate_chain::X509CertificateChain;
	using crypto_x509_private_key::X509PrivateKey;

	using string_user_admin::UserCertAdmin;

	using string_user::UserCertID;
	using string_user::UserCert;

	using string_user::StringUserID;
	using string_user::StringUser;
	using string_user_admin::StringUserAdmin;

	struct user_admin_cert_chain_t
	{
		/*!
		 * \brief Chain signing Admin certificate
		 */
		X509CertificateChain	CertChain;

		/*!
		 * \brief Admin certificate
		 */
		X509PrivateKey	Key;
	};

	/*!
	 * \brief Class that manages all StringUser and StringUserAdmin structs
	 */
	class StringUserStorage
	{			
			using string_user_vector_t = vector_t<StringUser>;
			using string_user_admin_vector_t = vector_t<StringUserAdmin>;

		public:
			/*!
			 * 	\brief Constructor
			 */
			StringUserStorage() = default;

			/*!
			 * \brief Returns pointer to requested user if it is found in _StringUsers (or nullptr if not available)
			 * \param UserID StringUserID of StringUser to find
			 */
			StringUser *FindUserInUserStorage(const StringUserID &UserID);

			/*!
			 * \brief Returns pointer to requested user _StringUsers (or nullptr if not available)
			 * \param UserID StringUserID of StringUser to find
			 */
			const StringUser *FindUserInUserStorage(const StringUserID &UserID) const;

			/*!
			 * \brief Returns pointer to requested user if it is found in _StringUsers or in _StringUserAdmins (or nullptr if not available)
			 * \param UserID StringUserID of StringUser to find
			 */
			StringUser *FindUserEverywhere(const StringUserID &UserID);

			/*!
			 * \brief Returns pointer to requested user _StringUsers or in _StringUserAdmins (or nullptr if not available)
			 * \param UserID StringUserID of StringUser to find
			 */
			const StringUser *FindUserEverywhere(const StringUserID &UserID) const;

			/*!
			 * \brief Find Owner of UserCert
			 */
			const StringUser *FindUserCertOwner(const UserCertID &UserID) const;

			/*!
			 * \brief Find Owner of UserCert
			 */
			const StringUserAdmin *FindAdminUserCertOwner(const UserCertID &UserID)const;

			/*!
			 * \brief Finds a user cert
			 * \param UserID
			 */
			const UserCert *FindUserCert(const UserCertID &UserID) const;

			/*!
			 * \brief Finds a user cert
			 * \param UserID
			 * \param StrUserID ID of StringUser that possesses this UserCert
			 */
			const UserCert *FindUserCert(const UserCertID &UserID, const StringUserID &StrUserID) const;

			/*!
			 * \brief Finds a user cert admin
			 * \param UserID
			 */
			const UserCertAdmin *FindUserCertAdmin(const UserCertID &UserID) const;

			/*!
			 * \brief Finds a user cert admin
			 * \param UserID
			 * \param StrUserID ID of StringUserAdmin that possesses this UserCertAdmin
			 */
			const UserCertAdmin *FindUserCertAdmin(const UserCertID &UserID, const StringUserID &StrUserID) const;

			/*!
			 * \brief Returns pointer to requested user (or nullptr if not available)
			 * \param UserID StringUserID of StringUserAdmin to find
			 */
			StringUserAdmin *FindUserAdmin(const StringUserID &UserID);

			/*!
			 * \brief Returns pointer to requested user (or nullptr if not available)
			 * \param UserID StringUserID of StringUserAdmin to find
			 */
			const StringUserAdmin *FindUserAdmin(const StringUserID &UserID) const;

			/*!
			 * \brief Registers a New User
			 * \param NewUserID ID to register
			 * \return Returns a pointer to the created or existing StringUser
			 */
			StringUser *RegisterNewUser(const StringUserID &NewUserID);

			/*!
			 * \brief Registers a New User Admin
			 * \param NewUserID ID to register
			 * \return Returns a pointer to the created or existing StringUserAdmin
			 */
			StringUserAdmin *RegisterNewUserAdmin(UserCertAdmin &&AdminCertificate);

			/*!
			 * \brief Adds New StringUserAdmin
			 * \param NewUserAdmin
			 * \return Returns pointer to created or existing StringUserAdmin
			 */
			StringUserAdmin *AddNewUserAdmin(StringUserAdmin &&NewUserAdmin);

			/*!
			 * \brief Adds New StringUser
			 * \param NewUserAdmin
			 * \return Returns pointer to created or existing StringUser
			 */
			StringUser *AddNewUser(StringUser &&NewUser);

			/*!
			 * \brief Get Chain to validate the certificate with the given ID
			 * \param UserID ID of cert to verify
			 * \param StrID ID of StringUser if known
			 */
			X509CertificateChain GetValidationChain(const UserCertID &UserID, const StringUserID *StrID = nullptr) const;

			/*!
			 * \brief Get Admin certificate with AdminUserID and validation chain
			 * \param AdminUserID ID of Admin certificate
			 * \param StrAdminID ID of StringUserAdmin if known
			 * \return Returns requested certificate and validation chain
			 */
			user_admin_cert_chain_t GetAdminCertAndValidationChain(const UserCertID &AdminUserID, const StringUserID *StrAdminID = nullptr) const;

			/*!
			 * \brief Generates TLS credentials using all available StringUsers and StringUserAdmins
			 * \return
			 */
			TLSCertificateCredentials GenerateCompleteTLSCredentials() const;

			/*!
			 * \brief Combine With new Storage
			 * \param ExtraStorage Storage to add to this one
			 */
			void CombineWithStorage(StringUserStorage &&ExtraStorage);

			/*!
			 * \brief Access StringUserAdmin storage
			 * \return Returns reference to StringUserAdmin storage
			 */
			string_user_admin_vector_t &GetUserAdmins();

			/*!
			 * \brief Access StringUserAdmin storage
			 * \return Returns reference to StringUserAdmin storage
			 */
			const string_user_admin_vector_t &GetUserAdmins() const;

			/*!
			 * \brief Empty all stored data
			 */
			void Reset();

		private:

			/*!
			 * \brief Contains all StringUsers
			 */
			string_user_vector_t _StringUsers;

			/*!
			 * \brief Contains all StringUserAdmins
			 */
			string_user_admin_vector_t _StringUserAdmins;



			string_user_vector_t::iterator FindUserIteratorInUserStorage(const StringUserID &UserID);

			string_user_vector_t::const_iterator FindUserIteratorInUserStorage(const StringUserID &UserID) const;

			string_user_admin_vector_t::iterator FindUserAdminIterator(const StringUserID &UserID);

			string_user_admin_vector_t::const_iterator FindUserAdminIterator(const StringUserID &UserID) const;

			string_user_admin_vector_t::iterator UpgradeToStringUserAdmin(string_user_vector_t::iterator StringUserIterator, StringUserAdmin &&NewAdminData);

			/*!
			 * \brief Function for finding a specific user
			 * \param UserID
			 * \return
			 */
			//static string_user_vector_t::iterator FindUserFcn(const StringUserID &UserID);
	};
} // namespace string_user_storage


#endif // STRING_USER_STORAGE_H
