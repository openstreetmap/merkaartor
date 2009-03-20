// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_CONVEX_HULL_HPP
#define _GEOMETRY_CONVEX_HULL_HPP

#include <boost/concept/requires.hpp>


#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/cs.hpp>

#include <geometry/strategies/strategies.hpp>


/*!
\defgroup convex_hull convex hull calculation
\par Source descriptions:
- OGC description: Returns a geometric object that represents the convex hull of this geometric
  object. Convex hulls, being dependent on straight lines, can be accurately represented in linear interpolations
  for any geometry restricted to linear interpolations.
\see http://en.wikipedia.org/wiki/Convex_hull

\par Performance
2776 counties of US are "hulled" in 0.52 seconds (other libraries: 2.8 seconds, 2.4 seconds, 3.4 seconds, 1.1 seconds)

\note The convex hull is always a ring, holes are not possible. Therefore it is modelled as an output iterator.
This gives the most flexibility, the user can decide what to do with it.
\par Geometries:
In the images below the convex hull is painted in red.
- POINT: will not compile
- POLYGON: will deliver a polygon without holes \image html convexhull_polygon_polygon.png
*/
namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace convex_hull
		{
			template <typename R, typename O_IT>
			struct convex_hull_range
			{
				static inline O_IT calculate(const R& range, O_IT out)
				{
					typedef typename point_type<R>::type P;
					typedef typename strategy_convex_hull<typename cs_tag<P>::type, P>::type strategy;
					strategy s(range);
					s.get(out);
					return out;
				}
			};
		}

	}
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG, typename G, typename O_IT>
		struct convex_hull {};


		template <typename L, typename O_IT>
		struct convex_hull<linestring_tag, L, O_IT> : impl::convex_hull::convex_hull_range<L, O_IT> {};


		template <typename R, typename O_IT>
		struct convex_hull<ring_tag, R, O_IT> : impl::convex_hull::convex_hull_range<R, O_IT> {};


		template <typename P, typename O_IT>
		struct convex_hull<polygon_tag, P, O_IT>
		{
			static inline O_IT calculate(const P& poly, O_IT out)
			{
				// Checking outer ring is sufficient for convex hull of polygon, holes are not relevant
				typedef typename ring_type<P>::type R;
				return impl::convex_hull::convex_hull_range<R, O_IT>::calculate(exterior_ring(poly), out);
			}
		};


	} // namespace dispatch
	#endif

	/*!
		\brief Calculate the convex hull of a geometry
		\ingroup convex_hull
		\param geometry the geometry to calculate convex hull from
		\param out an output iterator outputing points of the convex hull
		\return the output iterator
	 */
	template<typename G, typename O_IT>
	inline O_IT convex_hull(const G& geometry, O_IT out)
	{
		return dispatch::convex_hull<typename tag<G>::type, G, O_IT>::calculate(geometry, out);
	}


} // namespace geometry


#endif // _GEOMETRY_CONVEX_HULL_HPP
