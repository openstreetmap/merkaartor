// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ENVELOPE_HPP
#define GGL_MULTI_ENVELOPE_HPP

#include <vector>

#include <ggl/core/exterior_ring.hpp>
#include <ggl/algorithms/envelope.hpp>

#include <ggl/multi/core/point_type.hpp>


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL

namespace detail { namespace envelope {


template<typename MultiLinestring, typename Box, typename Strategy>
struct envelope_multi_linestring
{
    static inline void apply(MultiLinestring const& mp, Box& mbr, Strategy const& strategy)
    {
        typename Strategy::state_type state(mbr);
        for (typename boost::range_const_iterator<MultiLinestring>::type
                    it = mp.begin();
            it != mp.end();
            ++it)
        {
            envelope_range_state(*it, strategy, state);
        }
    }
};


// version for multi_polygon: outer linear_ring's of all polygons
template<typename MultiPolygon, typename Box, typename Strategy>
struct envelope_multi_polygon
{
    static inline void apply(MultiPolygon const& mp, Box& mbr, Strategy const& strategy)
    {
        typename Strategy::state_type state(mbr);
        for (typename boost::range_const_iterator<MultiPolygon>::type
                    it = mp.begin();
            it != mp.end();
            ++it)
        {
            envelope_range_state(exterior_ring(*it), strategy, state);
        }
    }
};


}} // namespace detail::envelope

#endif


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename M, typename B, typename S>
struct envelope<multi_point_tag, box_tag, M, B, S>
    : detail::envelope::envelope_range<M, B, S>
{};

template <typename M, typename B, typename S>
struct envelope<multi_linestring_tag, box_tag, M, B, S>
    : detail::envelope::envelope_multi_linestring<M, B, S>
{};


template <typename M, typename B, typename S>
struct envelope<multi_polygon_tag, box_tag, M, B, S>
    : detail::envelope::envelope_multi_polygon<M, B, S>
{};


} // namespace dispatch
#endif




} // namespace ggl


#endif // GGL_MULTI_ENVELOPE_HPP
