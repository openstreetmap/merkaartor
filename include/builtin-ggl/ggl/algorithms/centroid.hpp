// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_CENTROID_HPP
#define GGL_ALGORITHMS_CENTROID_HPP

#include <cstddef>

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/core/exception.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/strategies/strategies.hpp>
#include <ggl/util/copy.hpp>
#include <ggl/util/loop.hpp>

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

namespace ggl
{

class centroid_exception : public ggl::exception
{
public:

    centroid_exception()  {}

    virtual char const* what() const throw()
    {
        return "centroid calculation exception";
    }
};



#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace centroid {

/*!
    \brief Generic function which checks if enough points are present
*/
template<typename P, typename R>
inline bool ring_ok(R const& ring, P& c)
{
    std::size_t const n = boost::size(ring);
    if (n > 1)
    {
        return true;
    }
    else if (n <= 0)
    {
        throw centroid_exception();
    }
    else
    {
        // n == 1: Take over the first point in a "coordinate neutral way"
        copy_coordinates(ring.front(), c);
        return false;
    }
    return true;
}

/*!
    \brief Calculate the centroid of a ring.
*/
template<typename Ring, typename Point, typename Strategy>
struct centroid_ring
{
    static inline void apply(Ring const& ring, Point& c, Strategy const& strategy)
    {
        if (ring_ok(ring, c))
        {
            typename Strategy::state_type state;
            loop(ring, strategy, state);
            state.centroid(c);
        }
    }
};


/*!
    \brief Centroid of a polygon.
    \note Because outer ring is clockwise, inners are counter clockwise,
    triangle approach is OK and works for polygons with rings.
*/
template<typename Polygon, typename Point, typename Strategy>
struct centroid_polygon
{
    static inline void apply(Polygon const& poly, Point& c, Strategy const& strategy)
    {
        if (ring_ok(exterior_ring(poly), c))
        {
            typename Strategy::state_type state;

            loop(exterior_ring(poly), strategy, state);

            typedef typename boost::range_const_iterator
                <
                    typename interior_type<Polygon>::type
                >::type iterator_type;

            for (iterator_type it = boost::begin(interior_rings(poly));
                 it != boost::end(interior_rings(poly));
                 ++it)
            {
                loop(*it, strategy, state);
            }
            state.centroid(c);
        }
    }
};

/*!
    \brief Calculate centroid (==center) of a box
    \todo Implement strategy
*/
template<typename Box, typename Point, typename Strategy>
struct centroid_box
{
    static inline void apply(Box const& box, Point& c, Strategy const&)
    {
        // TODO: adapt using strategies
        assert_dimension<Box, 2>();
        set<0>(c, (get<min_corner, 0>(box) + get<max_corner, 0>(box)) / 2);
        set<1>(c, (get<min_corner, 1>(box) + get<max_corner, 1>(box)) / 2);
    }
};

}} // namespace detail::centroid
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry, typename Point, typename Strategy>
struct centroid {};

template <typename Box, typename Point, typename Strategy>
struct centroid<box_tag, Box, Point, Strategy>
    : detail::centroid::centroid_box<Box, Point, Strategy>
{};

template <typename Ring, typename Point, typename Strategy>
struct centroid<ring_tag, Ring, Point, Strategy>
    : detail::centroid::centroid_ring<Ring, Point, Strategy>
{};

template <typename Polygon, typename Point, typename Strategy>
struct centroid<polygon_tag, Polygon, Point, Strategy>
    : detail::centroid::centroid_polygon<Polygon, Point, Strategy>

{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


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
    typedef typename point_type<G>::type point_type;
    typedef typename strategy_centroid
        <
            typename cs_tag<point_type>::type,
            P,
            point_type
        >::type strategy_type;

    dispatch::centroid
        <
            typename tag<G>::type,
            G,
            P,
            strategy_type
        >::apply(geometry, c, strategy_type());
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
inline void centroid(const G& geometry, P& c, S const& strategy)
{
    dispatch::centroid
        <
            typename tag<G>::type,
            G,
            P,
            S
        >::apply(geometry, c, strategy);
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
    centroid(geometry, c);
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
inline P make_centroid(const G& geometry, S const& strategy)
{
    P c;
    centroid(geometry, c, strategy);
    return c;
}

} // namespace ggl

#endif // GGL_ALGORITHMS_CENTROID_HPP
