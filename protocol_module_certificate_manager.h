#ifndef PROTOCOL_MODULE_CERTIFICATE_MANAGER_H
#define PROTOCOL_MODULE_CERTIFICATE_MANAGER_H

/*! \file protocol_module_certificate_manager.h
 *  \brief Header for ProtocolModuleCertificateManager class
 */


#include "protocol_messages.h"
#include "protocol_thread_memory.h"
#include "protocol_module_instantiator.h"
#include "string_thread_message.h"
#include "user_io_messages.h"
#include "string_user_storage.h"

#include "debug_flag.h"

/*!
 *  \brief Namespace for ProtocolModuleCertificateManager class
 */
namespace protocol_module_certificate_manager
{
	using namespace protocol_messages;

	using protocol_thread_memory::ProtocolThreadMemory;
	using global_message_queue_thread::GlobalMessageQueueThread;

	using protocol_module_instantiator::ProtocolModuleInstantiator;

	using string_user_storage::StringUserStorage;

	//using string_thread_messages::user_request_with_id_t;
	using string_thread_messages::user_storage_answer_t;

	using user_io_messages::registration_answer_message_t;

	class TestProtocolModuleCertificateManager;

	/*!
	 * \brief Mananges all StringUser and StringUserAdmin interactions
	 */
	class ProtocolModuleCertificateManager : public thread_multi_module_t
	{
		public:
			static constexpr identifier_t::module_id_t ModuleID = ProtocolCertificateManagerModuleID;

			/*!
			 * 	\brief Constructor
			 */
			ProtocolModuleCertificateManager(ProtocolThreadMemory &ThreadMemory);

			void HandleMessage(msg_struct_t &Message);

		private:

			ProtocolThreadMemory &_Memory;

			/*!
			 * \brief ID of peer when waiting for registration/verififcation
			 */
			StringUserID	_PendingRegistrationID;

			/*!
			 * \brief Storage for certificates used in this connection
			 */
			StringUserStorage _CertStorage;

			/*!
			 * \brief Handle state changes
			 */
			void HandleInternalStateChangeMessage(protocol_state_t UpdatedState);

			/*!
			 * \brief Handle receeived peer messages. Verifies the received IDs
			 * \param PeerData
			 */
			void HandlePeerMessage(received_data_t &PeerData);

			/*!
			 * \brief Handles answers from the thread that verifies received IDs
			 * \param VerificationAnswer
			 */
			void HandleStorageAnswer(user_storage_answer_t &StorageAnswer);

			/*!
			 * \brief Handles when
			 * \param RegistrationAnswer
			 */
			void HandleUserRegistrationAnswer(registration_answer_message_t &RegistrationAnswer);

			/*!
			 * \brief Send own authentication certificate ID to peer
			 * \param OwnCertificateID
			 */
			void HandleAdminCertificateSend(const StringUserID &OwnCertificateID);

			/*!
			 * \brief Request that the  protocol state be changed
			 */
			void RequestStateChange(protocol_state_t NewState) const;

			/*!
			 * \brief Requests an authentication ID to send to peer
			 */
			void RequestID(const StringUserID &PeerID);

			/*!
			 * \brief Request a verification of the received ID
			 */
			void RequestIDVerification(const StringUserID &ReceivedID);

			/*!
			 * \brief Request Registration of new ID
			 */
			void RequestIDRegistration(const StringUserID &IDToRegister);

			/*!
			 * \brief SendTLSCredentials
			 */
			void SendTLSCredentialsToModule(const StringUserStorage &Storage);

			/*!
			 * \brief Protocol reaction to a certificate failure
			 */
			void AbortSequence();

			template<class U>
			friend class ::TestingClass;
			friend class TestProtocolModuleCertificateManager;
	};

	class ProtocolModuleCertificateManagerInstantiator : public ProtocolModuleInstantiator
	{
		public:
			ProtocolModuleCertificateManagerInstantiator();

		private:

			thread_multi_module_shared_ptr_t CreateNewInstanceHandle(instantiation_data_t &Instance) const;
	};
} // namespace protocol_module_certificate_manager


#endif // PROTOCOL_MODULE_CERTIFICATE_MANAGER_H
