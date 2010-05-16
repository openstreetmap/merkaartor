// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_OVERLAPS_HPP
#define GGL_ALGORITHMS_OVERLAPS_HPP

#include <ggl/core/access.hpp>


namespace ggl
{

/*!
    \brief Determines overlap between two geometries
    \details parameters are now boxes but in the future will be geometries
    \ingroup boolean_relations
    \return true if there is overlap


    \par Source descriptions:
    - Egenhofer: Two objects overlap if they have common interior faces and the bounding faces have common parts
    with the opposite interior faces.

    \note Implemented in scratch the Generic Geometry library. Will be reworked internally to support more geometries
    but function call will stay as it is now.
    \see http://docs.codehaus.org/display/GEOTDOC/02+Geometry+Relationships#02GeometryRelationships-Overlaps
    where is stated that "inside" is not an "overlap", this is probably true and should then be implemented as such.

 */
template <typename B>
inline bool overlaps(const B& b1, const B& b2)
{
    return !(
            get<max_corner, 0>(b1) <= get<min_corner, 0>(b2) ||
            get<min_corner, 0>(b1) >= get<max_corner, 0>(b2) ||
            get<max_corner, 1>(b1) <= get<min_corner, 1>(b2) ||
            get<min_corner, 1>(b1) >= get<max_corner, 1>(b2)
            );

}

} // namespace ggl

#endif // GGL_ALGORITHMS_OVERLAPS_HPP
