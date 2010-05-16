// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_SIDE_HPP
#define GGL_STRATEGY_CARTESIAN_SIDE_HPP



#include <ggl/geometries/point_xy.hpp>
#include <ggl/geometries/segment.hpp>

#include <ggl/util/select_coordinate_type.hpp>



namespace ggl
{
namespace strategy
{
    namespace side
    {

        template <typename P, typename PS>
        struct xy_side
        {

            // Check at which side of a segment a point lies:
            // left of segment (> 0), right of segment (< 0), on segment (0)
            // In fact this is twice the area of a triangle
            static inline typename select_coordinate_type<P, PS>::type
                side(const segment<const PS>& s, const P& p)
            {
                typedef typename select_coordinate_type<P, PS>::type T;

                // Todo: might be changed to subtract_point
                T dx = get<1, 0>(s) - get<0, 0>(s);
                T dy = get<1, 1>(s) - get<0, 1>(s);
                T dpx = get<0>(p) - get<0, 0>(s);
                T dpy = get<1>(p) - get<0, 1>(s);
                return dx * dpy - dy * dpx;
            }


            static inline int side(const P& p0, const P& p1, const P& p2)
            {
                typename coordinate_type<P>::type s = side(segment<const P>(p0, p1), p2);
                return s > 0 ? 1 : s < 0 ? -1 : 0;
            }
        };

    } // namespace side
} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P, typename PS>
struct strategy_side<cartesian_tag, P, PS>
{
    typedef strategy::side::xy_side<P, PS> type;
};
#endif

} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_SIDE_HPP
