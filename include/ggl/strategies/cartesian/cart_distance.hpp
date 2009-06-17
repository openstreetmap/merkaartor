// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_DISTANCE_HPP
#define GGL_STRATEGY_CARTESIAN_DISTANCE_HPP



#include <boost/type_traits/remove_const.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/point_type.hpp>

#include <ggl/arithmetic/arithmetic.hpp>
#include <ggl/arithmetic/dot_product.hpp>

#include <ggl/strategies/strategy_traits.hpp>
#include <ggl/strategies/distance_result.hpp>

#include <ggl/util/promotion_traits.hpp>
#include <ggl/util/copy.hpp>


// Helper geometry
#include <ggl/geometries/segment.hpp>

namespace ggl {

namespace strategy { namespace distance {

#ifndef DOXYGEN_NO_IMPL
namespace impl
{

template <typename P1, typename P2, size_t I, typename T>
struct compute_pythagoras
{
    static T result(const P1& p1, const P2& p2)
    {
        T d = get<I-1>(p2) - get<I-1>(p1);
        return d*d + compute_pythagoras<P1, P2, I-1, T>::result(p1, p2);
    }
};

template <typename P1, typename P2, typename T>
struct compute_pythagoras<P1, P2, 0, T>
{
    static T result(const P1&, const P2&)
    {
        return 0;
    }
};

}
#endif

/*!
    \brief Strategy for distance point to point: pythagoras
    \ingroup distance
    \tparam P1 first point type
    \tparam P2 optional, second point type, defaults to first point type
    \par Concepts for P1 and P2:
    - specialized point_traits class
*/
template <typename P1, typename P2 = P1>
struct pythagoras
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;
    typedef cartesian_distance return_type;


    inline BOOST_CONCEPT_REQUIRES(((concept::ConstPoint<P1>))
    ((concept::ConstPoint<P2>)), (cartesian_distance))
    operator()(const P1& p1, const P2& p2) const
    {
        // Calculate distance using Pythagoras
        // (Leave comment above for Doxygen)

        assert_dimension_equal<P1, P2>();

        return cartesian_distance(impl::compute_pythagoras
            <
                P1, P2,
                dimension<P1>::value,
                coordinate_type
            >::result(p1, p2));
    }
};


/*!
    \brief Strategy for distance point to segment
    \ingroup distance
    \details Calculates distance using projected-point method, and (optionally) Pythagoras
    \author Adapted from: http://geometryalgorithms.com/Archive/algorithm_0102/algorithm_0102.htm
    \tparam P point type
    \tparam SEG segment type
    \tparam S strategy, optional, defaults to pythagoras
    \par Concepts for S:
    - cartesian_distance operator(P,P)
*/
template
<
    typename P,
    typename Segment,
    typename Strategy = pythagoras<P, typename point_type<Segment>::type>
>
struct xy_point_segment
{
    typedef typename select_coordinate_type<P, Segment>::type coordinate_type;
    typedef cartesian_distance return_type;
    typedef Strategy distance_strategy_type; // OBSOLETE!
    typedef Strategy point_strategy_type;


    inline cartesian_distance operator()(const P& p, const Segment& s) const
    {
        assert_dimension_equal<P, Segment>();

        /* Algorithm
        POINT v(x2 - x1, y2 - y1);
        POINT w(px - x1, py - y1);
        c1 = w . v
        c2 = v . v
        b = c1 / c2
        RETURN POINT(x1 + b * vx, y1 + b * vy);
        */

        typedef typename boost::remove_const
        <
            typename point_type<Segment>::type
        >::type segment_point_type;

        segment_point_type v, w;

        // TODO
        // ASSUMPTION: segment
        // SOLVE THIS USING OTHER FUNCTIONS using get<,>
        copy_coordinates(s.second, v);
        copy_coordinates(p, w);
        subtract_point(v, s.first);
        subtract_point(w, s.first);

        Strategy strategy;

        coordinate_type c1 = dot_product(w, v);
        if (c1 <= 0)
        {
            return strategy(p, s.first);
        }
        coordinate_type c2 = dot_product(v, v);
        if (c2 <= c1)
        {
            return strategy(p, s.second);
        }

        // Even in case of char's, we have to turn to a point<double/float>
        // because of the division.
        coordinate_type b = c1 / c2;

        // Note that distances with integer coordinates do NOT work because
        // - the project point is integer
        // - if we solve that, the used distance_strategy cannot handle double points
        segment_point_type projected;
        copy_coordinates(s.first, projected);
        multiply_value(v, b);
        add_point(projected, v);

        return strategy(p, projected);

    }
};

}} // namespace strategy::distance



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P1, typename P2>
struct strategy_distance<cartesian_tag, cartesian_tag, P1, P2>
{
    typedef strategy::distance::pythagoras<P1, P2> type;
};

template <typename Point, typename Segment>
struct strategy_distance_segment<cartesian_tag, cartesian_tag, Point, Segment>
{
    typedef typename point_type<Segment>::type segment_point_type;

    typedef strategy::distance::xy_point_segment
    <
        Point,
        Segment,
        typename strategy_distance
        <
            cartesian_tag, cartesian_tag, Point, segment_point_type
        >::type
    > type;
};
#endif


template <typename P1, typename P2>
struct strategy_tag<strategy::distance::pythagoras<P1, P2> >
{
    typedef strategy_tag_distance_point_point type;
};

template <typename Point, typename Segment, typename PPStrategy>
struct strategy_tag<strategy::distance::xy_point_segment<Point, Segment, PPStrategy> >
{
    typedef strategy_tag_distance_point_segment type;
};



} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_DISTANCE_HPP
