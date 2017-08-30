#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

/*! \file protocol_manager.h
 *  \brief Header for ProtocolManager class
 */

#include "global_message_queue_thread.h"

#include "protocol_data.h"
#include "protocol_map.h"
#include "silkstring_message.h"
#include "protocol_network_connection.h"
#include "vector_t.h"

#include "protocol_thread.h"

#include "protocol_module_instantiator.h"


/*!
 *  \brief Namespace for ProtocolManager class
 */
namespace protocol_manager
{
	using std::list;
	using std::shared_ptr;

	using network_connection::NetworkConnectionUniquePtr;

	using protocol_data::protocol_thread_id_t;
	using protocol_network_connection::ProtocolNetworkConnection;

	using protocol_map::ProtocolMap;

	using protocol_thread::ProtocolThread;
	using protocol_thread::ProtocolThreadSharedPtr;

	using protocol_messages::identifier_t;

	using protocol_module_instantiator::ProtocolModuleInstantiator;
	using protocol_module_instantiator::ProtocolModuleInstantiatorUniquePtr;

	using global_message_queue_thread::GlobalMessageQueueThread;

	using network_connection::connection_side_t;

	using vector_t::vector_type;

	class TestProtocolManager;

	/*!
	 * \brief The ProtocolManager class
	 */
	class ProtocolManager
	{
			using connection_list_t = list<ProtocolNetworkConnection>;

			using thread_instance_t = ProtocolThread;
			using thread_instance_shared_ptr_t = ProtocolThreadSharedPtr;

			using thread_map_t = ProtocolMap<protocol_thread_id_t, thread_instance_shared_ptr_t>;

			using instantiator_vector_t = protocol_thread::instantiator_vector_t;

		public:
			using ProtocolManagerSharedPtr = shared_ptr<ProtocolManager>;

			/*!
			 * 	\brief Constructor
			 */
			ProtocolManager(GlobalMessageQueueThread &GlobalMessageQueue);

			/*!
			 * \brief Creates a New Thread for the given connection
			 * \param Connection Connection that this thread manages
			 */
			protocol_thread_id_t CreateNewInstance(NetworkConnectionUniquePtr &&Connection, connection_side_t ConnectionSide);

			/*!
			 * \brief Stops an Instance and returns pointer to network connection
			 */
			NetworkConnectionUniquePtr StopInstance(protocol_thread_id_t ConnectionID);

			/*!
			 * \brief RegisterModule Registers a new module
			 * \param ModuleInstantiator Instantiator to use to create new modules
			 */
			void RegisterModuleInstantiator(ProtocolModuleInstantiatorUniquePtr &&ModuleInstantiator);

			/*!
			 * \brief UnregisterModule Unregisters the module with ModuleID
			 * \param ModuleID
			 * \return
			 */
			ProtocolModuleInstantiatorUniquePtr UnregisterModule(identifier_t::module_id_t ModuleID);

			/*!
			 * \brief RequestRead from one connection
			 */
			void RequestRead(protocol_thread_id_t ProtocolThreadID);

			/*!
			 * \brief Access all registered instantiation modules
			 */
			const instantiator_vector_t &GetAllInstantionModules() const noexcept;

			// Make sure class isn't moved to keep references valid
			ProtocolManager(const ProtocolManager &S) = delete;
			ProtocolManager(ProtocolManager &&S) = default;

			ProtocolManager &operator=(const ProtocolManager &S) = delete;
			ProtocolManager &operator=(ProtocolManager &&S) = delete;

		private:

			/*!
			 * \brief Reference to global message queue
			 */
			GlobalMessageQueueThread &_GlobalMessageQueue;

			/*!
			 * \brief All Threads mapped to their respective IDs
			 */
			thread_map_t _Threads;

			protocol_thread_id_t _NextFreeID = 1;

			instantiator_vector_t _ModuleInstantiators;

			/*!
			 * \brief Store connections
			 */
			connection_list_t _Connections;
	};

	using ProtocolManagerSharedPtr = ProtocolManager::ProtocolManagerSharedPtr;
} // namespace protocol_manager


#endif // PROTOCOL_MANAGER_H
