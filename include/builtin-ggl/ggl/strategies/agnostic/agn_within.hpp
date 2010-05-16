// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_AGNOSTIC_WITHIN_HPP
#define GGL_STRATEGY_AGNOSTIC_WITHIN_HPP



#include <ggl/geometries/segment.hpp>

#include <ggl/strategies/strategy_traits.hpp>



namespace ggl
{
namespace strategy
{
    namespace within
    {

        /*!
            \brief Within detection using winding rule
            \tparam P point type of point to examine
            \tparam PS point type of segments, defaults to P
            \author Barend Gehrels
            \note The implementation is inspired by terralib http://www.terralib.org (LGPL)
            \note but totally revised afterwards, especially for cases on segments
            \note More efficient (less comparisons and no divison) than the cross count algorithm
            \note Only dependant on "side", -> agnostic, suitable for latlong
         */
        template<typename P, typename PS = P>
        class winding
        {
            private :
                typedef typename coordinate_type<P>::type PT;
                typedef typename coordinate_type<PS>::type ST;

                /*! subclass to keep state */
                struct windings
                {
                    int count;
                    bool touches;
                    const P& p;
                    inline explicit windings(const P& ap)
                        : count(0)
                        , touches(false)
                        , p(ap)
                    {}
                    inline bool within() const
                    {
                        return ! touches && count != 0;
                    }
                };

                template <size_t D>
                static inline int check_touch(const segment<const PS>& s, windings& state)
                {
                    const PT& p = get<D>(state.p);
                    const ST& s1 = get<0, D>(s);
                    const ST& s2 = get<1, D>(s);
                    if ((s1 <= p && s2 >= p) || (s2 <= p && s1 >= p))
                    {
                        state.touches = true;
                    }
                    return 0;
                }


                template <size_t D>
                static inline int check_segment(const segment<const PS>& s, windings& state)
                {
                    const PT& p = get<D>(state.p);
                    const ST& s1 = get<0, D>(s);
                    const ST& s2 = get<1, D>(s);

                    // Check if one of segment endpoints is at same level of point
                    bool eq1 = math::equals(s1, p);
                    bool eq2 = math::equals(s2, p);

                    if (eq1 && eq2)
                    {
                        // Both equal p -> segment is horizontal (or vertical for D=0)
                        // The only thing which has to be done is check if point is ON segment
                        return check_touch<1 - D>(s, state);
                    }

                    return
                          eq1 ? (s2 > p ?  1 : -1)  // P on level s1, UP/DOWN depending on s2
                        : eq2 ? (s1 > p ? -1 :  1)  // idem
                        : s1 < p && s2 > p ?  2     // P between s1 -> s2 --> UP
                        : s2 < p && s1 > p ? -2     // P between s2 -> s1 --> DOWN
                        : 0;
                }


            public :

                typedef windings state_type;

                inline bool operator()(const segment<const PS>& s, state_type& state) const
                {
                    int cnt = check_segment<1>(s, state);
                    if (cnt != 0)
                    {
                        typedef typename strategy_side<typename cs_tag<P>::type, P, PS>::type SS;

                        typename select_coordinate_type<P, PS>::type side = SS::side(s, state.p);

                        if (math::equals(side, 0))
                        {
                            // Point is lying on segment
                            state.touches = true;
                            state.count = 0;
                            return false;
                        }

                        // Side is NEG for right, POS for left.
                        // CNT is -2 for down, 2 for up (or -1/1)
                        // Side positive thus means UP and LEFTSIDE or DOWN and RIGHTSIDE
                        // See accompagnying figure (TODO)
                        int sd = (side > 0 ? 1 : -1) * cnt;
                        if (sd > 0)
                        {
                            state.count += cnt;
                        }

                    }
                    return ! state.touches;
                }

        };

    } // namespace within

} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P, typename PS>
struct strategy_within<cartesian_tag, cartesian_tag, P, PS>
{
    typedef strategy::within::winding<P, PS> type;
};

template <typename P, typename PS>
struct strategy_within<geographic_tag, geographic_tag, P, PS>
{
    typedef strategy::within::winding<P, PS> type;
};
#endif


} // namespace ggl


#endif // GGL_STRATEGY_AGNOSTIC_WITHIN_HPP
