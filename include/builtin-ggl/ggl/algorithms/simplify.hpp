// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_SIMPLIFY_HPP
#define GGL_ALGORITHMS_SIMPLIFY_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/distance.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/strategies/agnostic/agn_simplify.hpp>


/*!
\defgroup simplify simplification (generalization)
\par Source description:
- Wikipedia: given a 'curve' composed of line segments to find a curve not too dissimilar but that has fewer points

\see http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm

\par Performance
Performance is measured on simplification of a collection of rings, such that 10% of the points is kept.
- 2776 counties of US are simplified in 0.8 seconds (2.5 seconds or 11.5 seconds in 2 other libraries)
- 3918 zipcode areas of the Netherlands are simplified in 0.03 seconds (0.1 seconds or 0.45 seconds in 2 other libraries)


\par Geometries
- LINESTRING:
\image html simplify_linestring.png
- POLYGON: simplifying a valid simple polygon (which never intersects itself) might result in an invalid polygon,
where the simplified rings intersect themselves or one of the other outer or inner rings.
Efficient simplification of a ring/polygon is still an "Open Problem"
(http://maven.smith.edu/~orourke/TOPP/P24.html#Problem.24)

*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace simplify {

template<typename R, typename OutputIterator, typename S>
inline void simplify_range_strategy(R const& range, OutputIterator out, double max_distance, S const& strategy)
{
    if (boost::begin(range) == boost::end(range) || max_distance < 0)
    {
        std::copy(boost::begin(range), boost::end(range), out);
    }
    else
    {
        typename boost::range_const_iterator<R>::type it = boost::begin(range) + 1;
        if (it == boost::end(range) || it + 1 == boost::end(range))
        {
            std::copy(boost::begin(range), boost::end(range), out);
        }
        else
        {
            strategy.simplify(range, out, max_distance);
        }
    }
}

template<typename R, typename OutputIterator>
inline void simplify_range(R const& range, OutputIterator out, double max_distance)
{
    // Define default strategy
    typedef typename point_type<R>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance_segment
        <
            cs_tag,
            cs_tag,
            point_type,
            segment<const point_type>
        >::type strategy_type;

    strategy::simplify::douglas_peucker<R, OutputIterator, strategy_type> douglas;

    simplify_range_strategy(range, out, max_distance, douglas);
}

template<typename R, typename OutputIterator, typename S>
inline void simplify_ring(R const& r_in, OutputIterator out, double max_distance, S const& strategy)
{
    // Call do_container for a ring

    // The first/last point (the closing point of the ring) should maybe be excluded because it
    // lies on a line with second/one but last. Here it is never excluded.

    // Note also that, especially if max_distance is too large, the output ring might be self intersecting
    // while the input ring is not, although chances are low in normal polygons

    // Finally the inputring might have 4 points (=correct), the output < 4(=wrong)
    if (boost::size(r_in) <= 4 || max_distance < 0)
    {
        std::copy(boost::begin(r_in), boost::end(r_in), out);
    }
    else
    {
        simplify_range_strategy(r_in, out, max_distance, strategy);
    }
}

template<typename Y, typename S>
inline void simplify_polygon(Y const& poly_in, Y& poly_out, double max_distance, S const& strategy)
{
    typedef typename boost::range_iterator
        <typename interior_type<Y>::type>::type iterator_type;
    typedef typename boost::range_const_iterator
        <typename interior_type<Y>::type>::type const_iterator_type;

    poly_out.clear();

    // Note that if there are inner rings, and distance is too large, they might intersect with the
    // outer ring in the output, while it didn't in the input.
    simplify_ring(exterior_ring(poly_in), std::back_inserter(exterior_ring(poly_out)),
                  max_distance, strategy);

    interior_rings(poly_out).resize(boost::size(interior_rings(poly_in)));

    const_iterator_type it_in = boost::begin(interior_rings(poly_in));
    iterator_type it_out = boost::begin(interior_rings(poly_out));

    for (; it_in != boost::end(interior_rings(poly_in)); it_in++, it_out++)
    {
        simplify_ring(*it_in, std::back_inserter(*it_out), max_distance, strategy);
    }
}

template<typename Y>
inline void simplify_polygon(Y const& poly_in, Y& poly_out, double max_distance)
{
    // Define default strategy
    typedef typename ring_type<Y>::type ring_type;
    typedef std::back_insert_iterator<ring_type> iterator_type;

    typedef typename point_type<Y>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance_segment
        <
            cs_tag,
            cs_tag,
            point_type,
            segment<const point_type>
        >::type strategy_type;

    strategy::simplify::douglas_peucker<ring_type, iterator_type, strategy_type> douglas;

    simplify_polygon(poly_in, poly_out, max_distance, douglas);
}

}} // namespace detail::simplify
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename G>
struct simplify
{
};

// Partial specializations
template <typename R>
struct simplify<linestring_tag, R>
{
    template<typename OutputIterator, typename S>
    static inline void apply(R const& range, OutputIterator out, double max_distance, S const& strategy)
    {
        strategy.simplify(range, out, max_distance);
    }

    template<typename OutputIterator>
    static inline void apply(R const& range, OutputIterator out, double max_distance)
    {
        // Define default strategy
        typedef typename point_type<R>::type point_type;
        typedef typename cs_tag<point_type>::type cs_tag;
        typedef typename strategy_distance_segment
            <
                cs_tag,
                cs_tag,
                point_type,
                segment<const point_type>
           >::type strategy_type;

        strategy::simplify::douglas_peucker<R, OutputIterator, strategy_type> douglas;

        detail::simplify::simplify_range_strategy(range, out, max_distance, douglas);
    }
};

template <typename R>
struct simplify<ring_tag, R>
{
    /// Simplify a ring, using a strategy
    template<typename S>
    static inline void apply(R const& ring_in, R& ring_out, double max_distance, S const& strategy)
    {
        using detail::simplify::simplify_ring;
        simplify_ring(ring_in, std::back_inserter(ring_out), max_distance, strategy);
    }

    /// Simplify a ring
    static inline void apply(R const& ring_in, R& ring_out, double max_distance)
    {
        // Define default strategy
        typedef typename point_type<R>::type point_type;
        typedef typename cs_tag<point_type>::type cs_tag;
        typedef typename strategy_distance_segment
            <
                cs_tag,
                cs_tag,
                point_type,
                segment<const point_type>
            >::type strategy_type;
        typedef std::back_insert_iterator<R> iterator_type;

        strategy::simplify::douglas_peucker<R, iterator_type, strategy_type> douglas;

        detail::simplify::simplify_ring(ring_in, std::back_inserter(ring_out), max_distance, douglas);
    }
};

template <typename P>
struct simplify<polygon_tag, P>
{
    /// Simplify a polygon, using a strategy
    template<typename S>
    static inline void apply(P const& poly_in, P& poly_out, double max_distance, S const& strategy)
    {
        detail::simplify::simplify_polygon(poly_in, poly_out, max_distance, strategy);
    }

    /// Simplify a polygon
    static inline void apply(P const& poly_in, P& poly_out, double max_distance)
    {
        detail::simplify::simplify_polygon(poly_in, poly_out, max_distance);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


// Model 1, using output iterator

/*!
    \brief Simplify a geometry
    \ingroup simplify
    \details The simplify algorithm removes points, keeping the shape as much as possible.
    This version of simplify uses an output iterator
    \param geometry the geometry to be simplified, being a ggl::linestring, vector, iterator pair, or any other boost compatible range
    \param out output iterator, outputs all simplified points
    \param max_distance distance (in units of input coordinates) of a vertex to other segments to be removed
    \par Example:
    The simplify algorithm can be used as following:
    \dontinclude doxygen_examples.cpp
    \skip example_simplify_linestring1
    \line {
    \until }
 */
template<typename G, typename O>
inline void simplify(const G& geometry, O out, double max_distance)
{
    typedef dispatch::simplify<typename tag<G>::type, G> simplify_type;
    simplify_type::apply(geometry, out, max_distance);
}

/*!
    \brief Simplify a geometry
    \ingroup simplify
    \details The simplify algorithm removes points, keeping the shape as much as possible.
    This version of simplify uses an output iterator and a simplify strategy
    \param geometry the geometry to be simplified, being a ggl::linestring, vector, iterator pair, or any other boost compatible range
    \param out output iterator, outputs all simplified points
    \param max_distance distance (in units of input coordinates) of a vertex to other segments to be removed
    \param strategy simplify strategy to be used for simplification, might include point-distance strategy
    \par Example:
    The simplify algorithm with strategy can be used as following:
    \dontinclude doxygen_examples.cpp
    \skip example_simplify_linestring2
    \line {
    \until }
 */
template<typename G, typename O, typename S>
inline void simplify(const G& geometry, O out, double max_distance, S const& strategy)
{
    typedef dispatch::simplify<typename tag<G>::type, G> simplify_type;
    simplify_type::apply(geometry, out, max_distance, strategy);
}

// Model 2, where output is same type as input

/*!
    \brief Simplify a geometry
    \ingroup simplify
    \details This version of simplify simplifies a geometry using the default strategy (Douglas Peucker),
    where the output is of the same geometry type as the input.
    \param geometry input geometry, to be simplified
    \param out output geometry, simplified version of the input geometry
    \param max_distance distance (in units of input coordinates) of a vertex to other segments to be removed
 */
template<typename G>
inline void simplify(const G& geometry, G& out, double max_distance)
{
    typedef dispatch::simplify<typename tag<G>::type, G> simplify_type;
    simplify_type::apply(geometry, out, max_distance);
}

/*!
    \brief Simplify a geometry
    \ingroup simplify
    \details This version of simplify simplifies a geometry using a specified strategy
    where the output is of the same geometry type as the input.
    \param geometry input geometry, to be simplified
    \param out output geometry, simplified version of the input geometry
    \param max_distance distance (in units of input coordinates) of a vertex to other segments to be removed
    \param strategy simplify strategy to be used for simplification, might include point-distance strategy
 */
template<typename G, typename S>
inline void simplify(const G& geometry, G& out, double max_distance, S const& strategy)
{
    typedef dispatch::simplify<typename tag<G>::type, G> simplify_type;
    simplify_type::apply(geometry, out, max_distance, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_SIMPLIFY_HPP
