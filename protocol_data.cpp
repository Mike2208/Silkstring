#include "protocol_data.h"

namespace protocol_data
{
	bool protocol_header_name_t::operator==(const protocol_header_name_t &Other) const
	{
		if(this->size() != Other.size())
			return false;

		auto curIterator = this->begin();
		auto otherIterator = Other.begin();

		while(curIterator != this->end())
		{
			if(*curIterator != *otherIterator)
				return false;

			curIterator++;
			otherIterator++;
		}

		return true;
	}

	bool protocol_header_name_t::operator!=(const protocol_header_name_t &Other) const
	{
		return !(*this == Other);
	}

	bool protocol_header_name_t::operator<(const protocol_header_name_t &Other) const
	{
		if(this->size() < Other.size())
			return true;
		else if(this->size() > Other.size())
			return false;

		auto curIterator = this->begin();
		auto otherIterator = Other.begin();

		while(curIterator != this->end())
		{
			if(*curIterator < *otherIterator)
				return true;
			else if(*curIterator > *otherIterator)
				return false;

			curIterator++;
			otherIterator++;
		}

		// Both are equal
		return false;
	}
}
