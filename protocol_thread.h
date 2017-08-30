#ifndef PROTOCOL_THREAD_H
#define PROTOCOL_THREAD_H

/*! \file protocol_thread.h
 *  \brief Header for ProtocolThread class
 */


#include "protocol_network_connection.h"
#include "protocol_send_handle.h"
#include "protocol_messages.h"
#include "protocol_vector.h"
#include "protocol_thread_memory.h"
#include "global_message_queue_thread.h"
#include "protocol_module_instantiator.h"

#include "testing_class_declaration.h"

/*!
 *  \brief Namespace for ProtocolThread class
 */
namespace protocol_thread
{
	using std::shared_ptr;

	using vector_t::vector_type;

	using protocol_network_connection::ProtocolNetworkConnection;

	using protocol_send_handle::ProtocolSendHandle;
	using protocol_send_handle::ProtocolModuleSendHandle;

	using protocol_data::protocol_thread_id_t;
	using protocol_data::protocol_header_t;

	using protocol_messages::identifier_t;
	using protocol_messages::message_t;
	using protocol_messages::message_ptr;
	using protocol_messages::StartModuleID;
	using protocol_messages::UnusedID;
	using protocol_messages::thread_multi_module_manager_t;
	using protocol_messages::thread_multi_module_t;
	using protocol_messages::thread_multi_module_message_t;
	using protocol_messages::ProtocolQueueID;
	using protocol_messages::ProtocolConnectionModuleID;
	using protocol_messages::ProtocolConnectionModuleRequestReceiveMessageType;
	using protocol_messages::request_receive_t;

	using protocol_thread_memory::ProtocolThreadMemory;

	using protocol_vector::protocol_vector_t;

	using protocol_messages::ProtocolConnectionModuleReceivedDataMessageType;
	using protocol_messages::received_data_t;

	using global_message_queue_thread::GlobalMessageQueueThread;

	using protocol_module_instantiator::ProtocolModuleInstantiator;
	using protocol_module_instantiator::ProtocolModuleInstantiatorUniquePtr;

	using instantiator_vector_t = vector_type<ProtocolModuleInstantiatorUniquePtr>;

	using network_connection::connection_side_t;

	/*!
	 * \brief The ProtocolThread class
	 */
	class ProtocolThread : public thread_multi_module_manager_t
	{
		public:
			/*!
			 * 	\brief Constructor
			 */
			ProtocolThread(protocol_thread_id_t ThreadID, ProtocolNetworkConnection &Connection, GlobalMessageQueueThread &GlobalQueue, connection_side_t ConnectionSide);

			void ExecuteModuleInstantiators(const instantiator_vector_t &ModuleInstantiators, ProtocolNetworkConnection &Connection);

			/*!
			 * \brief Access memory that all modules of this thread share
			 */
			ProtocolThreadMemory &GetMemory();

			/*!
			 * \brief Access memory that all modules of this thread share
			 */
			const ProtocolThreadMemory &GetMemory() const;

		private:
			ProtocolThreadMemory _ThreadMemory;

			template<class U>
			friend class ::TestingClass;
	};

	using ProtocolThreadSharedPtr = shared_ptr<ProtocolThread>;
} // namespace protocol_thread


#endif // PROTOCOL_THREAD_H
