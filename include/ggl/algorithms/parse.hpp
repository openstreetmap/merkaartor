// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_PARSE_HPP
#define GGL_ALGORITHMS_PARSE_HPP

#include <string>

#include <boost/concept/requires.hpp>

#include <ggl/core/tags.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup parse parse and assign string values
*/

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl
{

} // namespace impl
#endif // DOXYGEN_NO_IMPL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
template <typename Tag, typename G>
struct parsing
{
};

template <typename P>
struct parsing<point_tag, P>
{
    template <typename S>
    static inline void parse(P& point, std::string const& c1, std::string const& c2, S const& strategy)
    {
        assert_dimension<P, 2>();
        dms_result r1 = strategy(c1.c_str());
        dms_result r2 = strategy(c2.c_str());

        if (0 == r1.axis())
            set<0>(point, r1);
        else
            set<1>(point, r1);

        if (0 == r2.axis())
            set<0>(point, r2);
        else
            set<1>(point, r2);
    }

    static inline void parse(P& point, std::string const& c1, std::string const& c2)
    {
        // strategy-parser corresponding to degree/radian
        typename strategy_parse
            <
            typename cs_tag<P>::type,
            typename coordinate_system<P>::type
            >::type strategy;

        parse(point, c1, c2, strategy);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief parse two strings to a spherical/geographic point, using W/E/N/S
    \ingroup parse
 */
template <typename G>
inline void parse(G& geometry, const std::string& c1, const std::string& c2)
{
    dispatch::parsing<typename tag<G>::type, G>::parse(geometry, c1, c2);
}

/*!
    \brief parse two strings to a spherical/geographic point, using a specified strategy
    \details user can use N/E/S/O or N/O/Z/W or other formats
    \ingroup parse
 */
template <typename G, typename S>
inline void parse(G& geometry, const std::string& c1, const std::string& c2, const S& strategy)
{
    dispatch::parsing<typename tag<G>::type, G>::parse(geometry, c1, c2, strategy);
}

// There will be a parsing function with three arguments (ANGLE,ANGLE,RADIUS)

template <typename G>
inline G parse(const std::string& c1, const std::string& c2)
{
    G geometry;
    dispatch::parsing<typename tag<G>::type, G>::parse(geometry, c1, c2);
    return geometry;
}

} // namespace ggl

#endif // GGL_ALGORITHMS_PARSE_HPP
