// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_ASSIGN_BOX_CORNER_HPP
#define _GEOMETRY_ASSIGN_BOX_CORNER_HPP


#include <boost/numeric/conversion/cast.hpp>

#include <geometry/core/coordinate_dimension.hpp>

// TODO: merge with "assign"


namespace geometry
{

	/*!
		\brief Assign one point of a 2D box
		\ingroup assign
		\todo will be merged with assign
	*/
	template <size_t C1, size_t C2, typename B, typename P>
	inline void assign_box_corner(const B& box, P& point)
	{
		// Be sure both are 2-Dimensional
		assert_dimension<B, 2>();
		assert_dimension<P, 2>();

		// Copy coordinates
		typedef typename coordinate_type<P>::type T;
		set<0>(point, boost::numeric_cast<T>(get<C1, 0>(box)));
		set<1>(point, boost::numeric_cast<T>(get<C2, 1>(box)));
	}

	/*!
		\brief Assign the 4 points of a 2D box
		\ingroup assign
		\todo will be merged with assign
		\note The order can be crucial. Most logical is LOWER, UPPER and sub-order LEFT, RIGHT
	*/
	template <typename B, typename P>
	inline void assign_box_corners(const B& box, P& lower_left, P& lower_right, P& upper_left, P& upper_right)
	{
		assign_box_corner<min_corner, min_corner>(box, lower_left);
		assign_box_corner<max_corner, min_corner>(box, lower_right);
		assign_box_corner<min_corner, max_corner>(box, upper_left);
		assign_box_corner<max_corner, max_corner>(box, upper_right);
	}


} // namespace


#endif //_GEOMETRY_ASSIGN_BOX_CORNER_HPP
