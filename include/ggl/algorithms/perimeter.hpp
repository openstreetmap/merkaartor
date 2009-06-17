// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_PERIMETER_HPP
#define GGL_ALGORITHMS_PERIMETER_HPP

#include <cmath>
#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/length.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup perimeter perimeter calculation
The perimeter algorithm is implemented for polygon,box,linear_ring,multi_polygon
*/

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace perimeter {

template<typename R, typename S>
struct range_perimeter : impl::length::range_length<R, S>
{
};

// might be merged with "range_area" with policy to sum/subtract interior rings
template<typename Polygon, typename S>
struct polygon_perimeter
{
    static inline double calculate(Polygon const& poly, S const& strategy)
    {
        typedef typename ring_type<Polygon>::type ring_type;
        typedef typename boost::range_const_iterator
            <
            typename interior_type<Polygon>::type
            >::type iterator_type;

        double sum = std::abs(range_perimeter<ring_type, S>::calculate(exterior_ring(poly), strategy));

        for (iterator_type it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly)); it++)
        {
            sum += std::abs(range_perimeter<ring_type, S>::calculate(*it, strategy));
        }
        return sum;
    }
};

}} // namespace impl:;perimeter
#endif // DOXYGEN_NO_IMPL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// Default perimeter is 0.0, specializations implement calculated values
template <typename Tag, typename G, typename S>
struct perimeter : impl::calculate_null<double, G, S>
{
};

template <typename G, typename S>
struct perimeter<ring_tag, G, S> : impl::perimeter::range_perimeter<G, S>
{
};

template <typename G, typename S>
struct perimeter<polygon_tag, G, S> : impl::perimeter::polygon_perimeter<G, S>
{
};

// box,n-sphere: to be implemented

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate perimeter of a geometry
    \ingroup perimeter
    \details The function perimeter returns the perimeter of a geometry, using the default distance-calculation-strategy
    \param geometry the geometry, be it a ggl::ring, vector, iterator pair, or any other boost compatible range
    \return the perimeter
 */
template<typename G>
inline double perimeter(G const& geometry)
{
    typedef typename point_type<G>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance<cs_tag, cs_tag, point_type, point_type>::type strategy_type;

    typedef dispatch::perimeter
        <
        typename tag<G>::type,
        G,
        strategy_type
        > calculator_type;

    return calculator_type::calculate(geometry, strategy_type());
}

/*!
    \brief Calculate perimeter of a geometry
    \ingroup perimeter
    \details The function perimeter returns the perimeter of a geometry, using specified strategy
    \param geometry the geometry, be it a ggl::ring, vector, iterator pair, or any other boost compatible range
    \param strategy strategy to be used for distance calculations.
    \return the perimeter
 */
template<typename G, typename S>
inline double perimeter(G const& geometry, S const& strategy)
{
    typedef dispatch::perimeter<typename tag<G>::type, G, S> calculator_type;

    return calculator_type::calculate(geometry, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_PERIMETER_HPP
