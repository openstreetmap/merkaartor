// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_MAKE_HPP
#define _GEOMETRY_MAKE_HPP


#include <geometry/algorithms/assign.hpp>

namespace geometry
{

	/*!
		\brief Make a geometry
		\ingroup access
		\details The Geometry Library uses concepts for all its geometries. Therefore it does not rely
		on constructors. The "make" functions are object generators creating geometries. There are overloads
		with two, three, four or six values, which are implemented depending on the geometry specified.
		\tparam G the geometry type
		\tparam T the coordinate type
		\return the geometry
	 */
	template <typename G, typename T>
	inline G make(const T& coor1, const T& coor2)
	{
		G geometry;
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2);
		return geometry;
	}


	/*!
		\brief Make a geometry
		\ingroup access
		\return a 3D point, a circle
	 */
	template <typename G, typename T>
	inline G make(const T& coor1, const T& coor2, const T& coor3)
	{
		G geometry;
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2, coor3);
		return geometry;
	}

	template <typename G, typename T>
	inline G make(const T& coor1, const T& coor2, const T& coor3, const T& coor4)
	{
		G geometry;
		dispatch::assignment<typename tag<G>::type, G>::assign(geometry, coor1, coor2, coor3, coor4);
		return geometry;
	}

	template <typename G, typename R>
	inline G make(const R& range)
	{
		G geometry;
		append(geometry, range);
		return geometry;
	}


	/*!
		\brief Create a box with inverse infinite coordinates
		\ingroup access
		\details The make_inverse function initialize a 2D or 3D box with large coordinates, the
		min corner is very large, the max corner is very small
		\tparam G the geometry type
		\return the box
	 */
	template <typename G>
	inline G make_inverse()
	{
		G geometry;
		dispatch::assignment<typename tag<G>::type, G>::assign_inverse(geometry);
		return geometry;
	}

	/*!
		\brief Create a geometry with "zero" coordinates
		\ingroup access
		\details The make_zero function initializes a 2D or 3D point or box with coordinates of zero
		\tparam G the geometry type
		\return the geometry
	 */
	template <typename G>
	inline G make_zero()
	{
		G geometry;
		dispatch::assignment<typename tag<G>::type, G>::assign_zero(geometry);
		return geometry;
	}

};

#endif // _GEOMETRY_MAKE_HPP
