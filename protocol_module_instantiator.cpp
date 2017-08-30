#include "protocol_module_instantiator.h"

namespace protocol_module_instantiator
{
	ProtocolModuleInstantiator::ProtocolModuleInstantiator(identifier_t::module_id_t ModuleID, id_vector_t &&ExternalSenderLinks, module_id_vector_t &&InternalSenderLinks, id_vector_t &&ExternalReceiverLinks, module_id_vector_t &&InternalReceiverLinks)
		: _ExternalReceiverLinks(std::move(ExternalReceiverLinks)),
		  _InternalReceiverLinks(std::move(InternalReceiverLinks)),
		  _ExternalSenderLinks(std::move(ExternalSenderLinks)),
		  _InternalSenderLinks(std::move(InternalSenderLinks)),
		  _ModuleID(ModuleID)
	{}

	id_vector_t ProtocolModuleInstantiator::CreateSenderLinkVector(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID) const
	{
		return ProtocolModuleInstantiator::CreateLinkVector(this->_ExternalSenderLinks, this->_InternalSenderLinks, QueueID, ProtocolThreadID);
	}

	id_vector_t ProtocolModuleInstantiator::CreateReceiverLinkVector(identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID) const
	{
		return ProtocolModuleInstantiator::CreateLinkVector(this->_ExternalReceiverLinks, this->_InternalReceiverLinks, QueueID, ProtocolThreadID);
	}

	identifier_t::module_id_t ProtocolModuleInstantiator::GetModuleID() const
	{
		return this->_ModuleID;
	}

	thread_multi_module_shared_ptr_t ProtocolModuleInstantiator::CreateNewInstance(instantiation_data_t &Instance) const
	{
		return this->CreateNewInstanceHandle(Instance);
	}

	id_vector_t ProtocolModuleInstantiator::CreateLinkVector(const id_vector_t &ExternalIDLinks, const module_id_vector_t &InternalIDLinks, identifier_t::queue_id_t QueueID, identifier_t::thread_id_t ProtocolThreadID)
	{
		id_vector_t retVal = ExternalIDLinks;

		retVal.reserve(retVal.size() + InternalIDLinks.size());

		for(const auto &curModuleID : InternalIDLinks)
		{
			retVal.push_back(identifier_t(QueueID, curModuleID, ProtocolThreadID));
		}

		return retVal;
	}
}
