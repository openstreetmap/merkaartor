// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_PROMOTION_TRAITS_HPP
#define _GEOMETRY_PROMOTION_TRAITS_HPP

#include <geometry/core/coordinate_type.hpp>


/*!
\defgroup utility utility: utilities
*/



namespace geometry
{
	/*!
		\brief Traits class to select, of two types, the most accurate type for calculations
		\ingroup utility
		\details The promotion traits classes, base class and specializations, compares two types on compile time.
		For example, if an addition must be done with a double and an integer, the result must be a double.
		If both types are integer, the results can be an integer.
		The select_type_traits class and its specializations define the appropriate type in the member type <em>type</em>.
		\note Might be replaced by the new promotion_traits class of boost.
	*/
	template<typename T1, typename T2>
	struct select_type_traits
	{
		typedef T1 type;
	};


	#ifndef DOXYGEN_NO_SPECIALIZATIONS
	// (Partial) specializations

	// Any combination with double will define a double
	template<typename T> struct select_type_traits<double, T> { typedef double type; };
	template<typename T> struct select_type_traits<T, double> { typedef double type; };
	// Avoid ambiguity for the double/double case
	template<> struct select_type_traits<double, double> { typedef double type; };

	// List other cases
	template<> struct select_type_traits<int, float> { typedef float type; };
	template<> struct select_type_traits<float, int> {	typedef float type; };


	// to be extended

	#endif




	/*!
		\brief Utility selecting the most appropriate coordinate type.
		\ingroup utility
	 */
	template <typename PC1, typename PC2>
	struct select_coordinate_type
	{
		typedef typename select_type_traits<
			typename coordinate_type<PC1>::type,
			typename coordinate_type<PC2>::type
		>::type type;
	};



}

#endif // _GEOMETRY_PROMOTION_TRAITS_HPP
