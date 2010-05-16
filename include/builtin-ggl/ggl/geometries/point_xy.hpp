// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_POINT_XY_HPP
#define GGL_GEOMETRIES_POINT_XY_HPP

#include <cstddef>

#include <boost/mpl/int.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/geometries/point.hpp>

namespace ggl
{

/*!
    \brief 2D point in Cartesian coordinate system
    \ingroup Geometry
    \tparam T numeric type, arguments can be, for example, double, float, int
*/
template<typename T, typename C = cs::cartesian>
class point_xy : public point<T, 2, C>
{
public:

    /// Default constructor, does not initialize anything
    inline point_xy() : point<T, 2, C>() {}

    /// Constructor with x/y values
    inline point_xy(T const& x, T const& y) : point<T, 2, C>(x, y) {}

    /// Get x-value
    inline T const& x() const
    { return this->template get<0>(); }

    /// Get y-value
    inline T const& y() const
    { return this->template get<1>(); }

    /// Set x-value
    inline void x(T const& v)
    { this->template set<0>(v); }

    /// Set y-value
    inline void y(T const& v)
    { this->template set<1>(v); }

    /// Compare two points
    inline bool operator<(point_xy const& other) const
    {
        return math::equals(x(), other.x()) ? y() < other.y() : x() < other.x();
    }

};

// Adapt the point_xy to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename T, typename C>
struct tag<point_xy<T, C> >
{
    typedef point_tag type;
};

template<typename T, typename C>
struct coordinate_type<point_xy<T, C> >
{
    typedef T type;
};

template<typename T, typename C>
struct coordinate_system<point_xy<T, C> >
{
    typedef C type;
};

template<typename T, typename C>
struct dimension<point_xy<T, C> > : boost::mpl::int_<2> {};

template<typename T, typename C>
struct access<point_xy<T, C> >
{
    template <std::size_t I>
    static inline T get(point_xy<T, C> const& p)
    {
        return p.template get<I>();
    }

    template <std::size_t I>
    static inline void set(point_xy<T, C>& p, T const& value)
    {
        p.template set<I>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_GEOMETRIES_POINT_XY_HPP
