// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_REPLACE_POINT_TYPE_HPP
#define GGL_CORE_REPLACE_POINT_TYPE_HPP


#include <boost/type_traits/remove_const.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>
#include <ggl/core/coordinate_type.hpp>

// For now: use ggl-provided geometries
// TODO: figure out how to get the class and replace the type
// TODO: take "const" into account
#include <ggl/geometries/point.hpp>
#include <ggl/geometries/linestring.hpp>
#include <ggl/geometries/linear_ring.hpp>
#include <ggl/geometries/polygon.hpp>
#include <ggl/geometries/segment.hpp>
#include <ggl/geometries/box.hpp>
#include <ggl/geometries/nsphere.hpp>


namespace ggl {

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{
template <typename GeometryTag, typename Geometry, typename NewPointType>
struct replace_point_type {};

template <typename Geometry, typename NewPointType>
struct replace_point_type<point_tag, Geometry, NewPointType>
{
    typedef NewPointType type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<linestring_tag, Geometry, NewPointType>
{
    typedef linestring<NewPointType> type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<segment_tag, Geometry, NewPointType>
{
    typedef segment<NewPointType> type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<ring_tag, Geometry, NewPointType>
{
    typedef linear_ring<NewPointType> type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<box_tag, Geometry, NewPointType>
{
    typedef box<NewPointType> type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<polygon_tag, Geometry, NewPointType>
{
    typedef polygon<NewPointType> type;
};

template <typename Geometry, typename NewPointType>
struct replace_point_type<nsphere_tag, Geometry, NewPointType>
{
    typedef typename ggl::coordinate_type<Geometry>::type coortype;
    typedef nsphere<NewPointType, coortype> type;
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


template <typename Geometry, typename NewPointType>
struct replace_point_type : core_dispatch::replace_point_type
        <
        typename tag<Geometry>::type,
        typename boost::remove_const<Geometry>::type,
        NewPointType
        >
{};

} // namespace ggl

#endif // GGL_CORE_REPLACE_POINT_TYPE_HPP
