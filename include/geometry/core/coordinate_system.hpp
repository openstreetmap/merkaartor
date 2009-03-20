// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_COORDINATE_SYSTEM_HPP
#define _GEOMETRY_COORDINATE_SYSTEM_HPP


#include <boost/type_traits/remove_const.hpp>
#include <geometry/core/point_type.hpp>


namespace geometry
{

	namespace traits
	{

		/*!
			\brief Traits class defining the coordinate system of a point, important for strategy selection
			\ingroup traits
			\par Geometries:
				- point
			\par Specializations should provide:
				- typedef CS type; (cs::cartesian, cs::spherical, etc)
		*/
		template <typename P>
		struct coordinate_system {};

	} // namespace traits




	#ifndef DOXYGEN_NO_DISPATCH
	namespace core_dispatch
	{
		template <typename TAG, typename G>
		struct coordinate_system
		{
			typedef typename point_type<TAG, G>::type P;

			// Call its own specialization on point-tag
			typedef typename coordinate_system<point_tag, P>::type type;
		};


		template <typename P>
		struct coordinate_system<point_tag, P>
		{
			typedef typename traits::coordinate_system<P>::type type;
		};


	} // namespace core_dispatch
	#endif


	/*!
		\brief Meta-function which defines coordinate system for any geometry
		\ingroup core
	*/
	template <typename G>
	struct coordinate_system
	{
		typedef typename boost::remove_const<G>::type NCG;
		typedef typename core_dispatch::coordinate_system<typename tag<G>::type, NCG>::type type;
	};

}


#endif
