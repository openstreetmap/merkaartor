// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_WITHIN_HPP
#define GGL_STRATEGY_CARTESIAN_WITHIN_HPP



#include <ggl/geometries/segment.hpp>



namespace ggl
{
namespace strategy
{
    namespace within
    {
        /*!
            \brief Within detection using cross counting

            \author adapted from Randolph Franklin algorithm
            \author Barend and Maarten, 1995
            \author Revised for templatized library, Barend Gehrels, 2007
            \return true if point is in ring, works for closed rings in both directions
            \note Does NOT work correctly for point ON border
         */

        template<typename P, typename PS = P>
        struct franklin
        {
            private :
                /*! subclass to keep state */
                struct crossings
                {
                    P p;
                    bool crosses;
                    explicit crossings(const P& ap)
                        : p(ap)
                        , crosses(false)
                    {}
                    bool within() const
                    {
                        return crosses;
                    }
                };

            public :

                typedef crossings state_type;

                inline bool operator()(const segment<const PS>& s, state_type& state) const
                {
                    /* Algorithm:
                    if (
                        ( (y2 <= py && py < y1)
                            || (y1 <= py && py < y2) )
                        && (px < (x1 - x2)
                                * (py - y2)
                                    / (y1 - y2) + x2)
                        )
                            crosses = ! crosses
                    */


                    if (
                        ((get<1, 1>(s) <= get<1>(state.p) && get<1>(state.p) < get<0, 1>(s))
                            || (get<0, 1>(s) <= get<1>(state.p) && get<1>(state.p) < get<1, 1>(s)))
                        && (get<0>(state.p) < (get<0, 0>(s) - get<1, 0>(s))
                            * (get<1>(state.p) - get<1, 1>(s))
                                    / (get<0, 1>(s) - get<1, 1>(s)) + get<1, 0>(s))
                        )
                    {
                        state.crosses = ! state.crosses;
                    }
                    return true;
                }
        };



    } // namespace within



} // namespace strategy





} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_WITHIN_HPP
