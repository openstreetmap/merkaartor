// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_ENVELOPE_HPP
#define GGL_STRATEGY_CARTESIAN_ENVELOPE_HPP



#include <ggl/geometries/point_xy.hpp>
#include <ggl/geometries/segment.hpp>

#include <ggl/algorithms/combine.hpp>


namespace ggl
{
namespace strategy
{
    namespace envelope
    {
        // envelope calculation strategy for xy-points
        template <typename P, typename B>
        struct combine_xy
        {
            struct state
            {
                B& m_box;
                state(B& box) : m_box(box)
                {
                    assign_inverse(m_box);
                }
            };

            typedef state state_type;

            void operator()(const P& p, state_type& s) const
            {
                ggl::combine(s.m_box, p);
            }
        };
    } // namespace envelope

} // namespace strategy



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P, typename B>
struct strategy_envelope<cartesian_tag, cartesian_tag, P, B>
{
    typedef strategy::envelope::combine_xy<P, B> type;
};


#endif


} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_ENVELOPE_HPP
