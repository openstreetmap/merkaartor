// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_APPEND_HPP
#define _GEOMETRY_APPEND_HPP

#include <boost/type_traits/remove_const.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/access.hpp>
#include <geometry/core/point_type.hpp>
#include <geometry/core/tags.hpp>



namespace geometry
{

	namespace traits
	{

		/*!
			\brief Traits class, optional, might be implemented to append a point
			\details If a geometry type should not use the std "push_back" then it can specialize
			the "use_std" traits class to false, it should then implement (a.o.) append_point
			\ingroup traits
			\par Geometries:
				- linestring
				- linear_ring
			\par Specializations should provide:
				- run
		 */
		template <typename G, typename P>
		struct append_point
		{
		};
	}


	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace append
		{
			template <typename G, typename P, bool STD>
			struct append_point {};

			template <typename G, typename P>
			struct append_point<G, P, true>
			{
				static inline void run(G& geometry, const P& point, int , int )
				{
					typename point_type<G>::type point_to_add;
					copy_coordinates(point, point_to_add);
					geometry.push_back(point_to_add);
				}
			};

			template <typename G, typename P>
			struct append_point<G, P, false>
			{
				static inline void run(G& geometry, const P& point, int ring_index, int multi_index)
				{
					traits::append_point<G, P>::run(geometry, point, ring_index, multi_index);
				}
			};


			template <typename G, typename R, bool STD>
			struct append_range
			{
				typedef typename boost::range_value<R>::type P;

				static inline void run(G& geometry, const R& range, int ring_index, int multi_index)
				{
					for (typename boost::range_const_iterator<R>::type it = boost::begin(range);
							it != boost::end(range); it++)
					{
						append_point<G, P, STD>::run(geometry, *it, ring_index, multi_index);
					}
				}
			};


			template <typename P, typename PNT, bool STD>
			struct point_to_poly
			{
				typedef typename ring_type<P>::type R;

				static inline void run(P& polygon, const PNT& point, int ring_index, int /* multi_index */)
				{
					if (ring_index == -1)
					{
						append_point<R, PNT, STD>::run(exterior_ring(polygon), point, -1, -1);
					}
					else if (ring_index < boost::size(interior_rings(polygon)))
					{
						append_point<R, PNT, STD>::run(interior_rings(polygon)[ring_index], point, -1, -1);
					}
				}
			};

			template <typename P, typename R, bool STD>
			struct range_to_poly
			{
				typedef typename ring_type<P>::type RING;

				static inline void run(P& polygon, const R& range, int ring_index, int multi_index)
				{
					if (ring_index == -1)
					{
						append_range<RING, R, STD>::run(exterior_ring(polygon), range, -1, -1);
					}
					else if (ring_index < boost::size(interior_rings(polygon)))
					{
						append_range<RING, R, STD>::run(interior_rings(polygon)[ring_index], range, -1, -1);
					}
				}
			};


		} // namespace append

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		// (ROP = range or point, STD = use std library)

		// Default case (where ROP will be range/array/etc)
		template <typename TAG_G, typename TAG_ROP, typename G, typename ROP, bool STD>
		struct append
				: impl::append::append_range<G, ROP, STD> {};

		// Append a point to any geometry
		template <typename TAG, typename G, typename P, bool STD>
		struct append<TAG, point_tag, G, P, STD>
				: impl::append::append_point<G, P, STD> {};

		// Never possible to append anything to a point/box/n-sphere
		template <typename TAG_ROP, typename P, typename ROP, bool STD>
		struct append<point_tag, TAG_ROP, P, ROP, STD>  {};

		template <typename TAG_ROP, typename B, typename ROP, bool STD>
		struct append<box_tag, TAG_ROP, B, ROP, STD>  {};

		template <typename TAG_ROP, typename N, typename ROP, bool STD>
		struct append<nsphere_tag, TAG_ROP, N, ROP, STD>  {};

		template <typename P, typename TAG_R, typename R, bool STD>
		struct append<polygon_tag, TAG_R, P, R, STD>
				: impl::append::range_to_poly<P, R, STD> {};

		template <typename P, typename PNT, bool STD>
		struct append<polygon_tag, point_tag, P, PNT, STD>
				: impl::append::point_to_poly<P, PNT, STD> {};

		// Multi-linestring and multi-polygon might either implement traits or use standard...

	} // namespace dispatch
	#endif




	/*!
		\brief Appends one or more points to a linestring, linear-ring, polygon, multi
		\ingroup access
		\param geometry a geometry
		\param range_or_point the point or range to add
		\param ring_index the index of the ring in case of a polygon: exterior ring (-1, the default) or
			interior ring index
		\param multi_index reserved for multi polygons
	 */
	template <typename G, typename ROP>
	inline void append(G& geometry, const ROP& range_or_point,
				int ring_index = -1, int multi_index = 0)
	{
		typedef typename boost::remove_const<G>::type NCG;
		dispatch::append
			<typename tag<G>::type, typename tag<ROP>::type,
			NCG, ROP, traits::use_std<NCG>::value>
					::run(geometry, range_or_point, ring_index, multi_index);
	}

}


#endif // _GEOMETRY_APPEND_HPP
