// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_NSPHERE_HPP
#define GGL_GEOMETRIES_NSPHERE_HPP

#include <cstddef>

#include <ggl/algorithms/assign.hpp>
#include <ggl/core/concepts/nsphere_concept.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/util/copy.hpp>

namespace ggl
{

/*!
    \brief Class nsphere: defines a circle or a sphere: a point with radius
    \ingroup Geometry
    \details The name nsphere is quite funny but the best description of the class. It can be a circle (2D),
    a sphere (3D), or higher (hypersphere) or lower. According to Wikipedia this name is the most appropriate.
    It was mentioned on the Boost list.
    An alternative is the more fancy name "sphercle" but that might be a bit too much an invention.
    \note Circle is currently used for selections, for example polygon_in_circle. Currently not all
    algorithms are implemented for n-spheres.
    \tparam P point type of the center
    \tparam T number type of the radius
 */
template <typename P, typename T>
class nsphere
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );

public:

    typedef T radius_type;
    typedef typename coordinate_type<P>::type coordinate_type;

    nsphere()
        : m_radius(0)
    {
        detail::assign::assign_value(m_center, coordinate_type());
    }

    nsphere(P const& center, T const& radius)
        : m_radius(radius)
    {
        copy_coordinates(center, m_center);
    }

    inline P const& center() const { return m_center; }
    inline T const& radius() const { return m_radius; }

    inline void radius(T const& r) { m_radius = r; }
    inline P& center() { return m_center; }

private:

    P m_center;
    T m_radius;
};

// Traits specializations for n-sphere above
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename P, typename T>
struct tag< nsphere<P, T> >
{
    typedef nsphere_tag type;
};

template <typename P, typename T>
struct point_type<nsphere<P, T> >
{
    typedef P type;
};

template <typename P, typename T>
struct radius_type<nsphere<P, T> >
{
    typedef T type;
};

template <typename P, typename T>
struct access<nsphere<P, T> >
{
    typedef nsphere<P, T> nsphere_type;

    template <std::size_t D>
    static inline typename ggl::coordinate_type<nsphere_type>::type get(nsphere_type const& s)
    {
        return ggl::get<D>(s.center());
    }

    template <std::size_t D>
    static inline void set(nsphere_type& s, typename ggl::coordinate_type<nsphere_type>::type const& value)
    {
        ggl::set<D>(s.center(), value);
    }
};

template <typename P, typename T>
struct radius_access<nsphere<P, T>, T, 0>
{
    typedef nsphere<P, T> nsphere_type;

    static inline T get(nsphere_type const& s)
    {
        return s.radius();
    }

    static inline void set(nsphere_type& s, T const& value)
    {
        s.radius(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_GEOMETRIES_NSPHERE_HPP
