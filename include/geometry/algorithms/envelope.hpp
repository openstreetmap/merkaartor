// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_ENVELOPE_HPP
#define _GEOMETRY_ENVELOPE_HPP

#include <boost/concept/requires.hpp>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/numeric/conversion/cast.hpp>


#include <geometry/core/cs.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/box_concept.hpp>
#include <geometry/core/concepts/nsphere_concept.hpp>
#include <geometry/core/concepts/linestring_concept.hpp>
#include <geometry/core/concepts/ring_concept.hpp>
#include <geometry/core/concepts/polygon_concept.hpp>

#include <geometry/core/exterior_ring.hpp>

#include <geometry/algorithms/convert.hpp>



#include <geometry/strategies/strategies.hpp>


/*!
\defgroup envelope envelope calculation
\par Source descriptions:
- OGC: Envelope (): Geometry - The minimum bounding box for this Geometry,
	returned as a Geometry. The polygon is defined by the corner points of the bounding
	box [(MINX, MINY), (MAXX, MINY), (MAXX, MAXY), (MINX, MAXY), (MINX, MINY)].

\note Implemented in the Geometry library: The minimum bounding box, always as a box, having min <= max

The envelope algorithm calculates the bounding box, or envelope, of a geometry. There are two versions:
- envelope, taking a reference to a box as second parameter
- make_envelope, returning a newly constructed box (type as a template parameter in the function call)
- either of them has an optional strategy

\par Geometries:
- POINT: a box with zero area, the maximum and the minimum point of the box are
set to the point itself.
- LINESTRING, RING or RANGE is the smallest box that contains all points of the specified
point sequence.
If the linestring is empty, the envelope is the inverse infinite box, that is, the minimum point is very
large (max infinite) and the maximum point is very small (min infinite).
- POLYGON, the envelope of the outer ring
	\image html envelope_polygon.png

\par Example:
Example showing envelope calculation
\dontinclude doxygen_examples.cpp
\skip example_envelope_linestring
\line {
\until }


*/

namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		namespace envelope
		{

			/// Calculate envelope of an n-sphere, circle or sphere (currently only for Cartesian 2D points)
			template<typename B, typename S>
			inline void envelope_nsphere(const S& s, B& mbr)
			{
				assert_dimension<S, 2>;
				assert_dimension<B, 2>;

				typename radius_type<S>::type r = get_radius<0>(s);
				set<min_corner, 0>(mbr, get<0>(s) - r);
				set<min_corner, 1>(mbr, get<1>(s) - r);
				set<max_corner, 0>(mbr, get<0>(s) + r);
				set<max_corner, 1>(mbr, get<1>(s) + r);
			}

			/// Calculate envelope of an 2D or 3D point
			template<typename B, typename P>
			inline void envelope_point(const P& p, B& b)
			{
				// Envelope of a point is an empty box, a box with zero volume, located at the point.
				// We just user the convert algorithm here
				geometry::convert(p, b);
			}

			/// Version with state iterating through range (also used in multi*)
			template<typename R, typename S>
			inline void envelope_range_state(const R& range, const S& strategy, typename S::state_type& state)
			{
				typedef typename boost::range_const_iterator<R>::type IT;
				for (IT it = boost::begin(range); it != boost::end(range); it++)
				{
					strategy(*it, state);
				}
			}


			/// Version with strategy, calling state
			template<typename B, typename R, typename S>
			inline void envelope_range_strategy(const R& range, B& mbr, const S& strategy)
			{
				typename S::state_type state(mbr);
				envelope_range_state(range, strategy, state);
			}


			/// Generic range dispatching struct
			template <typename R, typename B>
			struct envelope_range
			{
				/// Calculate envelope of range using a strategy
				template<typename S>
				static inline void calculate(const R& range, B& b, const S& strategy)
				{
					envelope_range_strategy(range, b, strategy);
				}


				/// Calculate envelope of range
				static inline void calculate(const R& range, B& b)
				{
					typedef typename point_type<B>::type PB;
					typedef typename point_type<R>::type PR;
					typename strategy_envelope<
								typename cs_tag<PR>::type,
								typename cs_tag<PB>::type, PR, B>::type strategy;

					calculate (range, b, strategy);
				}
			};



		} // namespace envelope
	} // namespace impl
	#endif


	#ifndef DOXYGEN_NO_DISPATCH
	namespace dispatch
	{

		template <typename TAG1, typename TAG2, typename G, typename B>
		struct envelope {};

		template <typename P, typename B>
		struct envelope<point_tag, box_tag, P, B>
		{
			/*!
				\brief Calculate envelope of a point
				\details The envelope of a point is a box containing just the point itself. It is provided
				for consistence, on itself it is not useful.
			 */
			static inline void calculate(const P& p, B& b)
			{
				impl::envelope::envelope_point(p, b);
			}
			private :
				BOOST_CONCEPT_ASSERT((ConstPoint<P>));
				BOOST_CONCEPT_ASSERT((Box<B>));
		};


		template <typename S, typename B>
		struct envelope<nsphere_tag, box_tag, S, B>
		{

			/// Calculate envelope of a n-sphere
			static inline void calculate(const S& n, B& b)
			{
				impl::envelope::envelope_nsphere(n, b);
			}
			private :
				BOOST_CONCEPT_ASSERT((ConstNsphere<S>));
				BOOST_CONCEPT_ASSERT((Box<B>));
		};



		template <typename L, typename B>
		struct envelope<linestring_tag, box_tag, L, B> : impl::envelope::envelope_range<L, B>
		{
			private :
				BOOST_CONCEPT_ASSERT((ConstLinestring<L>));
				BOOST_CONCEPT_ASSERT((Box<B>));
		};


		template <typename R, typename B>
		struct envelope<ring_tag, box_tag, R, B> : impl::envelope::envelope_range<R, B>
		{
			private :
				BOOST_CONCEPT_ASSERT((ConstRing<R>));
				BOOST_CONCEPT_ASSERT((Box<B>));
		};



		template <typename P, typename B>
		struct envelope<polygon_tag, box_tag, P, B>
		{

			template<typename S>
			static inline void calculate(const P& poly, B& b, const S& strategy)
			{
				// For polygon inspecting outer linear_ring is sufficient
				impl::envelope::envelope_range_strategy(exterior_ring(poly), b, strategy);
			}

			static inline void calculate(const P& poly, B& b)
			{
				typedef typename point_type<B>::type PB;
				typedef typename point_type<P>::type PP;
				typename strategy_envelope<typename cs_tag<PP>::type,
							typename cs_tag<PB>::type, PP, B>::type strategy;

				calculate(poly, b, strategy);
			}

			private :
				BOOST_CONCEPT_ASSERT((ConstPolygon<P>));
				BOOST_CONCEPT_ASSERT((Box<B>));
		};

	} // namespace dispatch
	#endif



	/*!
		\brief Calculate envelope of a geometry
		\ingroup envelope
		\param geometry the geometry
		\param box the box receiving the envelope
		\par Example:
		Example showing envelope calculation, using point_ll latlong points
		\dontinclude doxygen_examples.cpp
		\skip example_envelope_polygon
		\line {
		\until }
	 */
	template<typename G, typename B>
	inline void envelope(const G& geometry, B& box)
	{
		dispatch::envelope<typename tag<G>::type,
					typename tag<B>::type, G, B>::calculate(geometry, box);
	}


	/*!
		\brief Calculate envelope of a geometry, using a specified strategy
		\ingroup envelope
		\param geometry the geometry
		\param box the box receiving the envelope
		\param strategy strategy to be used
	*/
	template<typename G, typename B, typename S>
	inline void envelope(const G& geometry, B& box, const S& strategy)
	{
		dispatch::envelope<typename tag<G>::type,
					typename tag<B>::type, G, B>::calculate(geometry, box, strategy);
	}



	/*!
		\brief Calculate and return envelope of a geometry
		\ingroup envelope
		\param geometry the geometry
	 */
	template<typename B, typename G>
	inline B make_envelope(const G& geometry)
	{
		B box;
		dispatch::envelope<typename tag<G>::type,
					typename tag<B>::type, G, B>::calculate(geometry, box);
		return box;
	}

	/*!
		\brief Calculate and return envelope of a geometry
		\ingroup envelope
		\param geometry the geometry
		\param strategy the strategy to be used
	 */
	template<typename B, typename G, typename S>
	inline B make_envelope(const G& geometry, const S& strategy)
	{
		B box;
		dispatch::envelope<typename tag<G>::type,
					typename tag<B>::type, G, B>::calculate(geometry, box, strategy);
		return box;
	}


} // namespace geometry


#endif // _GEOMETRY_ENVELOPE_HPP
