// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_CONVERT_HPP
#define _GEOMETRY_CONVERT_HPP

#include <cmath>
#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/cs.hpp>

#include <geometry/geometries/segment.hpp>

#include <geometry/strategies/strategies.hpp>

#include <geometry/algorithms/append.hpp>
#include <geometry/algorithms/foreach.hpp>


/*!
\defgroup convert convert geometries from one type to another
\details Convert from one geometry type to another type, for example from BOX to POLYGON
*/

namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace convert
		{

			template <typename P, typename B, size_t C, size_t D, size_t N>
			struct point_to_box
			{
				static inline void loop(const P& point, B& box)
				{
					typedef typename coordinate_type<B>::type T;
					set<C, D>(box, boost::numeric_cast<T>(get<D>(point)));
					point_to_box<P, B, C, D + 1, N>::loop(point, box);
				}
			};

			template <typename P, typename B, size_t C, size_t N>
			struct point_to_box<P, B, C, N, N>
			{
				static inline void loop(const P& point, B& box)
				{}
			};


		} // namespace convert

	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G1, typename G2>
		struct convert
		{
		};

		template <typename TAG, typename G1, typename G2>
		struct convert<TAG, TAG, G1, G2>
		{
			// Same geometry type -> copy coordinates from G1 to G2
		};

		template <typename TAG, typename G>
		struct convert<TAG, TAG, G, G>
		{
			// Same geometry -> can be copied
		};


		// Partial specializations
		template <typename B, typename R>
		struct convert<box_tag, ring_tag, B, R>
		{
			inline static void calculate(const B& box, R& ring)
			{
				// go from box to ring -> add coordinates in correct order
				// only valid for 2D
				assert_dimension<B, 2>();

				ring.clear();
				typename point_type<B>::type p;

				assign(p, get<min_corner, 0>(box), get<min_corner, 1>(box));
				geometry::append(ring, p);

				assign(p, get<min_corner, 0>(box), get<max_corner, 1>(box));
				geometry::append(ring, p);

				assign(p, get<max_corner, 0>(box), get<max_corner, 1>(box));
				geometry::append(ring, p);

				assign(p, get<max_corner, 0>(box), get<min_corner, 1>(box));
				geometry::append(ring, p);

				assign(p, get<min_corner, 0>(box), get<min_corner, 1>(box));
				geometry::append(ring, p);
			}
		};

		template <typename P, typename B>
		struct convert<point_tag, box_tag, P, B>
		{
			inline static void calculate(const P& point, B& box)
			{
				// go from point to box -> box with volume of zero, 2D or 3D
				static const size_t N = dimension<P>::value;
				impl::convert::point_to_box<P, B, min_corner, 0, N>::loop(point, box);
				impl::convert::point_to_box<P, B, max_corner, 0, N>::loop(point, box);
			}
		};


	} // namespace dispatch
	#endif





	/*!
		\brief Converts one geometry to another geometry
		\details The convert algorithm converts one geometry, e.g. a BOX, to another geometry, e.g. a RING. This only
		if it is possible and applicable.
		\ingroup convert
		\tparam G1 first geometry type
		\tparam G2 second geometry type
		\param geometry1 first geometry
		\param geometry2 second geometry
	 */
	template <typename G1, typename G2>
	inline void convert(const G1& geometry1, G2& geometry2)
	{
		dispatch::convert<typename tag<G1>::type,
				typename tag<G2>::type, G1, G2>::calculate(geometry1, geometry2);
	}

} // namespace geometry


#endif // _GEOMETRY_CONVERT_HPP
