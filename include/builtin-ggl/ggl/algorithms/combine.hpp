// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_COMBINE_HPP
#define GGL_ALGORITHMS_COMBINE_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <ggl/arithmetic/arithmetic.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/concepts/box_concept.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/util/assign_box_corner.hpp>
#include <ggl/util/select_coordinate_type.hpp>

/*!
\defgroup combine combine: add a geometry to a bounding box
\par Geometries:
- BOX + BOX -> BOX: the box will be combined with the other box \image html combine_box_box.png
- BOX + POINT -> BOX: the box will combined with the point  \image html combine_box_point.png
\note Previously called "grow"
*/
namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace combine {

template
<
    typename Box, typename Point,
    std::size_t Dimension, std::size_t DimensionCount
>
struct point_loop
{
    typedef typename coordinate_type<Point>::type coordinate_type;

    static inline void apply(Box& box, Point const& source)
    {
        coordinate_type const coord = get<Dimension>(source);

        if (coord < get<min_corner, Dimension>(box))
        {
            set<min_corner, Dimension>(box, coord );
        }

        if (coord > get<max_corner, Dimension>(box))
        {
            set<max_corner, Dimension>(box, coord);
        }

        point_loop<Box, Point, Dimension + 1, DimensionCount>::apply(box, source);
    }
};


template
<
    typename Box, typename Point,
    std::size_t DimensionCount
>
struct point_loop<Box, Point, DimensionCount, DimensionCount>
{
    static inline void apply(Box&, Point const&) {}
};


template
<
    typename BoxIn, typename BoxOut,
    std::size_t Corner,
    std::size_t Dimension, std::size_t DimensionCount
>
struct box_loop
{
    typedef typename select_coordinate_type<BoxIn, BoxOut>::type coordinate_type;

    static inline void apply(BoxIn& box, BoxOut const& source)
    {
        coordinate_type const coord = get<Corner, Dimension>(source);

        if (coord < get<min_corner, Dimension>(box))
        {
            set<min_corner, Dimension>(box, coord);
        }

        if (coord > get<max_corner, Dimension>(box))
        {
            set<max_corner, Dimension>(box, coord);
        }

        box_loop
            <
                BoxIn, BoxOut, Corner, Dimension + 1, DimensionCount
            >::apply(box, source);
    }
};


template
<
    typename BoxIn, typename BoxOut,
    std::size_t Corner, std::size_t DimensionCount
>
struct box_loop<BoxIn, BoxOut, Corner, DimensionCount, DimensionCount>
{
    static inline void apply(BoxIn&, BoxOut const&) {}
};


// Changes a box b such that it also contains point p
template<typename Box, typename Point>
struct combine_box_with_point
    : point_loop<Box, Point, 0, dimension<Point>::type::value>
{};


// Changes a box such that the other box is also contained by the box
template<typename BoxOut, typename BoxIn>
struct combine_box_with_box
{
    static inline void apply(BoxOut& b, BoxIn const& other)
    {
        box_loop<BoxOut, BoxIn, min_corner, 0,
                    dimension<BoxIn>::type::value>::apply(b, other);
        box_loop<BoxOut, BoxIn, max_corner, 0,
                    dimension<BoxIn>::type::value>::apply(b, other);
    }
};

}} // namespace detail::combine
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename BoxOut, typename Geometry>
struct combine
{};


// Box + point -> new box containing also point
template <typename BoxOut, typename Point>
struct combine<point_tag, BoxOut, Point>
    : detail::combine::combine_box_with_point<BoxOut, Point>
{};


// Box + box -> new box containing two input boxes
template <typename BoxOut, typename BoxIn>
struct combine<box_tag, BoxOut, BoxIn>
    : detail::combine::combine_box_with_box<BoxOut, BoxIn>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Combines a box with another geometry (box, point)
    \ingroup combine
    \tparam Box type of the box
    \tparam Geometry of second geometry, to be combined with the box
    \param box box to combine another geometry with, might be changed
    \param geometry other geometry
 */
template <typename Box, typename Geometry>
inline void combine(Box& box, Geometry const& geometry)
{
    assert_dimension_equal<Box, Geometry>();
    dispatch::combine
        <
            typename tag<Geometry>::type,
            Box, Geometry
        >::apply(box, geometry);
}

} // namespace ggl

#endif // GGL_COMBINE_HPP
