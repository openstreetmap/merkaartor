// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_BOX_HPP
#define GGL_GEOMETRIES_BOX_HPP

#include <cstddef>

#include <ggl/algorithms/assign.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/util/copy.hpp>

namespace ggl
{

/*!
    \brief Class box: defines a box made of two describing points
    \ingroup Geometry
    \details Box is always described by a min_corner() and a max_corner() point. If another
    rectangle is used, use linear_ring or polygon.
    \note Boxes are for selections and for calculating the envelope of geometries. Not all algorithms
    are implemented for box. Boxes are also used in Spatial Indexes.
    \tparam P point type. The box takes a point type as template parameter.
    The point type can be any point type.
    It can be 2D but can also be 3D or more dimensional.
    The box can also take a latlong point type as template parameter.
 */

template<typename P>
class box
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );

public:

    inline box() {}

    /*!
        \brief Constructor taking the minimum corner point and the maximum corner point
    */
    inline box(P const& min_corner, P const& max_corner)
    {
        copy_coordinates(min_corner, m_min_corner);
        copy_coordinates(max_corner, m_max_corner);
    }

    inline P const& min_corner() const { return m_min_corner; }
    inline P const& max_corner() const { return m_max_corner; }

    inline P& min_corner() { return m_min_corner; }
    inline P& max_corner() { return m_max_corner; }

private:

    P m_min_corner;
    P m_max_corner;
};


// Traits specializations for box above
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename P>
struct tag< box<P> >
{
    typedef box_tag type;
};

template <typename P>
struct point_type<box<P> >
{
    typedef P type;
};

template <typename P, std::size_t C, std::size_t D>
struct indexed_access<box<P>, C, D>
{
    typedef box<P> box_type;

    static inline typename ggl::coordinate_type<box_type>::type get(box_type const& b)
    {
        return (C == min_corner ? ggl::get<D>(b.min_corner()) : ggl::get<D>(b.max_corner()));
    }

    static inline void set(box_type& b, typename ggl::coordinate_type<box_type>::type const& value)
    {
        if (C == min_corner)
        {
            ggl::set<D>(b.min_corner(), value);
        }
        else
        {
            ggl::set<D>(b.max_corner(), value);
        }
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_GEOMETRIES_BOX_HPP
