// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_CENTROID_HPP
#define GGL_STRATEGY_CARTESIAN_CENTROID_HPP


#include <boost/mpl/if.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_traits.hpp>

#include <ggl/geometries/point_xy.hpp>
#include <ggl/geometries/segment.hpp>

#include <ggl/algorithms/assign.hpp>

#include <ggl/util/select_coordinate_type.hpp>
#include <ggl/util/copy.hpp>


namespace ggl
{

namespace strategy
{
    namespace centroid
    {
        /*!
            \brief Centroid calculation
            \details Geolib original version,
            \par Template parameters and concepts: see bashein_detmer
            \author Barend and Maarten, 1995/1996
            \author Revised for templatized library, Barend Gehrels, 2007
            \note The results are slightly different from Bashein/Detmer, so probably slightly wrong.
        */

        template<typename PC, typename PS = PC>
        class geolib1995
        {
            private  :
                typedef typename select_most_precise
                    <
                        typename select_coordinate_type<PC, PS>::type,
                        double
                    >::type T;

                /*! subclass to keep state */
                struct sums
                {
                     PC sum_ms, sum_m;

                    sums()
                    {
                        assign_point(sum_m, T());
                        assign_point(sum_ms, T());
                    }
                    void centroid(PC& point)
                    {
                        point = sum_ms;

                        if (get<0>(sum_m) != 0 && get<1>(sum_m) != 0)
                        {
                            multiply_value(point, 0.5);
                            divide_point(point, sum_m);
                        }
                        else
                        {
                            // exception?
                        }
                    }
                };

            public :
                typedef sums state_type;
                inline bool operator()(const segment<const PS>& s, state_type& state) const
                {
                    /* Algorithm:
                    For each segment:
                    begin
                        dx = x2 - x1;
                        dy = y2 - y1;
                        sx = x2 + x1;
                        sy = y2 + y1;
                        mx = dx * sy;
                        my = sx * dy;

                        sum_mx += mx;
                        sum_my += my;
                        sum_msx += mx * sx;
                        sum_msy += my * sy;
                    end;
                    return POINT(0.5 * sum_msx / sum_mx, 0.5 * sum_msy / sum_my);
                    */

                    PS diff = s.second, sum = s.second;
                    subtract_point(diff, s.first);
                    add_point(sum, s.first);

                    // We might create an arithmatic operation for this.
                    PS m;
                    get<0>(m) = get<0>(diff) * get<1>(sum);
                    get<1>(m) = get<0>(sum) * get<1>(diff);

                    add_point(state.sum_m, m);
                    multiply_point(m, sum);
                    add_point(state.sum_ms, m);

                    return true;
                }

        };


        /*!
            \brief Centroid calculation using algorith Bashein / Detmer
            \details Calculates centroid using triangulation method published by Bashein / Detmer
            \tparam PC point type of centroid to calculate
            \tparam PS point type of segments, defaults to PC
            \par Concepts for PC and PS:
            - specialized point_traits class
            \author Adapted from  "Centroid of a Polygon" by Gerard Bashein and Paul R. Detmer<em>,
            in "Graphics Gems IV", Academic Press, 1994</em>
            \par Research notes
            The algorithm gives the same results as Oracle and PostGIS but differs from MySQL
            (tried 5.0.21 / 5.0.45 / 5.0.51a / 5.1.23).

            Without holes:
            - this:       POINT(4.06923363095238 1.65055803571429)
            - geolib:     POINT(4.07254 1.66819)
            - MySQL:      POINT(3.6636363636364  1.6272727272727)'
            - PostGIS:    POINT(4.06923363095238 1.65055803571429)
            - Oracle:           4.06923363095238 1.65055803571429
            - SQL Server: POINT(4.06923362245959 1.65055804168294)

            Statements:
            - \b MySQL/PostGIS: select AsText(Centroid(GeomFromText('POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2
                            ,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))')))
            - \b Oracle: select sdo_geom.sdo_centroid(sdo_geometry(2003, null, null,
                    sdo_elem_info_array(1, 1003, 1), sdo_ordinate_array(2,1.3,2.4,1.7,2.8,1.8
                    ,3.4,1.2,3.7,1.6,3.4,2,4.1,3,5.3,2.6,5.4,1.2,4.9,0.8,2.9,0.7,2,1.3))
                    , mdsys.sdo_dim_array(mdsys.sdo_dim_element('x',0,10,.00000005)
                    ,mdsys.sdo_dim_element('y',0,10,.00000005)))
                    from dual
            - \b SQL Server 2008: select geometry::STGeomFromText('POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2
                            ,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))',0)
                            .STCentroid()
                            .STAsText()

            With holes:
            - this:       POINT(4.04663 1.6349)
            - geolib:     POINT(4.04675 1.65735)
            - MySQL:      POINT(3.6090580503834 1.607573932092)
            - PostGIS:    POINT(4.0466265060241 1.63489959839357)
            - Oracle:           4.0466265060241 1.63489959839357
            - SQL Server: POINT(4.0466264962959677 1.6348996057331333)

            Statements:
            - \b MySQL/PostGIS: select AsText(Centroid(GeomFromText('POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2
                    ,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)
                    ,(4 2,4.2 1.4,4.8 1.9,4.4 2.2,4 2))')));
            - \b Oracle: select sdo_geom.sdo_centroid(sdo_geometry(2003, null, null
                    , sdo_elem_info_array(1, 1003, 1, 25, 2003, 1)
                    , sdo_ordinate_array(2,1.3,2.4,1.7,2.8,1.8,3.4,1.2,3.7,1.6,3.4,2,4.1,3,5.3
                    ,2.6,5.4,1.2,4.9,0.8,2.9,0.7,2,1.3,4,2, 4.2,1.4, 4.8,1.9, 4.4,2.2, 4,2))
                    , mdsys.sdo_dim_array(mdsys.sdo_dim_element('x',0,10,.00000005)
                    ,mdsys.sdo_dim_element('y',0,10,.00000005)))
                    from dual
         */
        template
        <
            typename CentroidPointType,
            typename SegmentPointType = CentroidPointType,
            typename CalculationType = void
        >
        class bashein_detmer
        {
            private :
                // If user specified a calculation type, use that type,
                //   whatever it is and whatever the point-type(s) are.
                // Else, use the most appropriate coordinate type
                //    of the two points, but at least double
                typedef typename
                    boost::mpl::if_c
                    <
                        boost::is_void<CalculationType>::type::value,
                        typename select_most_precise
                        <
                            typename select_coordinate_type
                                <
                                    CentroidPointType,
                                    SegmentPointType
                                >::type,
                            double
                        >::type,
                        CalculationType
                    >::type calc_type;

                /*! subclass to keep state */
                struct sums
                {
                    calc_type sum_a2;
                    calc_type sum_x;
                    calc_type sum_y;
                    inline sums()
                        : sum_a2(calc_type())
                        , sum_x(calc_type())
                        , sum_y(calc_type())
                    {
                        typedef calc_type ct;
                        //std::cout << "-> calctype: " << typeid(ct).name()
                        //    << " size: " << sizeof(ct)
                        //    << " init: " << sum_a2
                        //    << std::endl;
                    }

                    inline void centroid(CentroidPointType& point)
                    {
                        if (sum_a2 != 0)
                        {
                            calc_type const v3 = 3;
                            calc_type const a3 = v3 * sum_a2;

                            typedef typename ggl::coordinate_type
                                <
                                    CentroidPointType
                                >::type coordinate_type;

                            set<0>(point,
                                boost::numeric_cast<coordinate_type>(sum_x / a3));
                            set<1>(point,
                                boost::numeric_cast<coordinate_type>(sum_y / a3));
                        }
                        else
                        {
                            // exception?
                        }
                    }

                };

            public :
                typedef sums state_type;

                inline bool operator()(segment<const SegmentPointType> const& s, state_type& state) const
                {
                    /* Algorithm:
                    For each segment:
                    begin
                        ai = x1 * y2 - x2 * y1;
                        sum_a2 += ai;
                        sum_x += ai * (x1 + x2);
                        sum_y += ai * (y1 + y2);
                    end
                    return POINT(sum_x / (3 * sum_a2), sum_y / (3 * sum_a2) )
                    */

                    // Get coordinates and promote them to calc_type
                    calc_type const x1 = boost::numeric_cast<calc_type>(get<0, 0>(s));
                    calc_type const y1 = boost::numeric_cast<calc_type>(get<0, 1>(s));
                    calc_type const x2 = boost::numeric_cast<calc_type>(get<1, 0>(s));
                    calc_type const y2 = boost::numeric_cast<calc_type>(get<1, 1>(s));
                    calc_type const ai = x1 * y2 - x2 * y1;
                    state.sum_a2 += ai;
                    state.sum_x += ai * (x1 + x2);
                    state.sum_y += ai * (y1 + y2);

                    return true;
                }

        };
    } // namespace centroid

} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P, typename PS>
struct strategy_centroid<cartesian_tag, P, PS>
{
    typedef strategy::centroid::bashein_detmer<P, PS> type;
};
#endif

} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_CENTROID_HPP
