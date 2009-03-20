// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_POINT_TYPE_HPP
#define _GEOMETRY_POINT_TYPE_HPP


#include <boost/type_traits/remove_const.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/tag.hpp>
#include <geometry/core/tags.hpp>
#include <geometry/core/ring_type.hpp>

namespace geometry
{

	namespace traits
	{

		/*!
			\brief Traits class indicating the type of contained points
			\ingroup traits
			\par Geometries:
				- all geometries except point
			\par Specializations should provide:
				- typedef P type (where P should fulfil the Point concept)
			\tparam G geometry
		*/
		template <typename G>
		struct point_type {};

	} // namespace traits




	#ifndef DOXYGEN_NO_DISPATCH
	namespace core_dispatch
	{


		template <typename TAG, typename G>
		struct point_type
		{
			// Default: call traits to get point type
			typedef typename boost::remove_const<typename traits::point_type<G>::type>::type type;
		};

		// Specialization for point: the point itself
		template <typename P>
		struct point_type<point_tag, P>
		{
			typedef P type;
		};

		// Specializations for linestring/linear ring, via boost::range
		template <typename R>
		struct point_type<linestring_tag, R>
		{
			typedef typename boost::range_value<R>::type type;
		};

		template <typename R>
		struct point_type<ring_tag, R>
		{
			typedef typename boost::range_value<R>::type type;
		};


		// Specialization for polygon: the point-type is the point-type of its rinsg
		template <typename P>
		struct point_type<polygon_tag, P>
		{
			typedef typename ring_type<polygon_tag, P>::type R;
			typedef typename point_type<ring_tag, R>::type type;
		};

	} // namespace core_dispatch
	#endif


	/*!
		\brief Meta-function which defines point type of any geometry
		\ingroup core
	*/
	template <typename G>
	struct point_type
	{
		typedef typename boost::remove_const<G>::type NCG;
		typedef typename core_dispatch::point_type<typename tag<G>::type, NCG>::type type;
	};


}


#endif
