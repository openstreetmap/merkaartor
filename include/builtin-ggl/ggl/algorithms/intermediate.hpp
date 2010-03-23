// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_INTERMEDIATE_HPP
#define GGL_ALGORITHMS_INTERMEDIATE_HPP

#include <cstddef>
#include <iterator>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/core/cs.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/strategies/strategies.hpp>

/*!
\defgroup intermediate intermediate calculation
The intermediate algorithm calculate points IN BETWEEN of other points
\par Purpose:
- Remove corners in rectangular lines / polygons. Calling them several times will result in smooth lines
- Creating 3D models
*/

namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intermediate {

template <typename Src, typename Dst, std::size_t Dimension, std::size_t DimensionCount>
struct calculate_coordinate
{
    static inline void apply(Src const& p1, Src const& p2, Dst& p)
    {
        ggl::set<Dimension>(p,
                    (ggl::get<Dimension>(p1) + ggl::get<Dimension>(p2)) / 2.0);
        calculate_coordinate<Src, Dst, Dimension + 1, DimensionCount>::apply(p1, p2, p);
    }
};

template <typename Src, typename Dst, std::size_t DimensionCount>
struct calculate_coordinate<Src, Dst, DimensionCount, DimensionCount>
{
    static inline void apply(Src const&, Src const&, Dst&)
    {
    }
};

template<typename R, typename Iterator>
struct range_intermediate
{
    static inline void apply(R const& range, bool start_and_end, Iterator out)
    {
        typedef typename point_type<R>::type point_type;
        typedef typename boost::range_const_iterator<R>::type iterator_type;

        iterator_type it = boost::begin(range);

        if (start_and_end)
        {
            (*out++) = *it;
        }

        iterator_type prev = it++;
        for (; it != boost::end(range); prev = it++)
        {
            point_type p;
            calculate_coordinate
                <
                    point_type,
                    point_type,
                    0,
                    dimension<point_type>::type::value
                >::apply(*prev, *it, p);
            *(out++) = p;
        }

        if (start_and_end)
        {
            (*out++) = *prev;
        }
    }
};

}} // namespace detail::intermediate
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename GeometryTag, typename G, typename Iterator>
struct intermediate  {};

template <typename G, typename Iterator>
struct intermediate<ring_tag, G, Iterator>
        : detail::intermediate::range_intermediate<G, Iterator> {};

template <typename G, typename Iterator>
struct intermediate<linestring_tag, G, Iterator>
        : detail::intermediate::range_intermediate<G, Iterator> {};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate intermediate of a geometry
    \ingroup intermediate
 */
template<typename G, typename Iterator>
inline void intermediate(const G& geometry, bool start_and_end, Iterator out)
{
    dispatch::intermediate<typename tag<G>::type, G, Iterator>::apply(geometry, start_and_end, out);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_INTERMEDIATE_HPP
