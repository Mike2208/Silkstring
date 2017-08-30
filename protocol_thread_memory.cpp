#include "protocol_thread_memory.h"

namespace protocol_thread_memory
{
	bool protocol_header_module_map_t::AlreadyRegistered(map_t::key_type HeadeName, map_t::mapped_type ModuleID) const
	{
		auto curIterator = this->find(HeadeName);
		while(curIterator != this->end() &&
			  curIterator->first == HeadeName)
		{
			if(curIterator->second == ModuleID)
				return true;
		}

		return false;
	}

	bool protocol_header_module_map_t::AlreadyRegistered(map_t::value_type Pair) const
	{
		return this->AlreadyRegistered(Pair.first, Pair.second);
	}
}
