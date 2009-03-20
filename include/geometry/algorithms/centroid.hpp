// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_CENTROID_HPP
#define _GEOMETRY_CENTROID_HPP

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <geometry/core/cs.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>


#include <geometry/core/concepts/point_concept.hpp>

#include <geometry/util/copy.hpp>
#include <geometry/util/loop.hpp>

#include <geometry/strategies/strategies.hpp>

/*!
\defgroup centroid centroid calculation
\par Source descriptions:
- OGC description: The mathematical centroid for this Surface as a Point. The result is not guaranteed to be on this Surface.
- From Wikipedia: Informally, it is the "average" of all points
\see http://en.wikipedia.org/wiki/Centroid
\note The "centroid" functions are taking a non const reference to the centroid. The "make_centroid" functions
  return the centroid, the type has to be specified.

\note There are versions where the centroid calculation strategy can be specified
\par Geometries:
- RING: \image html centroid_ring.png
- BOX: the centroid of a 2D or 3D box is the center of the box
- CIRCLE: the centroid of a circle or a sphere is its center
- POLYGON \image html centroid_polygon.png
- POINT, LINESTRING, SEGMENT: trying to calculate the centroid will result in a compilation error
*/

namespace geometry
{
	// Move elsewhere?
    class centroid_exception : public std::exception
    {
		public:
			centroid_exception()  {}
			virtual const char *what() const throw()
			{
				return "centroid calculation exception";
			}
    };

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{


		namespace centroid
		{

			/*!
				\brief Generic function which checks if enough points are present
			*/
			template<typename P, typename R>
			inline bool ring_ok(const R& ring, P& c)
			{
				size_t n = boost::size(ring);
				if (n == 1)
				{
					// Take over the first point in a "coordinate neutral way"
					copy_coordinates(ring.front(), c);
					return false;
				}
				else if (n <= 0)
				{
					throw centroid_exception();
				}
				return true;
			}


			/*!
				\brief Calculate the centroid of a ring.
			*/
			template<typename P, typename R, typename S>
			inline void centroid_ring(const R& ring, P& c, const S& strategy)
			{
				if (ring_ok(ring, c))
				{
					typename S::state_type state;
					loop(ring, strategy, state);
					state.centroid(c);
				}
			}

			/*!
				\brief Centroid of a polygon.
				\note Because outer ring is clockwise, inners are counter clockwise,
				triangle approach is OK and works for polygons with rings.
			*/
			template<typename P, typename Y, typename S>
			inline void centroid_polygon(const Y& poly, P& c, const S& strategy)
			{
				if (ring_ok(exterior_ring(poly), c))
				{
					typename S::state_type state;
					loop(exterior_ring(poly), strategy, state);
					typedef typename boost::range_const_iterator<typename interior_type<Y>::type>::type IT;
					for (IT it = boost::begin(interior_rings(poly)); it != boost::end(interior_rings(poly)); it++)
					{
						loop(*it, strategy, state);
					}
					state.centroid(c);
				}
			}


			/*!
				\brief Calculate centroid (==center) of a box
				\todo Implement strategy
			*/
			template<typename P, typename B>
			inline void centroid_box(const B& box, P& c)
			{
				// TODO: adapt using strategies
				assert_dimension<B, 2>();
				set<0>(c, (get<min_corner, 0>(box) + get<max_corner, 0>(box)) / 2);
				set<1>(c, (get<min_corner, 1>(box) + get<max_corner, 1>(box)) / 2);
			}


		} // namespace centroid
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{
		template <typename TAG, typename G, typename P>
		struct centroid {};


		template <typename B, typename P>
		struct centroid<box_tag, B, P>
		{
			inline static void calculate(const B& b, P& c)
			{
				impl::centroid::centroid_box<P>(b, c);
			}
		};



		template <typename R, typename P>
		struct centroid<ring_tag, R, P>
		{

			inline static void calculate(const R& ring, P& c)
			{
				impl::centroid::centroid_ring(ring, c,
					typename strategy_centroid<typename cs_tag<P>::type,
								P, typename boost::range_value<R>::type>::type());
			}
		};

		template <typename Y, typename P>
		struct centroid<polygon_tag, Y, P>
		{
			inline static void calculate(const Y& poly, P& c)
			{
				impl::centroid::centroid_polygon(poly, c,
					typename strategy_centroid<typename cs_tag<P>::type,
						P, typename point_type<Y>::type>::type());
			}


			template<typename S>
			inline static void calculate(const Y& poly, P& c, const S& strategy)
			{
				impl::centroid::centroid_polygon(poly, c, strategy);
			}
		};

	} // namespace dispatch
	#endif



	/*!
		\brief Calculate centroid
		\ingroup centroid
		\details The function centroid calculates the centroid of a geometry using the default strategy.
		A polygon should be closed and orientated clockwise, holes, if any, must be orientated
		counter clockwise
		\param geometry a geometry (e.g. closed ring or polygon)
		\param c reference to point which will contain the centroid
		\exception centroid_exception if calculation is not successful, e.g. because polygon didn't contain points
		\par Example:
		Example showing centroid calculation
		\dontinclude doxygen_examples.cpp
		\skip example_centroid_polygon
		\line {
		\until }
	 */
	template<typename G, typename P>
	inline void centroid(const G& geometry, P& c)
	{
		dispatch::centroid<typename tag<G>::type, G, P>::calculate(geometry, c);
	}

	/*!
		\brief Calculate centroid using a specified strategy
		\ingroup centroid
		\param geometry the geometry to calculate centroid from
		\param c reference to point which will contain the centroid
		\param strategy Calculation strategy for centroid
		\exception centroid_exception if calculation is not successful, e.g. because polygon didn't contain points
	 */
	template<typename G, typename P, typename S>
	inline void centroid(const G& geometry, P& c, const S& strategy)
	{
		dispatch::centroid<typename tag<G>::type, G, P>::calculate(geometry, c, strategy);
	}


	// Versions returning a centroid

	/*!
		\brief Calculate and return centroid
		\ingroup centroid
		\param geometry the geometry to calculate centroid from
		\return the centroid
		\exception centroid_exception if calculation is not successful, e.g. because polygon didn't contain points
	 */
	template<typename P, typename G>
	inline P make_centroid(const G& geometry)
	{
		P c;
		dispatch::centroid<typename tag<G>::type, G, P>::calculate(geometry, c);
		return c;
	}

	/*!
		\brief Calculate and return centroid
		\ingroup centroid
		\param geometry the geometry to calculate centroid from
		\param strategy Calculation strategy for centroid
		\return the centroid
		\exception centroid_exception if calculation is not successful, e.g. because polygon didn't contain points
	 */
	template<typename P, typename G, typename S>
	inline P make_centroid(const G& geometry, const S& strategy)
	{
		P c;
		dispatch::centroid<typename tag<G>::type, G, P>::calculate(geometry, c, strategy);
		return c;
	}


} // namespace geometry


#endif // _GEOMETRY_CENTROID_HPP
