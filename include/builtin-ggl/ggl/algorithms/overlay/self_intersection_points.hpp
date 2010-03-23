// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_SELF_INTERSECTION_POINTS_HPP
#define GGL_ALGORITHMS_SELF_INTERSECTION_POINTS_HPP

#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/is_multi.hpp>

#include <ggl/algorithms/overlay/get_intersection_points.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace self_intersection_points {

template
<
    typename Geometry,
    typename IntersectionPoints
>
struct check_ips
{
    static inline bool apply(
            Geometry const& geometry,
            bool return_if_found,
            IntersectionPoints& intersection_points)
    {
        typedef typename ggl::sections
            <
                ggl::box < typename ggl::point_type<Geometry>::type >, 1
            > sections_type;

        sections_type sec;
        ggl::sectionalize(geometry, sec);

        bool trivial = true;
        for (typename boost::range_const_iterator<sections_type>::type
                    it1 = sec.begin();
            it1 != sec.end();
            ++it1)
        {
            for (typename boost::range_const_iterator<sections_type>::type
                        it2 = sec.begin();
                it2 != sec.end();
                ++it2)
            {
                if (! ggl::disjoint(it1->bounding_box, it2->bounding_box)
                    && ! it1->duplicate
                    && ! it2->duplicate
                    )
                {
                    ggl::detail::get_intersection_points::get_ips_in_sections
                    <
                        Geometry, Geometry,
                        typename boost::range_value<sections_type>::type,
                        typename boost::range_value<sections_type>::type,
                        IntersectionPoints
                    >::apply(
                            0, geometry, *it1,
                            0, geometry, *it2,
                            return_if_found,
                            intersection_points, trivial);
                }
            }
        }
        return trivial;
    }
};


}} // namespace detail::self_intersection_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    bool IsMulti,
    typename Geometry,
    typename IntersectionPoints

>
struct self_intersection_points
{
};


template<typename Ring, typename IntersectionPoints>
struct self_intersection_points
    <
        ring_tag, false, Ring,
        IntersectionPoints
    >
    : detail::self_intersection_points::check_ips
        <
            Ring,
            IntersectionPoints
        >
{};


template<typename Polygon, typename IntersectionPoints>
struct self_intersection_points
    <
        polygon_tag, false, Polygon,
        IntersectionPoints
    >
    : detail::self_intersection_points::check_ips
        <
            Polygon,
            IntersectionPoints
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate self intersections of a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam IntersectionPoints type of intersection container (e.g. vector of "intersection_point"'s)
    \param geometry geometry
    \param intersection_points container which will contain intersection points
    \return TRUE if it is trivial, else FALSE
 */
template <typename Geometry, typename IntersectionPoints>
inline bool get_intersection_points(Geometry const& geometry,
            IntersectionPoints& intersection_points)
{
    typedef typename boost::remove_const<Geometry>::type ncg_type;

    return dispatch::self_intersection_points
            <
                typename tag<ncg_type>::type,
                is_multi<ncg_type>::type::value,
                ncg_type,
               IntersectionPoints
            >::apply(geometry, false, intersection_points);
}



} // namespace ggl

#endif // GGL_ALGORITHMS_SELF_INTERSECTION_POINTS_HPP
