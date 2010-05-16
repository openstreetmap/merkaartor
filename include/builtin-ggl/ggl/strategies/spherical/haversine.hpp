// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_SPHERICAL_DISTANCE_HPP
#define GGL_STRATEGY_SPHERICAL_DISTANCE_HPP


#include <ggl/core/cs.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/access.hpp>
#include <ggl/core/radian_access.hpp>


#include <ggl/strategies/strategy_traits.hpp>

#include <ggl/strategies/distance_result.hpp>

#include <ggl/util/get_cs_as_radian.hpp>



namespace ggl
{
namespace strategy
{

    namespace distance
    {

        /*!
            \brief Distance calculation for spherical coordinates on a perfect sphere using haversine
            \ingroup distance
            \tparam P1 first point type
            \tparam P2 optional second point type
            \author Adapted from: http://williams.best.vwh.net/avform.htm
            \see http://en.wikipedia.org/wiki/Great-circle_distance
            \note It says: <em>The great circle distance d between two points with coordinates {lat1,lon1} and {lat2,lon2} is given by:
                        d=acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2))
                    A mathematically equivalent formula, which is less subject to rounding error for short distances is:
                        d=2*asin(sqrt((sin((lat1-lat2)/2))^2 + cos(lat1)*cos(lat2)*(sin((lon1-lon2)/2))^2))</em>
        */
        template <typename P1, typename P2 = P1>
        class haversine
        {
            public :
                //typedef spherical_distance return_type;
                typedef double return_type;

                inline haversine(double r = 1.0)
                    : m_radius(r)
                {}

                inline return_type operator()(const P1& p1, const P2& p2) const
                {
                    return calc(get_as_radian<0>(p1), get_as_radian<1>(p1),
                                    get_as_radian<0>(p2), get_as_radian<1>(p2));
                }

            private :
                double m_radius;
                typedef typename coordinate_type<P1>::type T1;
                typedef typename coordinate_type<P2>::type T2;

                inline return_type calc(const T1& lon1, const T1& lat1, const T2& lon2, const T2& lat2) const
                {
                    double a = math::hav(lat2 - lat1) + cos(lat1) * cos(lat2) * math::hav(lon2 - lon1);
                    double c = 2.0 * asin(sqrt(a));
                    return return_type(m_radius * c);
                }
        };



        /*!
            \brief Strategy functor for distance point to segment calculation
            \ingroup distance
            \details Class which calculates the distance of a point to a segment, using latlong points
            \tparam P point type
            \tparam S segment type
        */
        template <typename P, typename S>
        class ll_point_segment
        {
            public :
                typedef double return_type;

                inline ll_point_segment(double r = 1.0) : m_radius(r)
                {}

                inline return_type operator()(P const& p, S const& s) const
                {
                    PR pr, ps1, ps2;

                    // Select transformation strategy and transform to radians (if necessary)
                    typename strategy_transform<
                                typename cs_tag<P>::type,
                                typename cs_tag<PR>::type,
                                typename coordinate_system<P>::type,
                                typename coordinate_system<PR>::type,
                                dimension<P>::value,
                                dimension<PR>::value,
                                P, PR>::type transform_strategy;


                    // TODO
                    // ASSUMPTION: segment
                    // SOLVE THIS USING OTHER FUNCTIONS using get<,>
                    transform_strategy(p, pr);
                    transform_strategy(s.first, ps1);
                    transform_strategy(s.second, ps2);
                    return calc(pr, ps1, ps2);
                }

            private :
                typedef point
                    <
                        typename coordinate_type<P>::type,
                        ggl::dimension<P>::type::value,
                        typename ggl::detail::get_cs_as_radian
                            <
                                typename coordinate_system<P>::type
                            >::type
                    > PR;
                double m_radius;

                /// Calculate course (bearing) between two points. Might be moved to a "course formula" ...
                inline double course(PR const& p1, PR const& p2) const
                {
                    /***
                        Course between points

                        We obtain the initial course, tc1, (at point 1) from point 1 to point 2 by the following. The formula fails if the initial point is a pole. We can special case this with:

                        IF (cos(lat1) < EPS)   // EPS a small number ~ machine precision
                          IF (lat1 > 0): tc1= pi        //  starting from N pole
                          ELSE: tc1= 2*pi         //  starting from S pole
                          ENDIF
                        ENDIF

                        For starting points other than the poles:
                        IF sin(lon2-lon1)<0: tc1=acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1)))
                        ELSE: tc1=2*pi-acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1)))
                        ENDIF

                        An alternative formula, not requiring the pre-computation of d, the distance between the points, is:
                           tc1=mod(atan2(sin(lon1-lon2)*cos(lat2),
                                   cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon1-lon2))
                                   , 2*pi)
                     ***/
                    double dlon = get<0>(p2) - get<0>(p1);
                    double cos_p2lat = cos(get<1>(p2));
                    return atan2(sin(dlon) * cos_p2lat,
                        cos(get<1>(p1)) * sin(get<1>(p2))
                        - sin(get<1>(p1)) * cos_p2lat * cos(dlon));
                }

                inline return_type calc(PR const& p, PR const& sp1, PR const& sp2) const
                {
                    /***
                    Cross track error:
                    Suppose you are proceeding on a great circle route from A to B (course =crs_AB) and end up at D, perhaps off course.
                    (We presume that A is ot a pole!) You can calculate the course from A to D (crs_AD) and the distance from A to D (dist_AD)
                    using the formulae above. In shifteds of these the cross track error, XTD, (distance off course) is given by

                               XTD =asin(sin(dist_AD)*sin(crs_AD-crs_AB))

                    (positive XTD means right of course, negative means left)
                    (If the point A is the N. or S. Pole replace crs_AD-crs_AB with
                    lon_D-lon_B or lon_B-lon_D, respectively.)
                     ***/

                    // Calculate distances, in radians, on the unit sphere
                    // It seems not useful to let this strategy be templatized, it should be in radians and on the unit sphere
                    strategy::distance::haversine<PR, PR> strategy(1.0);
                    double d1 = strategy(sp1, p);

                    // Actually, calculation of d2 not necessary if we know that the projected point is on the great circle...
                    double d2 = strategy(sp2, p);

                    // Source: http://williams.best.vwh.net/avform.htm

                    double crs_AD = course(sp1, p);
                    double crs_AB = course(sp1, sp2);
                    double XTD = fabs(asin(sin(d1) * sin(crs_AD - crs_AB)));

                    // Return shortest distance, either to projected point on segment sp1-sp2, or to sp1, or to sp2
                    return return_type(m_radius * (std::min)((std::min)(d1, d2), XTD));
                }
        };





    } // namespace distance




} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P1, typename P2>
struct strategy_distance<spherical_tag, spherical_tag, P1, P2>
{
    typedef strategy::distance::haversine<P1, P2> type;
};


template <typename Point, typename Segment>
struct strategy_distance_segment<spherical_tag, spherical_tag, Point, Segment>
{
    typedef strategy::distance::ll_point_segment<Point, Segment> type;
};


// Use this point-segment for geographic as well. TODO: change this, extension!
template <typename Point, typename Segment>
struct strategy_distance_segment<geographic_tag, geographic_tag, Point, Segment>
{
    typedef strategy::distance::ll_point_segment<Point, Segment> type;
};




template <typename P1, typename P2>
struct strategy_tag<strategy::distance::haversine<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};

template <typename Point, typename Segment>
struct strategy_tag<strategy::distance::ll_point_segment<Point, Segment> >
{
    typedef strategy_tag_distance_point_segment type;
};



#endif






} // namespace ggl


#endif // GGL_STRATEGY_SPHERICAL_DISTANCE_HPP
