#ifndef CONTAINER_FUNCTIONS_H
#define CONTAINER_FUNCTIONS_H

/*! \file container_functions.h
 *  \brief Header for ContainerFunctions class
 */


#include <algorithm>

/*!
 *  \brief Namespace for ContainerFunctions class
 */
namespace container_functions
{
	using std::sort;

	/*!
	 * \brief Functions for manipulating containers
	 */
	template<class Container>
	class ContainerFunctions
	{
		public:
			using const_iterator_t	= typename Container::const_iterator;
			using iterator_t		= typename Container::iterator;
			using element_t			= typename Container::value_type;

			/*!
			 *	\brief Pushes new Element to end of container if no existing element that equals CompareValue was found
			 *	\param Begin StartIterator
			 *	\param End End Iterator
			 *	\param Element Element to push
			 *	\param PushFunction Function that pushes element to end of container. Format: void(element_t &&Element)
			 *	\param CompareValue Value to compare elements in container to
			 *	\param CompareFcn Function to compare element in container to CompareValue. Format: bool(const element_t &Element, CompareVal CmpVal)
			 */
			template<class CompareVal, class CompareFcn, class PushFcn>
			static void PushIfNotFound(iterator_t Begin, iterator_t End, element_t &&Element, PushFcn PushFunction, CompareVal CompareValue, CompareFcn CompareFunction = [](const element_t &Element, CompareVal CmpVal) { return Element == CmpVal; })
			{
				static_assert(std::is_same<PushFcn, void(element_t &&)>::value, "ERROR ContainerFunctions::PushIfNotFound(): PushFcn has wrong type\n");

				if(Find<CompareVal, CompareFcn>(Begin, End, CompareValue, CompareFunction) == End)
					PushFunction(std::move(Element));
			}

			/*!
			 *	\brief Inserts a new element before an existing element in the container that equals CompareValue, or at end of container if not found
			 *	\param Begin StartIterator
			 *	\param End End Iterator
			 *	\param Element Element to insert
			 *	\param InsertFunction Function that inserts element after Iterator that equals CompareValue. Format: void(iterator_t, element_t &&Element)
			 *	\param CompareValue Value to compare elements in container to
			 *	\param CompareFcn Function to compare element in container to CompareValue. Format: bool(const element_t &Element, CompareVal CmpVal)
			 */
			template<class CompareVal, class CompareFcn, class InsertFcn>
			static void InsertBeforeCompare(iterator_t Begin, iterator_t End, element_t &&Element, InsertFcn InsertFunction, CompareVal CompareValue, CompareFcn CompareFunction = [](const element_t &Element, CompareVal CmpVal) { return Element == CmpVal; })
			{
				static_assert(std::is_same<InsertFcn, void(iterator_t, element_t&&)>::value, "ERROR ContainerFunctions::InsertBeforeCompare(): InsertFcn has wrong type\n");

				auto foundIterator = Find<CompareVal, CompareFcn>(Begin, End, CompareValue, CompareFunction);
				InsertFunction(foundIterator, std::move(Element));
			}

			/*!
			 *	\brief Finds an element in the container that equals CompareValue
			 *	\param Begin StartIterator
			 *	\param End End Iterator
			 *	\param Element Element to insert
			 *	\param InsertFunction Function that inserts element after Iterator that equals CompareValue. Format: void(iterator_t)
			 *	\param CompareValue Value to compare elements in container to
			 *	\param CompareFcn Function to compare element in container to CompareValue. Format: bool(const element_t&, CompareVal)
			 *	\return Returns the first iterator in the container that corresponds to CompareValue
			 */
			template<class CompareVal, class CompareFcn>
			static iterator_t Find(iterator_t Begin, iterator_t End, CompareVal CompareValue, CompareFcn CompareFunction = [](const element_t &Element, CompareVal CmpVal) { return Element == CmpVal; })
			{
				static_assert(std::is_same<CompareFcn, void(const element_t &Element, CompareVal)>::value, "ERROR ContainerFunctions::Find(): CompareFunction has wrong type\n");

				while(Begin != End)
				{
					if(CompareFunction(*Begin, CompareValue))
						return Begin;

					Begin++;
				}

				return Begin;
			}

			template<class CompareVal, class CompareFcn>
			static const_iterator_t Find(const_iterator_t Begin, const_iterator_t End, CompareVal CompareValue, CompareFcn CompareFunction = [](const element_t &Element, CompareVal CmpVal) { return Element == CmpVal; })
			{
				static_assert(std::is_same<CompareFcn, void(const element_t &Element, CompareVal)>::value, "ERROR ContainerFunctions::Find(): CompareFunction has wrong type\n");

				while(Begin != End)
				{
					if(CompareFunction(*Begin, CompareValue))
						return Begin;

					Begin++;
				}

				return Begin;
			}

			template<class LessThanFcn>
			static void SimpleSort(iterator_t Begin, iterator_t End, LessThanFcn CompFcn = [](const element_t &Element1, const element_t &Element2) { return Element1 < Element2; })
			{
				//struct compClass
				//{
				//	bool operator()(const element_t &Element1, const element_t &Element2) const
				//	{	return CompFcn(Element1, Element2);	}
				//};

				sort(Begin, End, [CompFcn](const element_t &Element1, const element_t &Element2) { return CompFcn(Element1, Element2); });
			}

			template<class LessThanFcn>
			static void SimpleSort(const_iterator_t Begin, const_iterator_t End, LessThanFcn CompFcn = [](const element_t &Element1, const element_t &Element2) { return Element1 < Element2; })
			{
				sort(Begin, End, [CompFcn](const element_t &Element1, const element_t &Element2) { return CompFcn(Element1, Element2); });
			}
	};
} // namespace container_functions


#endif // CONTAINER_FUNCTIONS_H
