// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_INTERSECTION_HPP
#define GGL_ALGORITHMS_INTERSECTION_HPP

#include <ggl/algorithms/intersection_linestring.hpp>

#include <ggl/algorithms/overlay/get_intersection_points.hpp>
#include <ggl/algorithms/overlay/merge_intersection_points.hpp>
#include <ggl/algorithms/overlay/enrich_intersection_points.hpp>
#include <ggl/algorithms/overlay/traverse.hpp>

#include <ggl/algorithms/convert.hpp>
#include <ggl/algorithms/within.hpp>



/*!
\defgroup intersection intersection (AND operation) and clipping
\details The intersection of two geometries A and B is the geometry containing all points of A also belonging to B,
but no other elements. The so-called clip is an intersection of a geometry with a box.
\par Source description:
- OGC: Returns a geometric object that represents the Point set intersection of this geometric object with another Geometry.
\see http://en.wikipedia.org/wiki/Intersection_(set_theory)
\note Any intersection can result in no geometry at all

\note Used strategies still have to be modelled. Working only for cartesian
\par Geometries:
The intersection result is painted with a red outline.
- clip: POLYGON + BOX -> output iterator of polygons
\image html clip_polygon.png
- clip: LINESTRING + BOX -> output iterator of linestrings
\image html clip_linestring.png
\note There are some difficulties to model an intersection in the template world. The intersection of two segments can
result into nothing, into a point, into another segment. At compiletime the result type is not known. An output iterator
iterating points is appropriate here.
\image html clip_segment_segment.png
An intersection of two linestrings can result into nothing, one or more points, one or more segments or one or more
linestrings. So an output iterator will NOT do here.
So the output might be changed into a unified algorithm where the output is a multi-geometry.
For the current clip-only operations the output iterator will do.

\par Example:
Example showing clipping of linestring with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_linestring1
\line {
\until }
\par Example:
Example showing clipping of vector, outputting vectors, with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_linestring2
\line {
\until }
\par Example:
Example showing clipping of polygon with box
\dontinclude doxygen_examples.cpp
\skip example_intersection_polygon1
\line {
\until }
*/

namespace ggl
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag1, typename Tag2, typename Tag3,
    typename G1, typename G2,
    typename OutputIterator,
    typename GeometryOut
>
struct intersection {};


template
<
    typename Linestring, typename Box,
    typename OutputIterator, typename GeometryOut
>
struct intersection
    <
        linestring_tag, box_tag, linestring_tag,
        Linestring, Box,
        OutputIterator, GeometryOut
    >
{
    static inline OutputIterator apply(Linestring const& linestring,
            Box const& box, OutputIterator out)
    {
        typedef typename point_type<GeometryOut>::type point_type;
        strategy::intersection::liang_barsky<Box, point_type> strategy;
        return impl::intersection::clip_linestring_with_box<GeometryOut>(box, linestring, out, strategy);
    }
};


template
<
    typename Polygon1, typename Polygon2,
    typename OutputIterator, typename GeometryOut
>
struct intersection
    <
        polygon_tag, polygon_tag, polygon_tag,
        Polygon1, Polygon2,
        OutputIterator, GeometryOut
    >
{
    static inline OutputIterator apply(Polygon1 const& polygon1,
                Polygon2 const& polygon2, OutputIterator out)
    {
        typedef impl::intersection::intersection_point
            <
                typename ggl::point_type<GeometryOut>::type
            > ip_type;
        typedef std::deque<ip_type> ips_container;

        typedef typename ggl::ring_type<GeometryOut>::type ring_type;

        ips_container ips;

        bool non_trivial = ggl::get_intersection_points(polygon1, polygon2, ips);

        if (ips.size() <= 0)
        {
            // If there are no IP-s, check if one point is in other polygon
            // assume both polygons having points
            if (ggl::within(ggl::exterior_ring(polygon1).front(), polygon2))
            {
                // Assume same type (output = input)
                // TODO: solve this (we go to specialize again...)
                *out = polygon1;
                out++;
            }
            else if (ggl::within(ggl::exterior_ring(polygon2).front(), polygon1))
            {
                *out = polygon2;
                out++;
            }
        }
        else
        {
            if (non_trivial)
            {
                ggl::merge_intersection_points(ips);
            }
            ggl::enrich_intersection_points(ips, non_trivial);

            std::vector<ring_type> v;
            ggl::traverse<ring_type>
                (
                    polygon1,
                    polygon2,
                    -1,
                    ips, std::back_inserter(v)
                );


            // TODO:
            // assemble rings / inner rings / to polygons
            for (typename std::vector<ring_type>::const_iterator it = v.begin();
                it != v.end(); ++it)
            {
                // How can we avoid the double copy here! It is really bad!
                // We have to create a polygon, then copy it to the output iterator.
                // Having an output-vector would have been better: append it to the vector!
                // So output iterators are not that good.
                GeometryOut poly;
                poly.outer() = *it;
                *out = poly;
                out++;
            }
        }


        return out;
    }
};


template
<
    typename Box, typename Polygon,
    typename OutputIterator, typename GeometryOut
>
struct intersection
<
    polygon_tag, box_tag, polygon_tag,
    Polygon, Box,
    OutputIterator, GeometryOut
>
{
    static inline OutputIterator apply(Polygon const& poly,
                Box const& box, OutputIterator out)
    {
        GeometryOut as_polygon;
        ggl::convert(box, as_polygon);
        return intersection
            <
                polygon_tag, polygon_tag, polygon_tag,
                Polygon, Polygon,
                OutputIterator, GeometryOut
            >::apply(poly, as_polygon, out);
    }
};



template
<
    typename GeometryTag1, typename GeometryTag2, typename GeometryTag3,
    typename G1, typename G2,
    typename OutputIterator, typename GeometryOut
>
struct intersection_reversed
{
    static inline OutputIterator apply(G1 const& g1, G2 const& g2, OutputIterator out)
    {
        return intersection
            <
                GeometryTag2, GeometryTag1, GeometryTag3,
                G2, G1,
                OutputIterator, GeometryOut
            >::apply(g2, g1, out);
    }
};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Intersects two geometries which each other
    \ingroup intersection
    \details A sequence of points is intersected (clipped) by the specified box
    and the resulting linestring, or pieces of linestrings, are sent to the specified output operator.
    \tparam GeometryOut output geometry type, must be specified
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \tparam OutputIterator output iterator
    \param geometry1 first geometry (currently only a BOX)
    \param geometry2 second geometry (range, linestring, polygon)
    \param out the output iterator, outputting linestrings or polygons
    \return the output iterator
    \note For linestrings: the default clipping strategy, Liang-Barsky, is used. The algorithm is currently only
    implemented for 2D xy points. It could be generic for most ll cases, but not across the 180
    meridian so that issue is still on the todo-list.
*/
template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    typename OutputIterator
>
inline OutputIterator intersection(Geometry1 const& geometry1,
            Geometry2 const& geometry2,
            OutputIterator out)
{

    return boost::mpl::if_c
        <
            reverse_dispatch<Geometry1, Geometry2>::type::value,
            dispatch::intersection_reversed
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut
            >,
            dispatch::intersection
            <
                typename tag<Geometry1>::type,
                typename tag<Geometry2>::type,
                typename tag<GeometryOut>::type,
                Geometry1,
                Geometry2,
                OutputIterator, GeometryOut
            >
        >::type::apply(geometry1, geometry2, out);
}

} // ggl

#endif //GGL_ALGORITHMS_INTERSECTION_HPP
