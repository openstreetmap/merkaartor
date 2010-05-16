// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_CONVEX_HULL_HPP
#define GGL_ALGORITHMS_CONVEX_HULL_HPP


#include <boost/concept/requires.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/core/is_multi.hpp>

#include <ggl/core/concepts/point_concept.hpp>


#include <ggl/strategies/strategies.hpp>
#include <ggl/util/as_range.hpp>


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
namespace ggl {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace convex_hull {

template <typename Geometry, typename OutputIterator>
struct hull
{
    static inline OutputIterator apply(Geometry const& geometry,
            OutputIterator out)
    {
        typedef typename point_type<Geometry>::type point_type;

        typedef typename strategy_convex_hull
            <
                typename cs_tag<point_type>::type,
                point_type
            >::type strategy_type;

        strategy_type s(as_range<typename as_range_type<Geometry>::type>(geometry));
        s.get(out);
        return out;
    }
};


}} // namespace detail::convex_hull
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    bool IsMulti,
    typename Geometry,
    typename OutputIterator
 >
struct convex_hull {};

template <typename Linestring, typename OutputIterator>
struct convex_hull<linestring_tag, false, Linestring, OutputIterator>
    : detail::convex_hull::hull<Linestring, OutputIterator> 
{};

template <typename Ring, typename OutputIterator>
struct convex_hull<ring_tag, false, Ring, OutputIterator>
    : detail::convex_hull::hull<Ring, OutputIterator> 
{};

template <typename Polygon, typename OutputIterator>
struct convex_hull<polygon_tag, false, Polygon, OutputIterator>
    : detail::convex_hull::hull<Polygon, OutputIterator> 
{};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Calculate the convex hull of a geometry
    \ingroup convex_hull
    \param geometry the geometry to calculate convex hull from
    \param out an output iterator outputing points of the convex hull
    \return the output iterator
 */
template<typename Geometry, typename OutputIterator>
inline OutputIterator convex_hull(Geometry const& geometry, OutputIterator out)
{
    typedef typename boost::remove_const<Geometry>::type ncg_type;

    return dispatch::convex_hull
        <
            typename tag<ncg_type>::type,
            is_multi<ncg_type>::type::value,
            Geometry,
            OutputIterator
        >::apply(geometry, out);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_CONVEX_HULL_HPP
