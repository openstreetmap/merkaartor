// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_NUM_POINTS_HPP
#define _GEOMETRY_NUM_POINTS_HPP


#include <boost/type_traits/remove_const.hpp>


namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		template <typename R>
		struct range_number_of_points
		{
			static inline size_t get(const R& range)
			{
				return boost::size(range);
			}
		};

		template <typename G, size_t D>
		struct other_number_of_points
		{
			static inline size_t get(const G& geometry)
			{
				return D;
			}
		};


		template <typename P>
		struct polygon_number_of_points
		{
			static inline size_t get(const P& poly)
			{
				size_t n = boost::size(exterior_ring(poly));
				typedef typename boost::range_const_iterator<typename interior_type<P>::type>::type IT;
				for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
				{
					n += boost::size(*it);
				}

				return n;
			}
		};



	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G>
		struct num_points {};

		template <typename G>
		struct num_points<point_tag, G> : impl::other_number_of_points<G, 1> {};

		template <typename G>
		struct num_points<box_tag, G> : impl::other_number_of_points<G, 4> {};

		template <typename G>
		struct num_points<segment_tag, G> : impl::other_number_of_points<G, 2> {};

		template <typename G>
		struct num_points<nsphere_tag, G> : impl::other_number_of_points<G, 1> {};

		template <typename G>
		struct num_points<linestring_tag, G> : impl::range_number_of_points<G> {};

		template <typename G>
		struct num_points<ring_tag, G> : impl::range_number_of_points<G> {};

		template <typename G>
		struct num_points<polygon_tag, G> : impl::polygon_number_of_points<G> {};

	} // namespace dispatch
	#endif



	/*!
		\brief get number of points
		\ingroup access
		\tparam G geometry type
		\param geometry the geometry to get number of points from
		\return number of points
		\note For linestrings/rings also boost::size or .size() could be used, however,
			for polygons this is less obvious. So this function is provided. Besides that
			it is described by OGC (numPoints)
	*/
	template <typename G>
	inline size_t num_points(G& geometry)
	{
		typedef typename boost::remove_const<G>::type NCG;
		return dispatch::num_points<typename tag<G>::type, NCG>::get(geometry);
	}

}


#endif
