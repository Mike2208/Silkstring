#ifndef PROTOCOL_MODULE_TLS_CONNECTION_H
#define PROTOCOL_MODULE_TLS_CONNECTION_H

/*! \file protocol_module_tls_connection.h
 *  \brief Header for ProtocolModuleTLSConnection class
 */


#include "protocol_module_instantiator.h"
#include "protocol_messages.h"
#include "network_connection.h"
#include "tls_connection.h"
#include "string_thread_message.h"
#include "thread_function.h"
#include "crypto_tls_certificate_credentials.h"

#include "testing_class_declaration.h"

/*!
 *  \brief Namespace for ProtocolModuleTLSConnection class
 */
namespace protocol_module_tls_connection
{
	using std::shared_ptr;
	using std::atomic;
	using std::mutex;
	using std::function;

	using crypto_tls_certificate_credentials::TLSCertificateCredentials;

	using network_connection::NetworkConnection;

	using namespace protocol_messages;

	using protocol_module_instantiator::ProtocolModuleInstantiator;
	using protocol_thread_memory::ProtocolThreadMemory;
	using protocol_send_handle::ProtocolModuleSendHandle;

	//using string_thread_messages::request_user_shared_ptr_t;
	//using string_thread_messages::request_user_admin_shared_ptr_t;

	using global_message_queue_thread::GlobalMessageQueueThread;

	using tls_connection::TLSConnection;

	using thread_function::ThreadFunction;

	/*!
	 * \brief Read for 1ms
	 */
	static constexpr network_connection::timeout_ms_t TLSRequestCertTime(1);
	static constexpr network_connection::timeout_ms_t TLSWriteTransferTime(1);
	static constexpr network_connection::timeout_ms_t TLSReadTransferTime(TLSRequestCertTime.get()+TLSWriteTransferTime.get()+1);

	/*!
	 * \brief Read buffer size in bytes
	 */
	static constexpr size_t TLSReadBufferSize(1000);

	/*!
	 * \brief Number of attempts to transfer the certificate IDs
	 */
	static constexpr char TLSIDTransferTries = 5;

	class ProtocolModuleTLSNetworkConnectionBase;
	class ProtocolModuleTLSNetworkConnection;

	/*!
	 * \brief State of TLS handshake
	 */
	enum tls_handshake_state_t
	{
		/*!
		 *	\brief No handshake performed yet
		 */
		TLS_HANDSHAKE_UNDEFINED = -1,

		/*!
		 *	\brief Handshake performed, but failed
		 */
		TLS_HANDSHAKE_FAILED = 0,

		/*!
		 *	\brief Handshake performed, but succeeded
		 */
		TLS_HANDSHAKE_SUCCEEDED,

		/*!
		 *	\brief Request a handshake be performed
		 */
		TLS_HANDSHAKE_PERFORM,

		/*!
		 *	\brief Handshake running
		 */
		TLS_HANDSHAKE_RUNNING
	};

	struct transferred_shared_data_t
	{
		using atomic_id_t = atomic<StringUserID>;

		atomic_id_t &OwnID;

		GlobalMessageQueueThread &MessageQueue;
		identifier_t::thread_id_t ThreadID;
		connection_side_t ConnectionSide;
	};

	/*!
	 * \brief Data shared between ProtocolModuleTLSConnection and ProtocolModuleTLSNetworkConnection
	 */
	struct shared_data_t : public transferred_shared_data_t
	{
		NetworkConnection &Connection;		
	};

	/*!
	 * \brief Transforms the given data into
	 */
	class ProtocolModuleTLSConnection : public thread_multi_module_t, protected ProtocolModuleSendHandle
	{
			using thread_fcn_t  = TLSCertificateCredentials(ProtocolModuleTLSConnection *const Module, connection_side_t ConnectionSide, TLSCertificateCredentials &&Credentials);
			using thread_t		= ThreadFunction<thread_fcn_t, TLSCertificateCredentials, ProtocolModuleTLSConnection *const, connection_side_t , TLSCertificateCredentials&&>;

		public:
			/*!
			 * 	\brief Constructor
			 */
			ProtocolModuleTLSConnection(ProtocolThreadMemory &Memory);

			void HandleMessage(msg_struct_t &Message);

		private:
			ProtocolThreadMemory		&_Memory;

			/*!
			 * \brief Credentials used for TLS certificate
			 */
			TLSCertificateCredentials	_Credentials;

			/*!
			 * \brief Has the trust chain been added to the credentials?
			 */
			bool						_TrustChainReceived = false;

			/*!
			 * \brief Has the key chain been added to the credentials?
			 */
			bool						_KeyChainReceived = false;

			size_t						_HandshakeAttempts = 0;

			/*!
			 * \brief Stop the message thread
			 */
			atomic<bool>				_StopThread;

			/*!
			 * \brief Result of last handshake/Order to perform new handshake
			 */
			atomic<tls_handshake_state_t>	_HandshakeState;

			/*!
			 * \brief Own ID for connection
			 */
			atomic<StringUserID>			_OwnID;

			/*!
			 * \brief Lock for buffers
			 */
			mutex				_BuffersLock;

			/*!
			 * \brief Buffer for any read data that should be processed by ProtocolModuleTLSNetworkConnectionBase thread
			 */
			protocol_vector_t	_ReadBuffer;

			/*!
			 * \brief Buffer for write data that the ProtocolModuleTLSNetworkConnectionBase thread wishes to send
			 */
			protocol_vector_t	_WriteBuffer;

			/*!
			 * \brief Buffer for any data that this protocol wishes to send over an encrypted channel via the ProtocolModuleTLSNetworkConnectionBase thread
			 */
			protocol_vector_t	_InputBuffer;

			/*!
			 * \brief Buffer for any data that this protocol received from the peer over an encrypted channel via the ProtocolModuleTLSNetworkConnectionBase thread
			 */
			protocol_vector_t	_OutputBuffer;

			/*!
			 * \brief Separate thread for de/encryption of TLS data
			 */
			thread_t			_TLSThread;

			/*!
			 * \brief HandleStateChangeRequest
			 */
			void HandleStateUpdate(protocol_state_t UpdatedState);

			/*!
			 * \brief Copies Data To end of Buffer
			 * \param Data Data to copy
			 * \param Buffer Buffer where data should be inserted
			 * \param BufferLock Mutex to ensure only this thread accesses buffer
			 */
			static void CopyDataToBufferEnd(const protocol_vector_t &Data, protocol_vector_t &Buffer, mutex &BufferLock);

			/*!
			 * \brief Move Data From Buffer
			 * \param Buffer Buffer to move data from
			 * \param BufferLock Locks buffer
			 * \return Returns data in buffer
			 */
			static protocol_vector_t MoveDataFromBuffer(protocol_vector_t &Buffer, mutex &BufferLock);

			// Give base connection access to buffers
			friend class ProtocolModuleTLSNetworkConnectionBase;
			friend class ProtocolModuleTLSNetworkConnection;

			template<class U>
			friend class ::TestingClass;
	};

	using ProtocolModuleTLSConnectionSharedPtr = shared_ptr<ProtocolModuleTLSConnection>;

	class ProtocolModuleTLSConnectionInstantiator : public ProtocolModuleInstantiator
	{
		public:
			ProtocolModuleTLSConnectionInstantiator();

		private:
			thread_multi_module_shared_ptr_t CreateNewInstanceHandle(instantiation_data_t &InstanceData) const;
	};

	/*!
	 * \brief Base connection for protocol. Over this, the TLS is implemented
	 */
	class ProtocolModuleTLSNetworkConnectionBase : public NetworkConnection
	{
		public:
			ProtocolModuleTLSNetworkConnectionBase(ProtocolModuleTLSConnection &Module, GlobalMessageQueueThread &GlobalQueue);

			void Close();

			bool IsReadDataAvailable();

			size_t Read(byte_t *NetOutput, size_t OutputSize);

			size_t Write(const byte_t *NetInput, size_t InputSize);

		private:
			/*!
			 * \brief Access to read/write buffers
			 */
			ProtocolModuleTLSConnection *_Module;

			/*!
			 * \brief Push message when write buffer is updated
			 */
			GlobalMessageQueueThread	&_GlobalQueue;

			NetworkConnectionUniquePtr	Move();

			template<class U>
			friend class ::TestingClass;
	};

	/*!
	 * \brief ProtocolModuleTLSNetworkConnection class
	 */
	class ProtocolModuleTLSNetworkConnection : public TLSConnection
	{
		public:
			ProtocolModuleTLSNetworkConnection(NetworkConnectionUniquePtr &&BaseConnection, connection_side_t ConnectionSide, TLSCertificateCredentials &&Credentials, transferred_shared_data_t &&AtomicID);

			ProtocolModuleTLSNetworkConnection(ProtocolModuleTLSNetworkConnection &&S);
			ProtocolModuleTLSNetworkConnection &operator=(ProtocolModuleTLSNetworkConnection &&S);

			static TLSCertificateCredentials ThreadFcn(ProtocolModuleTLSConnection *const Module, connection_side_t ConnectionSide, TLSCertificateCredentials &&Credentials);

		private:

			/*!
			 * \brief Data for TLS Hook
			 */
			shared_data_t _ModuleData;

			/*!
			 * \brief Function for hooking into TLS Handshake
			 */
			hook_fcn_t _HookFunction;

			static bool SendStringUserID(const StringUserID &OwnID, shared_data_t &SharedData);
			static StringUserID ReceiveStringUserID(shared_data_t &SharedData);

			static int SendCertificateInfo(shared_data_t &SharedData);
			static int ReceiveCertificateInfo(shared_data_t &SharedData);

			static int ExchangeCertificateInfo(shared_data_t SharedData, gnutls_session_t Session, unsigned int htype, unsigned post, unsigned int incoming, const gnutls_datum_t *msg);

			/*!
			 * \brief BindHook
			 * \return
			 */
			hook_fcn_t BindHook();

			template<class U>
			friend class ::TestingClass;
	};
} // namespace protocol_module_tls_connection


#endif // PROTOCOL_MODULE_TLS_CONNECTION_H
