// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_TOPOLOGICAL_DIMENSION_HPP
#define _GEOMETRY_TOPOLOGICAL_DIMENSION_HPP


#include <boost/type_traits/remove_const.hpp>
#include <boost/mpl/int.hpp>

#include <geometry/core/tag.hpp>
#include <geometry/core/tags.hpp>


namespace geometry
{


	#ifndef DOXYGEN_NO_DISPATCH
	namespace core_dispatch
	{
		template <typename TAG, typename G>
		struct top_dim {};

		template <typename G> struct top_dim<point_tag, G>      : boost::mpl::int_<0> {};
		template <typename G> struct top_dim<linestring_tag, G> : boost::mpl::int_<1> {};
		template <typename G> struct top_dim<segment_tag, G>    : boost::mpl::int_<1> {};
		template <typename G> struct top_dim<ring_tag, G>       : boost::mpl::int_<2> {};
		template <typename G> struct top_dim<box_tag, G>        : boost::mpl::int_<2> {};
		template <typename G> struct top_dim<polygon_tag, G>    : boost::mpl::int_<2> {};

		// check, sphere=3?
		template <typename G> struct top_dim<nsphere_tag, G>    : boost::mpl::int_<2> {};



	} // namespace core_dispatch
	#endif




	/*!
		\brief Meta-function returning gives the topological dimension of a geometry
		\details The topological dimension defines a point as 0-dimensional, a linestring as 1-dimensional,
			and a ring or polygon as 2-dimensional.
		\see http://www.math.okstate.edu/mathdept/dynamics/lecnotes/node36.html
		\ingroup core
	*/
	template <typename G>
	struct topological_dimension
		: core_dispatch::top_dim
			< typename tag<G>::type , typename boost::remove_const<G>::type >
	{ };


}


#endif
