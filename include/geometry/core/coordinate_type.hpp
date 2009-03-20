// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_COORDINATE_TYPE_HPP
#define _GEOMETRY_COORDINATE_TYPE_HPP


#include <boost/type_traits/remove_const.hpp>
#include <geometry/core/point_type.hpp>

namespace geometry
{

	namespace traits
	{

		/*!
			\brief Traits class which indicate the coordinate type (double,float,...) of a point
			\ingroup traits
			\par Geometries:
				- point
			\par Specializations should provide:
				- typedef T type; (double,float,int,etc)
		*/
		template <typename P>
		struct coordinate_type {};


	} // namespace traits




	#ifndef DOXYGEN_NO_DISPATCH
	namespace core_dispatch
	{
		template <typename TAG, typename G>
		struct coordinate_type
		{
			typedef typename point_type<TAG, G>::type P;

			// Call its own specialization on point-tag
			typedef typename coordinate_type<point_tag, P>::type type;
		};


		template <typename P>
		struct coordinate_type<point_tag, P>
		{
			typedef typename traits::coordinate_type<P>::type type;
		};

	} // namespace core_dispatch
	#endif

	/*!
		\brief Meta-function which defines coordinate type (int, float, double, etc) of any geometry
		\ingroup core
	*/
	template <typename G>
	struct coordinate_type
	{
		typedef typename boost::remove_const<G>::type NCG;
		typedef typename core_dispatch::coordinate_type<typename tag<G>::type, NCG>::type type;
	};

}


#endif
