#ifndef MULTI_CONTAINER_H
#define MULTI_CONTAINER_H

/*! \file multi_container.h
 *  \brief Header for MultiContainer class
 */


#include <list>

#include "dynamic_pointer.h"

/*!
 *  \brief Namespace for MultiContainer class
 */
namespace multi_container
{
	using std::list;

	using dynamic_pointer::DynamicPointer;

	template<class T>
	class MultiContainer;

	template<class T>
	class MultiContainerElement;

	template<class T>
	class MultiContainerIterator
	{
		public:
			using dyn_void_ptr_t = DynamicPointer<void>;

			using container_t = MultiContainer<T>;
			using sub_container_t = typename container_t::sub_container_t;
			using sub_container_iterator_t = typename container_t::sub_container_iterator_t;

			using iterator_t = MultiContainerIterator<T>;
			using iterator_range_fcn_t = iterator_t(sub_container_t *);
			using iterator_cmp_fcn_t = bool(const dyn_void_ptr_t&, const dyn_void_ptr_t&);

			using iterator_base_fcn_t = T*(iterator_t &);

			MultiContainerIterator(dyn_void_ptr_t &&SubIteratorPtr, sub_container_iterator_t SubContainerIterator)
				:	_SubIteratorPtr(std::move(SubIteratorPtr)),
					_SubContainerIterator(SubContainerIterator)
			{}

			T *operator*(iterator_t &S)
			{
				return S->base();
			}

			T *base()
			{
				return this->_BaseFcn(this->_SubIteratorPtr);
			}

		private:
			/*!
			 * \brief Pointer to sub iterator
			 */
			dyn_void_ptr_t		_SubIteratorPtr;

			/*!
			 * \brief Iterator over current iterators
			 */
			sub_container_iterator_t		_SubContainerIterator;

			friend class MultiContainerElement<T>;
	};

	/*!
	 * \brief Element of the MultiContainer
	 */
	template<class T>
	class MultiContainerElement
	{
		public:

			using iterator_t = typename MultiContainerIterator<T>::iterator_t;

			using dyn_void_ptr_t = typename iterator_t::dyn_void_ptr_t;

			using iterator_range_fcn_t = typename iterator_t::iterator_range_fcn_t;
			using iterator_cmp_fcn_t = typename iterator_t::iterator_cmp_fcn_t;
			using iterator_base_fcn_t = typename iterator_t::iterator_base_fcn_t;

			template<class ContainerType>
			MultiContainerElement(ContainerType &&Container)
				:	_ContainerData(new ContainerType(std::move(Container))),
					_BeginFcn([] (MultiContainerElement<T> *Container) { return iterator_t(Container->_ContainerData.template Get<ContainerType>()->begin()->base(), Container); } ),
					_EndFcn([] (MultiContainerElement<T> *Container) { return iterator_t(Container->_ContainerData.template Get<ContainerType>()->end()->base(), Container); } ),
					_EqualFcn([] (const dyn_void_ptr_t &Iterator1, const dyn_void_ptr_t &Iterator2) { return *(Iterator1.template Get<ContainerType>()) == *(Iterator2._SubIterator.template Get<ContainerType>()); }),
					_UnequalFcn([] (const dyn_void_ptr_t &Iterator1, const dyn_void_ptr_t &Iterator2) { return *(Iterator1.template Get<ContainerType>()) != *(Iterator2._SubIterator.template Get<ContainerType>()); }),
					_BaseFcn([] (dyn_void_ptr_t &IteratorPtr) { return (*(IteratorPtr.template Get<ContainerType>()))->base(); })
			{
				static_assert(std::is_same<T, typename ContainerType::value_type>::value);
			}

			iterator_t begin()
			{
				return this->_BeginFcn(this);
			}

			iterator_t end()
			{
				return this->_EndFcn(this);
			}

			T *GetBaseData(dyn_void_ptr_t &IteratorPtr)
			{
				return this->_BaseFcn(IteratorPtr);
			}

		private:

			dyn_void_ptr_t				_ContainerData;

			iterator_range_fcn_t		*const _BeginFcn;
			iterator_range_fcn_t		*const _EndFcn;

			iterator_cmp_fcn_t			*const _EqualFcn;
			iterator_cmp_fcn_t			*const _UnequalFcn;

			iterator_base_fcn_t			*const _BaseFcn;

			friend class MultiContainerIterator<T>;
	};

	/*!
	 * \brief Class containing multiple different storage types and an iterator over them all
	 */
	template<class T>
	class MultiContainer
	{
			using sub_container_t = MultiContainer<T>;
			using sub_container_list_t = list<sub_container_t>;

		public:
			using container_iterator_t = typename sub_container_list_t::iterator;

			/*!
			 * 	brief Constructor
			 */
			MultiContainer();

		private:

			/*!
			 * \brief All containers that can be accessed
			 */
			sub_container_list_t _Containers;
	};
} // namespace multi_container


#endif // MULTI_CONTAINER_H
