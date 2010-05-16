// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_ASSIGN_BOX_CORNER_HPP
#define GGL_UTIL_ASSIGN_BOX_CORNER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <ggl/core/coordinate_dimension.hpp>

// TODO: merge with "assign"

namespace ggl
{

/*!
    \brief Assign one point of a 2D box
    \ingroup assign
    \todo will be merged with assign
*/
template <std::size_t C1, std::size_t C2, typename B, typename P>
inline void assign_box_corner(B const& box, P& point)
{
    // Be sure both are 2-Dimensional
    assert_dimension<B, 2>();
    assert_dimension<P, 2>();

    // Copy coordinates
    typedef typename coordinate_type<P>::type coordinate_type;

    set<0>(point, boost::numeric_cast<coordinate_type>(get<C1, 0>(box)));
    set<1>(point, boost::numeric_cast<coordinate_type>(get<C2, 1>(box)));
}

/*!
    \brief Assign the 4 points of a 2D box
    \ingroup assign
    \todo will be merged with assign
    \note The order can be crucial. Most logical is LOWER, UPPER and sub-order LEFT, RIGHT
*/
template <typename B, typename P>
inline void assign_box_corners(B const& box, P& lower_left, P& lower_right, P& upper_left, P& upper_right)
{
    assign_box_corner<min_corner, min_corner>(box, lower_left);
    assign_box_corner<max_corner, min_corner>(box, lower_right);
    assign_box_corner<min_corner, max_corner>(box, upper_left);
    assign_box_corner<max_corner, max_corner>(box, upper_right);
}

} // namespace

#endif // GGL_UTIL_ASSIGN_BOX_CORNER_HPP
