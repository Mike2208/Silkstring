#include "protocol_vector.h"
#include <assert.h>

namespace protocol_vector
{
	protocol_vector_t::size_type protocol_vector_t::GetRestSize() const
	{
		return this->GetRestSize(this->CurPos);
	}

	protocol_vector_t::size_type protocol_vector_t::GetRestSize(const byte_vector_t::iterator PositionInVector) const
	{
		assert(PositionInVector >= this->begin() && PositionInVector <= this->end());

		return this->end()-PositionInVector;
	}

	protocol_vector_t::protocol_vector_t() : byte_vector_t(), CurPos(begin())
	{}

	protocol_vector_t::protocol_vector_t(byte_vector_t &&ProtocolData) : byte_vector_t(std::move(ProtocolData)), CurPos(begin())
	{}

	protocol_vector_t::protocol_vector_t(byte_vector_t::iterator StartPos, byte_vector_t::iterator EndPos)
		: byte_vector_t(StartPos, EndPos)
	{}

	protocol_vector_t::protocol_vector_t(byte_vector_t::size_type VectorSize)
		: byte_vector_t(VectorSize)
	{}
}
