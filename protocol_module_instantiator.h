#ifndef PROTOCOL_MODULE_INSTANTIATOR_H
#define PROTOCOL_MODULE_INSTANTIATOR_H

/*! \file protocol_module_instantiator.h
 *  \brief Header for ProtocolModuleInstantiator class
 */


#include "protocol_messages.h"
#include "protocol_thread_memory.h"
#include "protocol_network_connection.h"

/*!
 *  \brief Namespace for ProtocolModuleInstantiator class
 */
namespace protocol_module_instantiator
{
	using std::unique_ptr;

	using vector_t::vector_type;

	using protocol_messages::identifier_t;
	using protocol_messages::thread_multi_module_shared_ptr_t;
	using id_vector_t = protocol_messages::thread_multi_module_manager_t::id_vector_t;

	using protocol_thread_memory::ProtocolThreadMemory;
	using protocol_network_connection::ProtocolNetworkConnection;

	struct instantiation_data_t
	{
		/*!
		 * \brief Memory shared by thread modules
		 */
		ProtocolThreadMemory &Memory;

		/*!
		 * \brief Connection to peer. Don't use this to send data! Usen SendHandle in Memory instead!
		 */
		ProtocolNetworkConnection &Connection;
	};

	/*!
	 * \brief Creates new modules for each instance
	 */
	class ProtocolModuleInstantiator
	{
		public:
			using instantiation_data_t = protocol_module_instantiator::instantiation_data_t;
			using module_id_vector_t = vector_type<identifier_t::module_id_t>;

			/*!
			 * 	\brief Constructor
			 */
			ProtocolModuleInstantiator(identifier_t::module_id_t ModuleID, id_vector_t &&ExternalSenderLinks, module_id_vector_t &&InternalSenderLinks, id_vector_t &&ExternalReceiverLinks, module_id_vector_t &&InternalReceiverLinks);

			virtual ~ProtocolModuleInstantiator() = default;

			thread_multi_module_shared_ptr_t CreateNewInstance(instantiation_data_t &Instance) const;

			id_vector_t CreateSenderLinkVector(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID) const;
			id_vector_t CreateReceiverLinkVector(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID) const;

			identifier_t::module_id_t GetModuleID() const;

		private:

			virtual thread_multi_module_shared_ptr_t CreateNewInstanceHandle(instantiation_data_t &Instance) const = 0;

			/*!
			 * \brief Links to modules outside of the newly created thread
			 */
			id_vector_t _ExternalReceiverLinks;

			/*!
			 * \brief Links to modules inside of the newly created thread
			 */
			module_id_vector_t _InternalReceiverLinks;

			/*!
			 * \brief Links to modules outside of the newly created thread
			 */
			id_vector_t _ExternalSenderLinks;

			/*!
			 * \brief Links to modules inside of the newly created thread
			 */
			module_id_vector_t _InternalSenderLinks;

			/*!
			 * \brief ID of the created modules
			 */
			identifier_t::module_id_t _ModuleID;

			static id_vector_t CreateLinkVector(const id_vector_t &ExternalIDLinks, const module_id_vector_t &InternalIDLinks, identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID);
	};

	using ProtocolModuleInstantiatorUniquePtr = unique_ptr<ProtocolModuleInstantiator>;
} // namespace protocol_module_instantiator


#endif // PROTOCOL_MODULE_INSTANTIATOR_H
