// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_CLEAR_HPP
#define _GEOMETRY_CLEAR_HPP


#include <geometry/core/access.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>



namespace geometry
{

	// This traits is currently NOT defined in ../core/ but here, just because it default
	// does not have to be implemented
	namespace traits
	{

		/*!
			\brief Traits class, optional, might be implemented to clear a geometry
			\details If a geometry type should not use the std ".clear()" then it can specialize
			the "use_std" traits class to false, it should then implement (a.o.) clear
			\ingroup traits
			\par Geometries:
				- linestring
				- linear_ring
			\par Specializations should provide:
				- run
		 */
		template <typename G>
		struct clear
		{
		};
	}


	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		template <typename G>
		struct use_std_clear
		{
			static inline void run(G& geometry)
			{
				geometry.clear();
			}
		};

		template <typename G>
		struct use_traits_clear
		{
			static inline void run(G& geometry)
			{
				traits::clear<G>::run(geometry);
			}
		};


		template <typename P>
		struct polygon_clear
		{
			static inline void run(P& polygon)
			{
				interior_rings(polygon).clear();
				exterior_ring(polygon).clear();
			}
		};


	} // namespace impl
	#endif

	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, bool STD, typename G>
		struct clear {};


		// True (default for all geometry types, unless otherwise implemented in traits)
		// uses std::clear
		template <typename TAG, typename G>
		struct clear<TAG, true, G> : impl::use_std_clear<G> {};


		// If any geometry specializes use_std<G> to false, specialize to use the traits clear.
		template <typename TAG, typename G>
		struct clear<TAG, false, G> : impl::use_traits_clear<G> {};


		// Point/box/nsphere/segment do not have clear. So specialize to do nothing.
		template <typename G> struct clear<point_tag, true, G>  {};
		template <typename G> struct clear<box_tag, true, G>  {};
		template <typename G> struct clear<segment_tag, true, G>  {};
		template <typename G> struct clear<nsphere_tag, true, G>  {};


		// Polygon can (indirectly) use std for clear
		template <typename P>
		struct clear<polygon_tag, true, P> : impl::polygon_clear<P> {};


	} // namespace dispatch
	#endif



	/*!
		\brief Clears a linestring, linear ring or polygon (exterior+interiors) or multi*
		\details Generic function to clear a geometry
		\ingroup access
		\note points and boxes cannot be cleared, instead they can be set to zero by "assign_zero"
	*/
	template <typename G>
	inline void clear(G& geometry)
	{
		typedef typename boost::remove_const<G>::type NCG;
		dispatch::clear<typename tag<G>::type, traits::use_std<NCG>::value, NCG>::run(geometry);
	}

}


#endif
