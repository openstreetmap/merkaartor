// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRIES_POINT_LL_HPP
#define GGL_GEOMETRIES_POINT_LL_HPP

#include <cstddef>
#include <sstream>
#include <string>

#include <boost/numeric/conversion/cast.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/arithmetic/arithmetic.hpp>
#include <ggl/geometries/point.hpp>
#include <ggl/util/copy.hpp>

#include <ggl/extensions/gis/latlong/detail/graticule.hpp>

namespace ggl
{

/*!
    \brief Point using spherical coordinates \a lat and \a lon, on Earth
    \ingroup Geometry
    \details The point_ll class implements a point with lat and lon functions.
    It can be constructed using latitude and longitude classes. The latlong
    class can be defined in degrees or in radians. There is a conversion method
    from degree to radian, and from radian to degree.
    \tparam T coordinate type, double (the default) or float
        (it might be int as well)
    \tparam C coordinate system, optional, should includes degree/radian
        indication, defaults to geographic<degree>
    \tparam D dimensions, optional, defaults to 2
    \note There is NO constructor with two values to avoid
        exchanging lat and long
    \note Construction with latitude and longitude can be done in both orders,
        so lat/long and long/lat
    \par Example:
    Example showing how the point_ll class can be constructed. Note that it
        can also be constructed using
    decimal degrees (43.123).
    \dontinclude doxygen_examples.cpp
    \skip example_point_ll_construct
    \line {
    \until }
*/
template
<
    typename T = double,
    typename C = cs::geographic<degree>,
    std::size_t D = 2
>
class point_ll : public point<T, D, C>
{
public:

    /// Default constructor, does not initialize anything
    inline point_ll() : point<T, D, C>() {}

    /// Constructor with longitude/latitude
    inline point_ll(longitude<T> const& lo, latitude<T> const& la)
        : point<T, D, C>(lo, la) {}

    /// Constructor with latitude/longitude
    inline point_ll(latitude<T> const& la, longitude<T> const& lo)
        : point<T, D, C>(lo, la) {}

    /// Get longitude
    inline T const& lon() const { return this->template get<0>(); }
    /// Get latitude
    inline T const& lat() const { return this->template get<1>(); }

    /// Set longitude
    inline void lon(T const& v) { this->template set<0>(v); }
    /// Set latitude
    inline void lat(T const& v) { this->template set<1>(v); }

    /// Set longitude using dms class
    inline void lon(dms<east, T> const& v)
    {
        this->template set<0>(v.as_value());
    }
    inline void lon(dms<west, T> const& v)
    {
        this->template set<0>(v.as_value());
    }

    inline void lat(dms<north, T> const& v)
    {
        this->template set<1>(v.as_value());
    }
    inline void lat(dms<south, T> const& v)
    {
        this->template set<1>(v.as_value());
    }
};

// Adapt the point_ll to the concept
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template <typename T, typename C, std::size_t D>
struct tag<point_ll<T, C, D> >
{
    typedef point_tag type;
};

template<typename T, typename C, std::size_t D>
struct coordinate_type<point_ll<T, C, D> >
{
    typedef T type;
};

template<typename T, typename C, std::size_t D>
struct coordinate_system<point_ll<T, C, D> >
{
    typedef C type;
};

template<typename T, typename C, std::size_t D>
struct dimension<point_ll<T, C, D> > : boost::mpl::int_<D> {};

template<typename T, typename C, std::size_t D>
struct access<point_ll<T, C, D> >
{
    template <size_t I>
    static inline T get(point_ll<T, C, D> const& p)
    {
        return p.template get<I>();
    }

    template <size_t I>
    static inline void set(point_ll<T, C, D>& p, T const& value)
    {
        p.template set<I>(value);
    }
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_GEOMETRIES_POINT_LL_HPP
