#ifndef TYPESAFE_CLASS_H
#define TYPESAFE_CLASS_H

/*!	\file typesafe_class.h
 *	\brief Header for a general typesafe class
 */

//#include <boost/type_traits.hpp>
#include <string>

// Addition operators
#define SINGLE_ADD_OPERATOR( RETURN_CLASS, CLASS_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)									\
/*!																																		\
* \brief operator +	Adds RIGHT_HAND_CLASS to this class																					\
* \param rhs RIGHT_HAND_CLASS for addition to VALUE_NAME																				\
* \return Returns RETURN_CLASS( this->CLASS_VALUE_NAME + rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
RETURN_CLASS operator+(const RIGHT_HAND_CLASS rhs) const { return RETURN_CLASS(this->CLASS_VALUE_NAME + rhs.RIGHT_HAND_VALUE_NAME); }

#define SINGLE_ADD_EQUALS_OPERATOR( PARENT_CLASS, PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)							\
/*!																																		\
* \brief operator += Adds RIGHT_HAND_CLASS to this class																				\
* \param rhs RIGHT_HAND_CLASS for addition to VALUE_NAME																				\
* \return Returns PARENT_CLASS( this->PARENT_VALUE_NAME += rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
PARENT_CLASS operator+=(const RIGHT_HAND_CLASS rhs) { this->PARENT_VALUE_NAME += rhs.RIGHT_HAND_VALUE_NAME; return *this; }


// Subtraction operators
#define SINGLE_SUBTRACT_OPERATOR( RETURN_CLASS, CLASS_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)								\
/*!																																		\
* \brief operator -	Subtracts RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for subtraction to VALUE_NAME																				\
* \return Returns RETURN_CLASS( this->CLASS_VALUE_NAME - rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
RETURN_CLASS operator-(const RIGHT_HAND_CLASS rhs) const { return RETURN_CLASS(this->CLASS_VALUE_NAME - rhs.RIGHT_HAND_VALUE_NAME); }

#define SINGLE_SUBTRACT_EQUALS_OPERATOR( PARENT_CLASS, PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)						\
/*!																																		\
* \brief operator -= Subtracts RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for subtraction to VALUE_NAME																				\
* \return Returns PARENT_CLASS( this->PARENT_VALUE_NAME -= rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
PARENT_CLASS operator-=(const RIGHT_HAND_CLASS rhs) { this->PARENT_VALUE_NAME -= rhs.RIGHT_HAND_VALUE_NAME; return *this; }


// Multiplication operators
#define SINGLE_MULTIPLY_OPERATOR( RETURN_CLASS, CLASS_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)								\
/*!																																		\
* \brief operator *	Multiplies RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for multiplication with VALUE_NAME																				\
* \return Returns RETURN_CLASS( this->CLASS_VALUE_NAME * rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
RETURN_CLASS operator*(const RIGHT_HAND_CLASS rhs) const { return RETURN_CLASS(this->CLASS_VALUE_NAME * rhs.RIGHT_HAND_VALUE_NAME); }

#define SINGLE_MULTIPLY_EQUALS_OPERATOR( PARENT_CLASS, PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)						\
/*!																																		\
* \brief operator *= Multiplies RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for multiplication with VALUE_NAME																				\
* \return Returns PARENT_CLASS( this->PARENT_VALUE_NAME *= rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
PARENT_CLASS operator*=(const RIGHT_HAND_CLASS rhs) { this->PARENT_VALUE_NAME *= rhs.RIGHT_HAND_VALUE_NAME; return *this; }


// Division operators
#define SINGLE_DIVIDE_OPERATOR( RETURN_CLASS, CLASS_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)								\
/*!																																		\
* \brief operator /	Divides RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for division with VALUE_NAME																				\
* \return Returns RETURN_CLASS( this->CLASS_VALUE_NAME / rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
RETURN_CLASS operator/(const RIGHT_HAND_CLASS rhs) const { return RETURN_CLASS(this->CLASS_VALUE_NAME / rhs.RIGHT_HAND_VALUE_NAME); }

#define SINGLE_DIVIDE_EQUALS_OPERATOR( PARENT_CLASS, PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME)						\
/*!																																		\
* \brief operator /= Divides RIGHT_HAND_CLASS to this class																			\
* \param rhs RIGHT_HAND_CLASS for division with VALUE_NAME																				\
* \return Returns PARENT_CLASS( this->PARENT_VALUE_NAME /= rhs.RIGHT_HAND_VALUE_NAME )													\
*/																																		\
PARENT_CLASS operator/=(const RIGHT_HAND_CLASS rhs) { this->PARENT_VALUE_NAME /= rhs.RIGHT_HAND_VALUE_NAME; return *this; }


/*!
 *	\brief Add operators + and += to PARENT_CLASS that add VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define ADD_OPERATORS( PARENT_CLASS, VALUE_NAME )										\
SINGLE_ADD_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)					\
SINGLE_ADD_EQUALS_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)


/*!
 *	\brief Add operators - and -= to PARENT_CLASS that subtract VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define SUBTRACTION_OPERATORS( PARENT_CLASS, VALUE_NAME )								\
SINGLE_SUBTRACT_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)			\
SINGLE_SUBTRACT_EQUALS_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)


/*!
 *	\brief Add operators * and *= to PARENT_CLASS that multiply VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define MULTIPLICATION_OPERATORS( PARENT_CLASS, VALUE_NAME )							\
SINGLE_MULTIPLY_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)			\
SINGLE_MULTIPLY_EQUALS_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)


/*!
 *	\brief Add operators / and /= to PARENT_CLASS that divide VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define DIVISION_OPERATORS( PARENT_CLASS, VALUE_NAME )								\
SINGLE_DIVIDE_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)			\
SINGLE_DIVIDE_EQUALS_OPERATOR(PARENT_CLASS, VALUE_NAME, PARENT_CLASS, VALUE_NAME)

/*!
 *	\brief Add pre- and post-operators ++ to PARENT_CLASS that increment VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define INCREMENT_OPERATORS( PARENT_CLASS, VALUE_NAME )																\
PARENT_CLASS operator++() { return PARENT_CLASS(this->VALUE_NAME++); }												\
PARENT_CLASS &operator++(const int) { ++this->VALUE_NAME; return *this; }

/*!
 *	\brief Add pre- and post-operators -- to PARENT_CLASS that decrement VALUE_NAME
 *
 *	Simple operators are added to PARENT_CLASS class.
 */
#define DECREMENT_OPERATORS( PARENT_CLASS, VALUE_NAME )																\
PARENT_CLASS operator--() { return PARENT_CLASS(this->VALUE_NAME--); }												\
PARENT_CLASS &operator--(const int) { --this->VALUE_NAME; return *this; }


#define GET_OPERATOR( VALUE_TYPE, VALUE_NAME )																		\
constexpr VALUE_TYPE get() const { return this->VALUE_NAME; }


#define DIFF_EQUALITY_OPERATORS( PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME )							\
bool operator==(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME == rhs.RIGHT_HAND_VALUE_NAME); }	\
bool operator!=(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME != rhs.RIGHT_HAND_VALUE_NAME); }

#define DIFF_COMPARISON_OPERATORS( PARENT_VALUE_NAME, RIGHT_HAND_CLASS, RIGHT_HAND_VALUE_NAME )							\
bool operator<(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME < rhs.RIGHT_HAND_VALUE_NAME); }		\
bool operator<=(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME <= rhs.RIGHT_HAND_VALUE_NAME); }	\
bool operator>(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME > rhs.RIGHT_HAND_VALUE_NAME); }		\
bool operator>=(const RIGHT_HAND_CLASS rhs) const { return (this->PARENT_VALUE_NAME >= rhs.RIGHT_HAND_VALUE_NAME); }


#define EQUALITY_OPERATORS( PARENT_CLASS, VALUE_NAME )																\
DIFF_EQUALITY_OPERATORS(VALUE_NAME, PARENT_CLASS, VALUE_NAME)

#define COMPARISON_OPERATORS( PARENT_CLASS, VALUE_NAME )															\
DIFF_COMPARISON_OPERATORS(VALUE_NAME, PARENT_CLASS, VALUE_NAME)

#define ARITHMETIC_OPERATORS( PARENT_CLASS, VALUE_NAME )			\
ADD_OPERATORS(PARENT_CLASS, VALUE_NAME)								\
SUBTRACTION_OPERATORS(PARENT_CLASS, VALUE_NAME)						\
MULTIPLICATION_OPERATORS(PARENT_CLASS, VALUE_NAME)					\
DIVISION_OPERATORS(PARENT_CLASS, VALUE_NAME)

#define ALL_COMPARISON_OPERATORS( PARENT_CLASS, _VALUE_NAME )		\
EQUALITY_OPERATORS(PARENT_CLASS, VALUE_NAME)						\
COMPARISON_OPERATORS(PARENT_CLASS, VALUE_NAME)

#define EXPLICIT_CONVERSION_OPERATORS( VALUE_CLASS, VALUE_NAME )			\
explicit operator VALUE_CLASS() const { return this->VALUE_NAME; }			\
explicit operator VALUE_CLASS&() { return this->VALUE_NAME; }

#define SIMPLE_CONSTRUCTOR( PARENT_CLASS, VALUE_CLASS, VALUE_NAME )		\
explicit PARENT_CLASS(const VALUE_CLASS Value) : VALUE_NAME(Value)	{}

#define EXPLICIT_STRING_OPERATOR( VALUE_NAME )										\
explicit operator std::string() const { return std::to_string(this->VALUE_NAME); }

#endif // TYPESAFE_CLASS_H
