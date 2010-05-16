// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_STRATEGIES_SPHERICAL_SPH_ENVELOPE_HPP
#define GGL_GEOMETRY_STRATEGIES_SPHERICAL_SPH_ENVELOPE_HPP

#include <boost/numeric/conversion/bounds.hpp>
#include <boost/numeric/conversion/cast.hpp>

// NOTE: maybe evaluate/rework this using new "compare" strategy
// - implement "compare" for latlong (e.g. return true if distance-difference < 90 deg, so 170 < -170 but 90 > -180)

#include <ggl/strategies/spherical/haversine.hpp>

namespace ggl
{
namespace strategy
{
    namespace envelope
    {
        // envelope calculation strategy for latlong-points
        namespace shift
        {
            template <typename D>
            struct shifted
            {
            };

            template<>
            struct shifted<radian>
            {
                inline static double shift() { return math::two_pi; }
            };
            template<>
            struct shifted<degree>
            {
                inline static double shift() { return 360.0; }
            };

        }

        /*!
            \par Algorithm:
            The envelope of latlong-points cannot be implemented as for xy-points. Suppose the
            envelope of the Aleutian Islands must be calculated. The span from 170E to 170W, from -170 to 170.
            Of course the real envelope is not -170..170 but 170..-170.
            On the other hand, there might be geometries that indeed span from -170 to 170. If there are
            two points, it is not known. If there are points in between, we probably should take the shorter
            range. So we do that for the moment.
            We shift coordinates and do as if we live in a world with longitude coordinates from 0 - 360,
            where 0 is still at Greenwich. Longitude ranges are then calculated twice: one for real world,
            one for the shifted world.
            The shortest range is taken for the bounding box. This might have coordinates > 180
        */

        template <typename P, typename B>
        struct grow_ll
        {

            struct state
            {
                typedef typename coordinate_type<B>::type T;
                bool has_west;
                T min_lat, max_lat;
                T min_lon1, min_lon2, max_lon1, max_lon2;
                B& mbr;

                state(B& b)
                    : mbr(b)
                    , has_west(false)
                    , min_lat(boost::numeric::bounds<T>::highest())
                    , min_lon1(boost::numeric::bounds<T>::highest())
                    , min_lon2(boost::numeric::bounds<T>::highest())
                    , max_lat(boost::numeric::bounds<T>::lowest())
                    , max_lon1(boost::numeric::bounds<T>::lowest())
                    , max_lon2(boost::numeric::bounds<T>::lowest())
                {}

                template <typename T>
                void take_minmax(const T& value, T& min_value, T& max_value)
                {
                    if (value < min_value)
                    {
                        min_value = value;
                    }
                    if (value > max_value)
                    {
                        max_value = value;
                    }
                }

                void grow(const P& p)
                {
                    // For latitude, we can take the min/max
                    take_minmax(get<1>(p), min_lat, max_lat);


                    // For longitude, we do the same...
                    take_minmax(get<0>(p), min_lon1, max_lon1);

                    // But we also add 360 (2pi) if it is negative
                    T value = get<0>(p);
                    while(value < 0)
                    {
                        has_west = true;
                        value += shift::shifted<typename coordinate_system<P>::type::units>::shift();
                    }
                    while (value > math::two_pi)
                    {
                        value -= shift::shifted<typename coordinate_system<P>::type::units>::shift();
                    }
                    take_minmax(value, min_lon2, max_lon2);
                }

                ~state()
                //void envelope(box<PB>& mbr)
                {
                    // For latitude it is easy
                    set<min_corner, 1>(mbr, min_lat);
                    set<max_corner, 1>(mbr, max_lat);

                    if (! has_west)
                    {
                        set<min_corner, 0>(mbr, min_lon1);
                        set<max_corner, 0>(mbr, max_lon1);
                    }
                    else
                    {
                        // Get both ranges
                        T diff1 = max_lon1 - min_lon1;
                        T diff2 = max_lon2 - min_lon2;

                        //std::cout << "range 1: " << min_lon1 * math::r2d << ".." << max_lon1 * math::r2d << std::endl;
                        //std::cout << "range 2: " << min_lon2 * math::r2d << ".." << max_lon2 * math::r2d << std::endl;

                        if (diff1 <= diff2)
                        {
                            set<min_corner, 0>(mbr, min_lon1);
                            set<max_corner, 0>(mbr, max_lon1);
                        }
                        else
                        {
                            set<min_corner, 0>(mbr, min_lon2);
                            set<max_corner, 0>(mbr, max_lon2);
                        }
                    }
                }
            };

            typedef state state_type;

            void operator()(const P& p, state_type& s) const
            {
                s.grow(p);
            }
        };
    } // namespace envelope
} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P, typename B>
struct strategy_envelope<geographic_tag, geographic_tag, P, B>
{
    typedef strategy::envelope::grow_ll<P, B>  type;
};
#endif

} // namespace ggl

#endif // GGL_GEOMETRY_STRATEGIES_SPHERICAL_SPH_ENVELOPE_HPP
