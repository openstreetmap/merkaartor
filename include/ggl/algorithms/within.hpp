// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ALGORITHMS_WITHIN_HPP
#define GGL_ALGORITHMS_WITHIN_HPP

#include <boost/concept/requires.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/distance.hpp>
#include <ggl/algorithms/make.hpp>
#include <ggl/core/access.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/concepts/nsphere_concept.hpp>
#include <ggl/core/cs.hpp>
#include <ggl/strategies/strategies.hpp>
#include <ggl/util/loop.hpp>




namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace within {


//-------------------------------------------------------------------------------------------------------
// Implementation for boxes. Supports boxes in 2 or 3 dimensions, in Euclidian system
// Todo: implement as strategy
//-------------------------------------------------------------------------------------------------------
template <typename P, typename B, size_t D, size_t N>
struct point_in_box
{
    static bool inside(P const& p, B const& b)
    {
        if (get<D>(p) <= get<min_corner, D>(b)
            || get<D>(p) >= get<max_corner, D>(b))
        {
            return false;
        }

        return point_in_box<P, B, D + 1, N>::inside(p, b);
    }
};

template <typename P, typename B, size_t N>
struct point_in_box<P, B, N, N>
{
    static bool inside(P const& p, B const& b)
    {
        return true;
    }
};

//-------------------------------------------------------------------------------------------------------
// Box-in-box, for 2/3 dimensions
//-------------------------------------------------------------------------------------------------------
template <typename B1, typename B2, size_t D, size_t N>
struct box_in_box
{
    static inline bool inside(B1 const& b1, B2 const& b2)
    {
        if (get<min_corner, D>(b1) <= get<min_corner, D>(b2)
            || get<max_corner, D>(b1) >= get<max_corner, D>(b2))
        {
            return false;
        }

        return box_in_box<B1, B2, D + 1, N>::inside(b1, b2);
    }
};

template <typename B1, typename B2, size_t N>
struct box_in_box<B1, B2, N, N>
{
    static inline bool inside(B1 const&, B2 const& b2)
    {
        return true;
    }
};


//-------------------------------------------------------------------------------------------------------
// Implementation for n-spheres. Supports circles or spheres, in 2 or 3 dimensions, in Euclidian system
// Circle center might be of other point-type as geometry
// Todo: implement as strategy
//-------------------------------------------------------------------------------------------------------
template<typename P, typename C>
inline bool point_in_circle(P const& p, C const& c)
{
    assert_dimension<C, 2>();

    typedef typename point_type<C>::type point_type;
    typedef typename strategy_distance
        <
            typename cs_tag<P>::type,
            typename cs_tag<point_type>::type,
            P,
            point_type
        >::type strategy_type;
    typedef typename strategy_type::return_type return_type;

    P const center = ggl::make<P>(get<0>(c), get<1>(c));
    strategy_type distance;
    return_type const r = distance(p, center);
    return_type const rad = make_distance_result<return_type>(get_radius<0>(c));

    return r < rad;
}
/// 2D version
template<typename T, typename C>
inline bool point_in_circle(const T& c1, const T& c2, C const& c)
{
    typedef typename point_type<C>::type point_type;

    point_type p = ggl::make<point_type>(c1, c2);
    return point_in_circle(p, c);
}

template<typename B, typename C>
inline bool box_in_circle(B const& b, C const& c)
{
    typedef typename point_type<B>::type point_type;

    // Currently only implemented for 2d geometries
    assert_dimension<point_type, 2>();
    assert_dimension<C, 2>();

    // Box: all four points must lie within circle

    // Check points lower-left and upper-right, then lower-right and upper-left
    return point_in_circle(get<min_corner, 0>(b), get<min_corner, 1>(b), c)
        && point_in_circle(get<max_corner, 0>(b), get<max_corner, 1>(b), c)
        && point_in_circle(get<min_corner, 0>(b), get<max_corner, 1>(b), c)
        && point_in_circle(get<max_corner, 0>(b), get<min_corner, 1>(b), c);
}

// Generic "range-in-circle", true if all points within circle
template<typename R, typename C>
inline bool range_in_circle(R const& range, C const& c)
{
    assert_dimension<R, 2>();
    assert_dimension<C, 2>();

    for (typename boost::range_const_iterator<R>::type it = boost::begin(range);
         it != boost::end(range); ++it)
    {
        if (! point_in_circle(*it, c))
        {
            return false;
        }
    }

    return true;
}

template<typename Y, typename C>
inline bool polygon_in_circle(Y const& poly, C const& c)
{
    return range_in_circle(exterior_ring(poly), c);
}

template<typename P, typename R, typename S>
inline bool point_in_ring(P const& p, R const& r, S const& strategy)
{
    if (boost::size(r) < 4)
    {
        return false;
    }

    typename S::state_type state(p);
    if (loop(r, strategy, state))
    {
        return state.within();
    }

    return false;
}

// Polygon: in exterior ring, and if so, not within interior ring(s)
template<typename P, typename Y, typename S>
inline bool point_in_polygon(P const& p, Y const& poly, S const& strategy)
{
    if (point_in_ring(p, exterior_ring(poly), strategy))
    {
        typedef typename boost::range_const_iterator
            <typename interior_type<Y>::type>::type iterator_type;

        for (iterator_type it = boost::begin(interior_rings(poly));
             it != boost::end(interior_rings(poly)); ++it)
        {
            if (point_in_ring(p, *it, strategy))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

}} // namespace detail::within
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag1, typename Tag2, typename G1, typename G2>
struct within {};

template <typename P, typename B>
struct within<point_tag, box_tag, P, B>
{
    static inline bool apply(P const& p, B const& b)
    {
        assert_dimension_equal<P, B>();

        return detail::within::point_in_box
            <
                P,
                B,
                0,
                dimension<P>::type::value
            >::inside(p, b);
    }
};

template <typename B1, typename B2>
struct within<box_tag, box_tag, B1, B2>
{
    static inline bool apply(B1 const& b1, B2 const& b2)
    {
        assert_dimension_equal<B1, B2>();

        return detail::within::box_in_box
            <
                B1,
                B2,
                0,
                dimension<B1>::type::value
            >::inside(b1, b2);
    }
};


template <typename P, typename C>
struct within<point_tag, nsphere_tag, P, C>
{
    static inline bool apply(P const& p, C const& c)
    {
        return detail::within::point_in_circle(p, c);
    }
};

template <typename B, typename C>
struct within<box_tag, nsphere_tag, B, C>
{
    static inline bool apply(B const& b, C const& c)
    {
        return detail::within::box_in_circle(b, c);
    }
};

template <typename R, typename C>
struct within<linestring_tag, nsphere_tag, R, C>
{
    static inline bool apply(R const& ln, C const& c)
    {
        return detail::within::range_in_circle(ln, c);
    }
};

template <typename R, typename C>
struct within<ring_tag, nsphere_tag, R, C>
{
    static inline bool apply(R const& r, C const& c)
    {
        return detail::within::range_in_circle(r, c);
    }
};

template <typename Y, typename C>
struct within<polygon_tag, nsphere_tag, Y, C>
{
    static inline bool apply(Y const& poly, C const& c)
    {
        return detail::within::polygon_in_circle(poly, c);
    }
};

template <typename P, typename R>
struct within<point_tag, ring_tag, P, R>
{
    static inline bool apply(P const& p, R const& r)
    {
        typedef typename boost::range_value<R>::type point_type;
        typedef typename strategy_within
            <
                typename cs_tag<P>::type,
                typename cs_tag<point_type>::type,
                P,
                point_type
            >::type strategy_type;

        return detail::within::point_in_ring(p, r, strategy_type());
    }
};

template <typename P, typename Y>
struct within<point_tag, polygon_tag, P, Y>
{
    static inline bool apply(P const& point, Y const& poly)
    {
        typedef typename point_type<Y>::type point_type;
        typedef typename strategy_within
            <
                typename cs_tag<P>::type,
                typename cs_tag<point_type>::type,
                P,
                point_type
            >::type strategy_type;

        return detail::within::point_in_polygon(point, poly, strategy_type());
    }

    template<typename S>
    static inline bool apply(P const& point, Y const& poly, S const& strategy)
    {
        return detail::within::point_in_polygon(point, poly, strategy);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Within check
    \details Examine if one geometry is within another geometry (a.o. point in polygon)
    \ingroup boolean_relations
    \param geometry1 geometry which might be within the second geometry
    \param geometry2 geometry which might contain the first geometry
    \return true if geometry1 is completely contained within geometry2, else false
    \note The default strategy is used for within detection

\par Source descriptions:
- OGC: Returns 1 (TRUE) if this geometric object is "spatially within" another Geometry.

\par Performance
2776 within determinations using bounding box and polygon are done in 0.09 seconds (other libraries: 0.14 seconds, 3.0 seconds, 3.8)

\par Example:
The within algorithm is used as following:
\dontinclude doxygen_examples.cpp
\skip example_within
\line {
\until }
\par Geometries:
- POINT + POLYGON: The well-known point-in-polygon, returning true if a point falls within a polygon (and not
within one of its holes) \image html within_polygon.png
- POINT + RING: returns true if point is completely within a ring \image html within_ring.png

 */
template<typename G1, typename G2>
inline bool within(G1 const& geometry1, G2 const& geometry2)
{
    typedef dispatch::within
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1,
            G2
        > within_type;

    return within_type::apply(geometry1, geometry2);
}

/*!
    \brief Within check using a strategy
    \ingroup boolean_relations
    \param geometry1 geometry which might be within the second geometry
    \param geometry2 geometry which might contain the first geometry
    \param strategy strategy to be used
    \return true if geometry1 is completely contained within geometry2, else false
 */
template<typename G1, typename G2, typename S>
inline bool within(G1 const& geometry1, G2 const& geometry2, S const& strategy)
{
    typedef dispatch::within
        <
            typename tag<G1>::type,
            typename tag<G2>::type,
            G1,
            G2
        > within_type;

    return within_type::apply(geometry1, geometry2, strategy);
}

} // namespace ggl

#endif // GGL_ALGORITHMS_WITHIN_HPP
