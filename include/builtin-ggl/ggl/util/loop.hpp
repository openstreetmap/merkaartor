// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_LOOP_HPP
#define GGL_UTIL_LOOP_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/geometries/segment.hpp>

namespace ggl
{

/*!
    \brief Loops through segments of a container and call specified functor for all segments.
    \ingroup loop
    \details for_each like implementation to:
    - walk over const segments of a linestring/polygon
    - be able to break out the loop (if the functor returns false)
    - have a const functor and keep state in separate state-object
    - we still keep the "functor" here so it might be a function or an object, at this place
    - in most algorithms the typename F::state_type is used; in those places it must be an object

    \tparam R range type, for example a vector, linestring, linear_ring
    \tparam F functor type, class or function, not modified by the algorithm
    \tparam S state type, might be modified
    \param range range (linestring iterator pair,vector,list,deque) containing points
    \param functor functor which is called at each const segment
    \param state state, specified separately from the strategy functor
    \return false if the functor returns false, otherwise true
    \par Concepts
    - \a V
        - const_iterator begin()
        - const_iterator end()
        - value_type
    - \a F
        - <em>if it is a function functor</em>: bool \b function (const segment&, state&)
        - <em>if it is a class functor</em>: bool operator()(const segment&, state&) const
    - \a S
        - no specific requirements here, requirments given by F
    \note Some algorithms from the Generic Geometry Library, for example within, centroid,
    use this method.
    \par Examples:
    First example, using a class functor
    \dontinclude doxygen_examples.cpp
    \skip example_loop1
    \line {
    \until //:\\
    Second example, using a function functor and latlong coordinates
    \dontinclude doxygen_examples.cpp
    \skip example_loop2
    \line {
    \until //:\\
*/
template<typename R, typename F, typename S>
inline bool loop(R const& range, F const& functor, S& state)
{
    typedef typename boost::range_const_iterator<R>::type iterator_type;

    iterator_type it = boost::begin(range);
    if (it != boost::end(range))
    {
        iterator_type previous = it++;
        while(it != boost::end(range))
        {
            segment<const typename boost::range_value<R>::type> s(*previous, *it);
            if (! functor(s, state))
            {
                return false;
            }
            previous = it++;
        }
    }
    return true;
}

} // namespace ggl

#endif // GGL_UTIL_LOOP_HPP
