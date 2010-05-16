// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_CORE_CS_HPP
#define GGL_CORE_CS_HPP

#include <cstddef>

#include <boost/type_traits.hpp>

#include <ggl/core/coordinate_system.hpp>
#include <ggl/core/tags.hpp>

/*!
\defgroup cs coordinate systems
\brief Defines coordinate systems
\details Coordinate systems are essential for any point in the Generic Geometry Library. Many
algorithms such as distance or transform use coordinate systems to select the strategy to use.
*/

namespace ggl
{

/*!
    \brief Unit of plan angles: degrees
    \ingroup cs
*/
class degree {};


/*!
    \brief Unit of plan angles: radians
    \ingroup cs
*/
class radian {};


namespace cs
{

/*!
    \brief Cartesian coordinate system
    \details Defines the Cartesian or rectangular coordinate system where points are defined in 2 or 3 (or more)
    dimensions and usually (but not always) known as x,y,z
    \see http://en.wikipedia.org/wiki/Cartesian_coordinate_system
    \ingroup cs
*/
struct cartesian {};




/*!
    \brief Geographic coordinate system, in degree or in radian
    \details Defines the geographic coordinate system where points are defined in two angles and usually
    known as lat,long or lo,la or phi,lambda
    \see http://en.wikipedia.org/wiki/Geographic_coordinate_system
    \ingroup cs
    \note might be moved to extensions/gis/geographic
*/
template<typename DegreeOrRadian>
struct geographic
{
    typedef DegreeOrRadian units;
};



/*!
    \brief Spherical coordinate system, in degree or in radian
    \details Defines the spherical coordinate system where points are defined in two angles
        and an optional radius usually known as r, theta, phi
    \par Coordinates:
    - coordinate 0:
        0 <= phi < 2pi is the angle between the positive x-axis and the line from the origin to the P projected onto the xy-plane.
    - coordinate 1:
        0 <= theta <= pi is the angle between the positive z-axis and the line formed between the origin and P.
    - coordinate 2 (if specified):
        r >= 0 is the distance from the origin to a given point P.

    \see http://en.wikipedia.org/wiki/Spherical_coordinates
    \ingroup cs
*/
template<typename DegreeOrRadian>
struct spherical
{
    typedef DegreeOrRadian units;
};

/*!
    \brief Polar coordinate system
    \details Defines the polar coordinate system "in which each point on a plane is determined by an angle and a distance"
    \see http://en.wikipedia.org/wiki/Polar_coordinates
    \ingroup cs
*/
template<typename DegreeOrRadian>
struct polar
{
    typedef DegreeOrRadian units;
};


} // namespace cs

namespace traits
{
/*!
    \brief Traits class defining coordinate system tag, bound to coordinate system
    \ingroup traits
    \tparam CoordinateSystem coordinate system
*/
template <typename CoordinateSystem>
struct cs_tag
{
};

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS

template<typename DegreeOrRadian>
struct cs_tag<cs::geographic<DegreeOrRadian> >
{
    typedef geographic_tag type;
};

template<typename DegreeOrRadian>
struct cs_tag<cs::spherical<DegreeOrRadian> >
{
    typedef spherical_tag type;
};

template<>
struct cs_tag<cs::cartesian>
{
    typedef cartesian_tag type;
};


#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS
} // namespace traits

/*!
    \brief Meta-function returning coordinate system tag (cs family) of any geometry
    \ingroup core
*/
template <typename G>
struct cs_tag
{
    typedef typename traits::cs_tag
        <
        typename ggl::coordinate_system<G>::type
        >::type type;
};


/*!
    \brief Meta-function to verify if a coordinate system is radian
    \ingroup core
*/
template <typename CoordinateSystem>
struct is_radian : boost::true_type {};


#ifndef DOXYGEN_NO_SPECIALIZATIONS

// Specialization for any degree coordinate systems
template <template<typename> class CoordinateSystem>
struct is_radian< CoordinateSystem<degree> > : boost::false_type
{
};

#endif // DOXYGEN_NO_SPECIALIZATIONS

} // namespace ggl

#endif // GGL_CORE_CS_HPP
