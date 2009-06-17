// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_GEOGRAPHIC_DISTANCE_HPP
#define GGL_STRATEGY_GEOGRAPHIC_DISTANCE_HPP


#include <ggl/strategies/strategy_traits.hpp>
#include <ggl/strategies/distance_result.hpp>
#include <ggl/core/radian_access.hpp>

namespace ggl
{
namespace strategy
{

    namespace distance
    {


        /*!
            \brief Defines ellipsoid values for use in distance calculations
            \details They have a constructor with the earth radius
            \tparam P1 first point type
            \tparam P2 optional second point type
            \note Will be moved / merged with projections
            \todo Optionally specify earth model, defaulting to WGS84
            - See http://en.wikipedia.org/wiki/Figure_of_the_Earth
            - and http://en.wikipedia.org/wiki/World_Geodetic_System#A_new_World_Geodetic_System:_WGS84
            \note
        */
        class ellipsoid
        {
            public :
                ellipsoid(double a, double b)
                    : m_a(a)
                    , m_b(b)
                    , m_f((a - b) / a)
                {}
                ellipsoid()
                    : m_a(6378137.0)
                    , m_b(6356752.314245)
                    , m_f((m_a - m_b) / m_a)
                {}
                // Unit sphere
                ellipsoid(double f)
                    : m_a(1.0)
                    , m_f(f)
                {}

                double a() const { return m_a; }
                double b() const { return m_b; }
                double f() const { return m_f; }

            private :
                double m_a, m_b, m_f; // equatorial radius, polar radius, flattening
        };


        /*!
            \brief Point-point distance approximation taking flattening into account
            \ingroup distance
            \tparam P1 first point type
            \tparam P2 optional second point type
            \author After Andoyer, 19xx, republished 1950, republished by Meeus, 1999
            \note Although not so well-known, the approximation is very good: in all cases the results
            are about the same as Vincenty. In my (Barend's) testcases the results didn't differ more than 6 m
            \see http://nacc.upc.es/tierra/node16.html
            \see http://sci.tech-archive.net/Archive/sci.geo.satellite-nav/2004-12/2724.html
            \see http://home.att.net/~srschmitt/great_circle_route.html (implementation)
            \see http://www.codeguru.com/Cpp/Cpp/algorithms/article.php/c5115 (implementation)
            \see http://futureboy.homeip.net/frinksamp/navigation.frink (implementation)
            \see http://www.voidware.com/earthdist.htm (implementation)
        */
        template <typename P1, typename P2 = P1>
        class andoyer
        {
            public :
                //typedef spherical_distance return_type;
                typedef double return_type;

                andoyer()
                    : m_ellipsoid()
                {}
                andoyer(double f)
                    : m_ellipsoid(f)
                {}

                inline return_type operator()(const P1& p1, const P2& p2) const
                {
                    return calc(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                    get_as_radian<0>(p2), get_as_radian<1>(p2));
                }


            private :
                typedef typename coordinate_type<P1>::type T1;
                typedef typename coordinate_type<P2>::type T2;
                ellipsoid m_ellipsoid;

                inline return_type calc(const T1& lon1, const T1& lat1, const T2& lon2, const T2& lat2) const
                {
                    double F = (lat1 + lat2) / 2.0;
                    double G = (lat1 - lat2) / 2.0;
                    double lambda = (lon1 - lon2) / 2.0;

                    double sinG2 = math::sqr(sin(G));
                    double cosG2 = math::sqr(cos(G));
                    double sinF2 = math::sqr(sin(F));
                    double cosF2 = math::sqr(cos(F));
                    double sinL2 = math::sqr(sin(lambda));
                    double cosL2 = math::sqr(cos(lambda));

                    double S = sinG2 * cosL2 + cosF2 * sinL2;
                    double C = cosG2 * cosL2 + sinF2 * sinL2;

                    double omega = atan(sqrt(S / C));
                    double r = sqrt(S * C) / omega; // not sure if this is r or greek nu

                    double D = 2.0 * omega * m_ellipsoid.a();
                    double H1 = (3 * r - 1.0) / (2.0 * C);
                    double H2 = (3 * r + 1.0) / (2.0 * S);

                    return return_type(D
                        * (1.0 + m_ellipsoid.f() * H1 * sinF2 * cosG2
                                    - m_ellipsoid.f() * H2 * cosF2 * sinG2));
                }
        };


        /*!
            \brief Distance calculation formulae on latlong coordinates, after Vincenty, 1975
            \ingroup distance
            \tparam P1 first point type
            \tparam P2 optional second point type
            \author See http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
            \author Adapted from various implementations to get it close to the original document
                - http://www.movable-type.co.uk/scripts/LatLongVincenty.html
                - http://exogen.case.edu/projects/geopy/source/geopy.distance.html
                - http://futureboy.homeip.net/fsp/colorize.fsp?fileName=navigation.frink

        */
        template <typename P1, typename P2 = P1>
        class vincenty
        {
            public :
                //typedef spherical_distance return_type;
                typedef double return_type;

                inline return_type operator()(const P1& p1, const P2& p2) const
                {
                    return calc(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                    get_as_radian<0>(p2), get_as_radian<1>(p2));
                }

            private :
                typedef typename coordinate_type<P1>::type T1;
                typedef typename coordinate_type<P2>::type T2;
                ellipsoid m_ellipsoid;

                inline return_type calc(const T1& lon1, const T1& lat1, const T2& lon2, const T2& lat2) const
                {
                    // lambda: difference in longitude on an auxiliary sphere
                    double L = lon2 - lon1;
                    double lambda = L;

                    if (L < -math::pi) L += math::two_pi;
                    if (L > math::pi) L -= math::two_pi;

                    if (lat1 == lat2 && lon1 == lon2)
                    {
                      return return_type(0);
                    }

                    // U: reduced latitude, defined by tan U = (1-f) tan phi
                    double U1 = atan((1-m_ellipsoid.f()) * tan(lat1)); // above (1)
                    double U2 = atan((1-m_ellipsoid.f()) * tan(lat2)); // above (1)

                    double cos_U1 = cos(U1);
                    double cos_U2 = cos(U2);
                    double sin_U1 = sin(U1);
                    double sin_U2 = sin(U2);

                    // alpha: azimuth of the geodesic at the equator
                    double cos2_alpha;
                    double sin_alpha;

                    // sigma: angular distance p1,p2 on the sphere
                    // sigma1: angular distance on the sphere from the equator to p1
                    // sigma_m: angular distance on the sphere from the equator to the midpoint of the line
                    double sigma;
                    double sin_sigma;
                    double cos2_sigma_m;

                    double previous_lambda;

                    do
                    {
                        previous_lambda = lambda; // (13)
                        double sin_lambda = sin(lambda);
                        double cos_lambda = cos(lambda);
                        sin_sigma = sqrt(math::sqr(cos_U2 * sin_lambda) + math::sqr(cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda)); // (14)
                        double cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda; // (15)
                        sin_alpha = cos_U1 * cos_U2 * sin_lambda / sin_sigma; // (17)
                        cos2_alpha = 1.0 - math::sqr(sin_alpha);
                        cos2_sigma_m = cos2_alpha == 0 ? 0 : cos_sigma - 2.0 * sin_U1 * sin_U2 / cos2_alpha; // (18)

                        double C = m_ellipsoid.f()/16.0 * cos2_alpha * (4.0 + m_ellipsoid.f() * (4.0 - 3.0 * cos2_alpha)); // (10)
                        sigma = atan2(sin_sigma, cos_sigma); // (16)
                        lambda = L + (1.0 - C) * m_ellipsoid.f() * sin_alpha *
                            (sigma + C * sin_sigma * ( cos2_sigma_m + C * cos_sigma * (-1.0 + 2.0 * math::sqr(cos2_sigma_m)))); // (11)

                    } while (fabs(previous_lambda - lambda) > 1e-12 && fabs(lambda) < math::pi);

                    double sqr_u = cos2_alpha * (math::sqr(m_ellipsoid.a()) - math::sqr(m_ellipsoid.b())) / math::sqr(m_ellipsoid.b()); // above (1)

                    double A = 1.0 + sqr_u/16384.0 * (4096 + sqr_u * (-768.0 + sqr_u * (320.0 - 175.0 * sqr_u))); // (3)
                    double B = sqr_u/1024.0 * (256.0 + sqr_u * ( -128.0 + sqr_u * (74.0 - 47.0 * sqr_u))); // (4)
                    double delta_sigma = B * sin_sigma * ( cos2_sigma_m + (B/4.0) * (cos(sigma)* (-1.0 + 2.0 * cos2_sigma_m)
                            - (B/6.0) * cos2_sigma_m * (-3.0 + 4.0 * math::sqr(sin_sigma)) * (-3.0 + 4.0 * cos2_sigma_m))); // (6)

                    double dist = m_ellipsoid.b() * A * (sigma - delta_sigma); // (19)

                    return return_type(dist);
                }
        };


        // We might add a vincenty-like strategy also for point-segment distance, but to calculate the projected point is not trivial

    } // namespace distance


} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P1, typename P2>
struct strategy_distance<geographic_tag, geographic_tag, P1, P2>
{
    typedef strategy::distance::andoyer<P1, P2> type;
};


template <typename P1, typename P2>
struct strategy_tag<strategy::distance::andoyer<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};
template <typename P1, typename P2>
struct strategy_tag<strategy::distance::vincenty<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};



#endif


} // namespace ggl


#endif // GGL_STRATEGY_GEOGRAPHIC_DISTANCE_HPP
