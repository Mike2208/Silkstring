#ifndef VECTOR_T_H
#define VECTOR_T_H

/*! \file vector_t.h
 *  \brief Header for vector_t class
 */

#include <vector>
#include <initializer_list>

/*!
 *  \brief Namespace for vector_t class
 */
namespace vector_t
{
	template<class T>
	using vector_type = std::vector<T>;

	using byte_t = unsigned char;
	using byte_vector_t = vector_type<byte_t>;

	using std::initializer_list;

	/*!
	 * \brief Vector class used in this project
	 */
	template<class T, class TAG = void>
	class vector_t : public vector_type<T>
	{
			template<class CmpValue>
			using cmp_fcn_t = bool(const T& Element, CmpValue ComparisonValue);

		public:			
			/*!
			 * 	brief Constructor
			 */
			vector_t() = default;

			explicit vector_t(typename vector_type<T>::size_type __n, const typename vector_type<T>::allocator_type& __a = typename vector_type<T>::allocator_type()) : vector_type<T>(__n, __a)
			{}

			vector_t(initializer_list<T> __l, const typename vector_type<T>::allocator_type& __a = typename vector_type<T>::allocator_type()) : vector_type<T>(__l, __a)
			{}

			template<class CmpValue>
			typename vector_type<T>::iterator Find(CmpValue ComparisonValue, cmp_fcn_t<CmpValue> *CompareFunction = [] (const T &Element, CmpValue ComparisonValue) { return Element == ComparisonValue; } )
			{
				for(auto curIterator = this->begin(); curIterator != this->end(); ++curIterator)
				{
					// Use comparison function to check equality
					if(CompareFunction(*curIterator, ComparisonValue))
						return curIterator;
				}

				return this->end();
			}

			template<class CmpValue>
			typename vector_type<T>::const_iterator Find(CmpValue ComparisonValue, cmp_fcn_t<CmpValue> *CompareFunction = [] (const T &Element, CmpValue ComparisonValue) { return Element == ComparisonValue; } ) const
			{
				for(typename vector_type<T>::const_iterator curIterator = this->begin(); curIterator != this->end(); ++curIterator)
				{
					// Use comparison function to check equality
					if(CompareFunction(*curIterator, ComparisonValue))
						return curIterator;
				}

				return this->end();
			}

		private:
	};
} // namespace vector_t


#endif // VECTOR_T_H
