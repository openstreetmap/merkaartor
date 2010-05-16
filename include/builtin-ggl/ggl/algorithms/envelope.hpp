// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_ENVELOPE_HPP
#define GGL_ALGORITHMS_ENVELOPE_HPP

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <ggl/algorithms/combine.hpp>
#include <ggl/algorithms/convert.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/concepts/box_concept.hpp>
#include <ggl/core/concepts/linestring_concept.hpp>
#include <ggl/core/concepts/nsphere_concept.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/concepts/polygon_concept.hpp>
#include <ggl/core/concepts/ring_concept.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup envelope envelope calculation
\par Source descriptions:
- OGC: Envelope (): Geometry - The minimum bounding rectangle (MBR) for this Geometry,
returned as a Geometry. The polygon is defined by the corner points of the bounding
box [(MINX, MINY), (MAXX, MINY), (MAXX, MAXY), (MINX, MAXY), (MINX, MINY)].

\note Implemented in the Generic Geometry Library: The minimum bounding box, always as a box, having min <= max

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

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace envelope {

/// Calculate envelope of an n-sphere, circle or sphere (currently only for Cartesian 2D points)
template<typename B, typename S, typename Strategy>
struct envelope_nsphere
{
    static inline void apply(S const& s, B& mbr, Strategy const&)
    {
        assert_dimension<S, 2>();
        assert_dimension<B, 2>();

        typename radius_type<S>::type r = get_radius<0>(s);
        set<min_corner, 0>(mbr, get<0>(s) - r);
        set<min_corner, 1>(mbr, get<1>(s) - r);
        set<max_corner, 0>(mbr, get<0>(s) + r);
        set<max_corner, 1>(mbr, get<1>(s) + r);
    }
};


/// Calculate envelope of an 2D or 3D point
template<typename P, typename B, typename Strategy>
struct envelope_point
{
    static inline void apply(P const& p, B& mbr, Strategy const&)
    {
        // Envelope of a point is an empty box, a box with zero volume, located at the point.
        // We just use the convert algorithm here
        ggl::convert(p, mbr);
    }
};


/// Calculate envelope of an 2D or 3D segment
template<typename S, typename B, typename Strategy>
struct envelope_segment
{
    static inline void apply(S const& s, B& mbr, Strategy const&)
    {
        ggl::assign_inverse(mbr);
        ggl::combine(mbr, s.first);
        ggl::combine(mbr, s.second);
    }
};


/// Version with state iterating through range (also used in multi*)
template<typename R, typename Strategy>
inline void envelope_range_state(R const& range, Strategy const& strategy, typename Strategy::state_type& state)
{
    typedef typename boost::range_const_iterator<R>::type iterator_type;

    for (iterator_type it = boost::begin(range); it != boost::end(range); it++)
    {
        strategy(*it, state);
    }
}



/// Generic range dispatching struct
template <typename R, typename B, typename Strategy>
struct envelope_range
{
    /// Calculate envelope of range using a strategy
    static inline void apply(R const& range, B& mbr, Strategy const& strategy)
    {
        typename Strategy::state_type state(mbr);
        envelope_range_state(range, strategy, state);
    }
};

}} // namespace detail::envelope
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1, typename Tag2,
    typename Geometry, typename Box,
    typename Strategy
>
struct envelope {};


template <typename P, typename B, typename Strategy>
struct envelope<point_tag, box_tag, P, B, Strategy>
    : detail::envelope::envelope_point<P, B, Strategy>
{
private:
    BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P>) );
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};


template <typename B, typename Strategy>
struct envelope<box_tag, box_tag, B, B, Strategy>
{
    /*!
        \brief Calculate envelope of a box
        \details The envelope of a box is itself, provided for consistency
        for consistency, on itself it is not useful.
     */
    static inline void apply(B const& b, B& mbr, Strategy const&)
    {
        mbr = b;
    }

private:
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};


template <typename S, typename B, typename Strategy>
struct envelope<segment_tag, box_tag, S, B, Strategy>
    : detail::envelope::envelope_segment<S, B, Strategy>
{};


template <typename S, typename B, typename Strategy>
struct envelope<nsphere_tag, box_tag, S, B, Strategy>
    : detail::envelope::envelope_nsphere<S, B, Strategy>
{
private:
    BOOST_CONCEPT_ASSERT( (concept::ConstNsphere<S>) );
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};

template <typename L, typename B, typename Strategy>
struct envelope<linestring_tag, box_tag, L, B, Strategy>
    : detail::envelope::envelope_range<L, B, Strategy>
{
private:
    BOOST_CONCEPT_ASSERT( (concept::ConstLinestring<L>) );
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};


template <typename R, typename B, typename Strategy>
struct envelope<ring_tag, box_tag, R, B, Strategy>
    : detail::envelope::envelope_range<R, B, Strategy>
{
private:
    BOOST_CONCEPT_ASSERT( (concept::ConstRing<R>) );
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};


template <typename P, typename B, typename Strategy>
struct envelope<polygon_tag, box_tag, P, B, Strategy>
{
    static inline void apply(P const& poly, B& mbr, Strategy const& strategy)
    {
        // For polygon inspecting outer linear_ring is sufficient

        detail::envelope::envelope_range
            <
                typename ring_type<P>::type,
                B,
                Strategy
            >::apply(exterior_ring(poly), mbr, strategy);
    }

private:
    BOOST_CONCEPT_ASSERT( (concept::ConstPolygon<P>) );
    BOOST_CONCEPT_ASSERT( (concept::Box<B>) );
};


} // namespace dispatch
#endif


/*!
\brief Calculate envelope of a geometry, using a specified strategy
\ingroup envelope
\param geometry the geometry
\param mbr the box receiving the envelope
\param strategy strategy to be used
*/
template<typename G, typename B, typename S>
inline void envelope(G const& geometry, B& mbr, S const& strategy)
{
    dispatch::envelope
        <
            typename tag<G>::type, typename tag<B>::type,
            G, B, S
        >::apply(geometry, mbr, strategy);
}




/*!
\brief Calculate envelope of a geometry
\ingroup envelope
\param geometry the geometry
\param mbr the box receiving the envelope
\par Example:
Example showing envelope calculation, using point_ll latlong points
\dontinclude doxygen_examples.cpp
\skip example_envelope_polygon
\line {
\until }
*/
template<typename G, typename B>
inline void envelope(G const& geometry, B& mbr)
{
    typename strategy_envelope
        <
        typename cs_tag<typename point_type<G>::type>::type,
        typename cs_tag<typename point_type<B>::type>::type,
        typename point_type<G>::type,
        B
        >::type strategy;

    envelope(geometry, mbr, strategy);
}


/*!
\brief Calculate and return envelope of a geometry
\ingroup envelope
\param geometry the geometry
\param strategy the strategy to be used
*/
template<typename B, typename G, typename S>
inline B make_envelope(G const& geometry, S const& strategy)
{
    B box;
    dispatch::envelope
        <
            typename tag<G>::type, typename tag<B>::type,
            G, B, S
        >::apply(geometry, box, strategy);

    return box;
}


/*!
\brief Calculate and return envelope of a geometry
\ingroup envelope
\param geometry the geometry
*/
template<typename B, typename G>
inline B make_envelope(G const& geometry)
{
    typename strategy_envelope
        <
        typename cs_tag<typename point_type<G>::type>::type,
        typename cs_tag<typename point_type<B>::type>::type,
        typename point_type<G>::type,
        B
        >::type strategy;
    return make_envelope<B>(geometry, strategy);
}


} // namespace ggl

#endif // GGL_ALGORITHMS_ENVELOPE_HPP
