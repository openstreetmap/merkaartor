// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_LENGTH_HPP
#define GGL_ALGORITHMS_LENGTH_HPP

#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/detail/calculate_null.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup length length calculation
The length algorithm is implemented for the linestring and the multi_linestring geometry and results
in the length of the linestring. If the points of a linestring have coordinates expressed in kilometers,
the length of the line is expressed in kilometers as well.
\par Example:
Example showing length calculation
\dontinclude doxygen_examples.cpp
\skip example_length_linestring_iterators1
\line {
\until }
*/

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace length {

template<typename Segment, typename S>
struct segment_length
{
    static inline double calculate(Segment const& segment, S const& strategy)
    {
        // BSG 10 APR 2009
        // TODO: the segment concept has to be such that it is easy to return a point from it.
        // Now it only accesses per coordinate
        return strategy(segment.first, segment.second);
    }
};

/*!
\brief Internal, calculates length of a linestring using iterator pairs and specified strategy
\note for_each could be used here, now that point_type is changed by boost range iterator
*/
template<typename R, typename S>
struct range_length
{
    static inline double calculate(R const& range, S const& strategy)
    {
        double sum = 0.0;

        typedef typename boost::range_const_iterator<R>::type iterator_type;
        iterator_type it = boost::begin(range);
        if (it != boost::end(range))
        {
            iterator_type previous = it++;
            while(it != boost::end(range))
            {
                // Add point-point distance using the return type belonging to strategy
                sum += strategy(*previous, *it);
                previous = it++;
            }
        }

        return sum;
    }
};

}} // namespace impl::length
#endif // DOXYGEN_NO_IMPL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{
template <typename Tag, typename G, typename S>
struct length : impl::calculate_null<double, G, S>
{
};

template <typename G, typename S>
struct length<linestring_tag, G, S> : impl::length::range_length<G, S>
{
};

// RING: length is currently 0.0 but it might be argued that it is the "perimeter"

template <typename G, typename S>
struct length<segment_tag, G, S> : impl::length::segment_length<G, S>
{
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Calculate length of a geometry
    \ingroup length
    \details The function length returns the length of a geometry, using the default distance-calculation-strategy
    \param geometry the geometry, being a ggl::linestring, vector, iterator pair, or any other boost compatible range
    \return the length
    Example showing length calculation on a vector
    \dontinclude doxygen_examples.cpp
    \skip example_length_linestring_iterators2
    \line {
    \until }
 */
template<typename G>
inline double length(const G& geometry)
{
    typedef typename point_type<G>::type point_type;
    typedef typename cs_tag<point_type>::type cs_tag;
    typedef typename strategy_distance
        <
        cs_tag,
        cs_tag,
        point_type,
        point_type
        >::type strategy_type;
    
    return dispatch::length
        <
        typename tag<G>::type,
        G,
        strategy_type
        >::calculate(geometry, strategy_type());
}

/*!
    \brief Calculate length of a geometry
    \ingroup length
    \details The function length returns the length of a geometry, using specified strategy
    \param geometry the geometry, being a ggl::linestring, vector, iterator pair, or any other boost compatible range
    \param strategy strategy to be used for distance calculations.
    \return the length
    \par Example:
    Example showing length calculation using iterators and the Vincenty strategy
    \dontinclude doxygen_examples.cpp
    \skip example_length_linestring_iterators3
    \line {
    \until }
 */
template<typename G, typename S>
inline double length(G const& geometry, S const& strategy)
{
    return dispatch::length<typename tag<G>::type, G, S>::calculate(geometry, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_LENGTH_HPP
