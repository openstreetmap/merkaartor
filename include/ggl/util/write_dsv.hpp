// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_WRITE_DSV_HPP
#define GGL_UTIL_WRITE_DSV_HPP

#include <iostream>
#include <string>

#include <boost/concept/assert.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <ggl/algorithms/convert.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/core/exterior_ring.hpp>
#include <ggl/core/interior_rings.hpp>
#include <ggl/core/ring_type.hpp>
#include <ggl/core/is_multi.hpp>
#include <ggl/geometries/linear_ring.hpp>



namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dsv {


struct dsv_settings
{
    std::string coordinate_separator;
    std::string point_open;
    std::string point_close;
    std::string point_separator;
    std::string list_open;
    std::string list_close;
    std::string list_separator;

    dsv_settings(std::string const& sep
            , std::string const& open
            , std::string const& close
            , std::string const& psep
            , std::string const& lopen
            , std::string const& lclose
            , std::string const& lsep
            )
        : coordinate_separator(sep)
        , point_open(open)
        , point_close(close)
        , point_separator(psep)
        , list_open(lopen)
        , list_close(lclose)
        , list_separator(lsep)
    {}
};

template <typename P, int Dimension, int Count>
struct stream_coordinate
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os, P const& p,
            dsv_settings const& settings)
    {
        os << (Dimension > 0 ? settings.coordinate_separator : "") 
            << get<Dimension>(p);
        stream_coordinate<P, Dimension + 1, Count>::apply(os, p, settings);
    }
};

template <typename P, int Count>
struct stream_coordinate<P, Count, Count>
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>&, P const&,
            dsv_settings const& settings)
    {}
};


template <typename P, int Index, int Dimension, int Count>
struct stream_box_corner
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os, P const& p,
            dsv_settings const& settings)
    {
        os << (Dimension > 0 ? settings.coordinate_separator : "") 
            << get<Index, Dimension>(p);
        stream_box_corner<P, Index, Dimension + 1, Count>::apply(os, p, settings);
    }
};

template <typename P, int Index, int Count>
struct stream_box_corner<P, Index, Count, Count>
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>&, P const&,
            dsv_settings const& settings)
    {}
};





/*!
\brief Stream points as \ref DSV
*/
template <typename Point>
struct dsv_point
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os, Point const& p,
            dsv_settings const& settings)
    {
        os << settings.point_open;
        stream_coordinate<Point, 0, dimension<Point>::type::value>::apply(os, p, settings);
        os << settings.point_close;
    }

    private:
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<Point>) );
};

/*!
\brief Stream ranges as DSV
\note policy is used to stream prefix/postfix, enabling derived classes to override this
*/
template <typename Range>
struct dsv_range
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Range const& range,
                dsv_settings const& settings)
    {
        typedef typename boost::range_const_iterator<Range>::type iterator_type;

        bool first = true;

        os << settings.list_open;

        // TODO: check EMPTY here

        for (iterator_type it = boost::begin(range);
            it != boost::end(range);
            ++it)
        {
            os << (first ? "" : settings.point_separator)
                << settings.point_open;

            stream_coordinate
                <
                    point, 0, dimension<point>::type::value
                >::apply(os, *it, settings);
            os << settings.point_close;

            first = false;
        }

        os << settings.list_close;
    }

    private:
        typedef typename boost::range_value<Range>::type point;
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<point>) );
};

/*!
\brief Stream sequence of points as DSV-part, e.g. (1 2),(3 4)
\note Used in polygon, all multi-geometries
*/





template <typename Polygon>
struct dsv_poly
{
    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
                Polygon const& poly,
                dsv_settings const& settings)
    {
        typedef typename ring_type<Polygon>::type ring;
        typedef typename boost::range_const_iterator<
                    typename interior_type<Polygon>::type>::type iterator;

        os << settings.list_open;

        dsv_range<ring>::apply(os, exterior_ring(poly), settings);
        for (iterator it = boost::begin(interior_rings(poly));
            it != boost::end(interior_rings(poly));
            ++it)
        {
            os << settings.list_separator;
            dsv_range<ring>::apply(os, *it, settings);
        }
        os << settings.list_close;
    }

    private:
        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<typename point_type<Polygon>::type>) );
};

template <typename Box, std::size_t Index>
struct dsv_box_corner
{
    typedef typename point_type<Box>::type point_type;

    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
            Box const& box,
            dsv_settings const& settings)
    {
        os << settings.point_open;
        stream_box_corner
            <
                Box, Index, 0, dimension<Box>::type::value
            >::apply(os, box, settings);
        os << settings.point_close;
    }
};


template <typename Box>
struct dsv_box
{
    typedef typename point_type<Box>::type point_type;

    template <typename Char, typename Traits>
    static inline void apply(std::basic_ostream<Char, Traits>& os,
            Box const& box,
            dsv_settings const& settings)
    {
        os << settings.list_open;
        dsv_box_corner<Box, 0>::apply(os, box, settings);
        os << settings.point_separator;
        dsv_box_corner<Box, 1>::apply(os, box, settings);
        os << settings.list_close;
    }
};

}} // namespace detail::dsv
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {

template <typename Tag, bool IsMulti, typename Geometry>
struct dsv {};


template <typename Point>
struct dsv<point_tag, false, Point>
    : detail::dsv::dsv_point<Point>
{};


template <typename Linestring>
struct dsv<linestring_tag, false, Linestring>
    : detail::dsv::dsv_range<Linestring>
{};


/*!
\brief Specialization to stream a box as DSV
*/
template <typename Box>
struct dsv<box_tag, false, Box>
    : detail::dsv::dsv_box<Box>
{};


/*!
\brief Specialization to stream a ring as DSV
*/
template <typename Ring>
struct dsv<ring_tag, false, Ring>
    : detail::dsv::dsv_range<Ring>
{};


/*!
\brief Specialization to stream polygon as DSV
*/
template <typename Polygon>
struct dsv<polygon_tag, false, Polygon>
    : detail::dsv::dsv_poly<Polygon>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
\brief Generic geometry template manipulator class, takes corresponding output class from traits class
\ingroup dsv
\details Stream manipulator, streams geometry classes as \ref DSV streams
\par Example:
Small example showing how to use the dsv class
\dontinclude doxygen_examples.cpp
\skip example_as_dsv_point
\line {
\until }
\note the template parameter must be specified. If that is inconvient, users might use streamdsv
which streams geometries as manipulators, or the object generator make_dsv
*/
template <typename Geometry>
class dsv_manipulator
{
public:

    inline dsv_manipulator(Geometry const& g,
            detail::dsv::dsv_settings const& settings)
        : m_geometry(g)
        , m_settings(settings)
    {}

    template <typename Char, typename Traits>
    inline friend std::basic_ostream<Char, Traits>& operator<<(
            std::basic_ostream<Char, Traits>& os,
            dsv_manipulator const& m)
    {
        dispatch::dsv
            <
                typename tag<Geometry>::type,
                is_multi<Geometry>::type::value,
                Geometry
            >::apply(os, m.m_geometry, m.m_settings);
        os.flush();
        return os;
    }

private:
    Geometry const& m_geometry;
    detail::dsv::dsv_settings m_settings;
};

/*!
\brief Main DSV-streaming function
\ingroup dsv
*/
template <typename Geometry>
inline dsv_manipulator<Geometry> dsv(Geometry const& geometry
    , std::string const& coordinate_separator = ", "
    , std::string const& point_open = "("
    , std::string const& point_close = ")"
    , std::string const& point_separator = ", "
    , std::string const& list_open = "("
    , std::string const& list_close = ")"
    , std::string const& list_separator = ", "
    )
{
    return dsv_manipulator<Geometry>(geometry,
        detail::dsv::dsv_settings(coordinate_separator,
            point_open, point_close, point_separator,
            list_open, list_close, list_separator));
}



} // namespace ggl

#endif // GGL_UTIL_WRITE_DSV_HPP
