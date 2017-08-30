#ifndef STRING_THREAD_H
#define STRING_THREAD_H

/*! \file string_managers.h
 *  \brief Header for StringGroupManager class
 */


#include "string_user.h"
#include "string_user_admin.h"
#include "string_thread_memory.h"

#include "string_thread_message.h"

#include "global_message_queue_thread.h"

#include <memory>

/*!
 *  \brief Namespace for StringGroupManager class
 */
namespace string_thread
{
	using std::shared_ptr;

	using string_user::UserCert;
	using string_user::UserCertID;

	using string_user::StringUser;
	using string_user_admin::StringUserAdmin;
	using string_user::StringUserID;

	using namespace string_thread_messages;

	using string_thread_memory::StringThreadMemory;

	using global_message_queue_thread::GlobalMessageQueueThread;

	/*!
	 * \brief Module for managing requests for StringUser credentials
	 */
	class StringThreadModuleRequests : public thread_multi_module_t
	{
		public:
			using StringThreadModuleRequestsSharedPtr = shared_ptr<StringThreadModuleRequests>;

			StringThreadModuleRequests(StringThreadMemory &Memory);

			void HandleMessage(msg_struct_t &Message);

			static void RegisterModule(GlobalMessageQueueThread &GlobalQueue, const StringThreadModuleRequestsSharedPtr &Module);

			void HandleModificationRequest(string_user_modification_t &ModificationRequest);

			void HandleUserStorageRequest(msg_struct_t &Request);

		private:

			StringThreadMemory &_Memory;

			//user_admin_request_t HandleRequest(const user_admin_request_t &Request);

			void HandleTLSCredentialsRequest(msg_struct_t &RequestMessage);

			/*!
			 * \brief Add all issuers to a given storage
			 */
			static void AddIssuerCertificateChain(StringUserStorage &NewStorage, const StringUserStorage &OldStorage, const StringUser *StringUser);
			static void AddIssuerCertificateChain(StringUserStorage &NewStorage, const StringUserStorage &OldStorage, const StringUserAdmin *StringAdmin);
	};

	using StringThreadModuleRequestsSharedPtr = StringThreadModuleRequests::StringThreadModuleRequestsSharedPtr;

	/*!
	 * \brief This thread manages all String credentials
	 */
	class StringThread : public thread_multi_module_manager_t
	{
		public:
			using StringThreadSharedPtr = shared_ptr<StringThread>;

			/*!
			 * 	\brief Constructor
			 */
			StringThread(GlobalMessageQueueThread &_GlobalQueue);

			/*!
			 * \brief Access to thread memory
			 */
			StringThreadMemory &GetMemory();

			/*!
			 * \brief Access to thread memory
			 */
			const StringThreadMemory &GetMemory() const;

			/*!
			 * \brief Registers this Manager with the global queue
			 */
			static void RegisterThread(GlobalMessageQueueThread &GlobalQueue, const StringThreadSharedPtr &Queue);

		private:

			/*!
			 * \brief Memory used by this thread
			 */
			StringThreadMemory _Memory;

			/*!
			 * \brief Module for managing requests
			 */
			StringThreadModuleRequests	_GeneralModule;

			/*!
			 * \brief Shared pointer to above module without destructor (used for registering with GlobalQueue)
			 */
			StringThreadModuleRequestsSharedPtr	_GeneralModulePtr;
	};

	using StringThreadSharedPtr = StringThread::StringThreadSharedPtr;
} // namespace string_managers

// JSON Serialization
namespace nlohmann
{
//	template <>
//	struct adl_serializer<string_managers::StringUserManager>
//	{
//		// note: the return type is no longer 'void', and the method only takes
//		// one argument
//		static string_managers::StringUserManager from_json(const json &J)
//		{
//			//return crypto_x509_certificate::X509CertificateID(J.at(crypto_x509_certificate::JSONX509CertificateID).get<crypto_x509_certificate::string>());
//		}

//		// Here's the catch! You must provide a to_json method! Otherwise you
//		// will not be able to convert move_only_type to json, since you fully
//		// specialized adl_serializer on that type
//		static void to_json(json &J, const string_managers::StringUserManager &T)
//		{
//			//J = json{ crypto_x509_certificate::JSONX509CertificateID, T.ConvertToString() };
//		}
//	};
}
#endif // STRING_THREAD_H
