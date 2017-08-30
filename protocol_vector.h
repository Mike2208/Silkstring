#ifndef PROTOCOL_VECTOR_H
#define PROTOCOL_VECTOR_H

/*! \file protocol_vector.h
 *  \brief Header for p class
 */


#include "vector_t.h"

/*!
 *  \brief Namespace for p class
 */
namespace protocol_vector
{
	using vector_t::byte_t;
	using vector_t::byte_vector_t;

	using protocol_vector_size_t = byte_vector_t::size_type;

	struct protocol_vector_t : public byte_vector_t
	{
		byte_vector_t::iterator CurPos = this->begin();

		/*!
		 * \brief Gets remaining vector size from CurPos to end()
		 */
		size_type GetRestSize() const;

		size_type GetRestSize(const byte_vector_t::iterator PositionInVector) const;

		/*!
		 * \brief Constructor
		 */
		protocol_vector_t();

		/*!
		 * \brief Constructor
		 */
		explicit protocol_vector_t(byte_vector_t &&ProtocolData);

		/*!
		 * \brief Constructor
		 */
		protocol_vector_t(byte_vector_t::iterator StartPos, byte_vector_t::iterator EndPos);

		protocol_vector_t(const byte_vector_t::size_type VectorSize);

		template<class T>
		T *ParseVector(byte_vector_t::iterator &PositionInVector)
		{
			if(this->GetRestSize(PositionInVector) < sizeof(T))
				return nullptr;

			auto pRetVal = reinterpret_cast<T*>(&(*this->CurPos));
			PositionInVector += sizeof(T);

			return pRetVal;
		}

		template<class T>
		T *ParseVector()
		{
			return this->ParseVector<T>(this->CurPos);
		}
	};
} // namespace protocol_vector


#endif // PROTOCOL_VECTOR_H
