// Generic Geometry Library
//
// Copyright Mateusz Loskot <mateusz@loskot.net> 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_IO_WKB_READ_WKB_HPP
#define GGL_IO_WKB_READ_WKB_HPP

#include <iterator>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/static_assert.hpp>

#include <ggl/core/tags.hpp>
#include <ggl/io/wkb/detail/parser.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename G>
struct read_wkb {};

template <typename G>
struct read_wkb<point_tag, G>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, G& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        return detail::wkb::parse_point(it, end, geometry, order);
    }
};

template <typename G>
struct read_wkb<linestring_tag, G>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, G& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        ggl::clear(geometry);
        typedef typename boost::range_value<G>::type point_type;
        return detail::wkb::parse_linestring
            <
            point_type
            >(it, end, std::back_inserter(geometry), order);
    }
};

template <typename G>
struct read_wkb<polygon_tag, G>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, G& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        return detail::wkb::parse_polygon(it, end, std::back_inserter(geometry), order);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template <typename G, typename Iterator>
inline bool read_wkb(Iterator begin, Iterator end, G& geometry)
{
    // Stream of bytes can only be parsed using random access iterator.
    BOOST_STATIC_ASSERT((
        boost::is_convertible
        <
            std::iterator_traits<Iterator>::iterator_category,
            const std::random_access_iterator_tag&
        >::value ));

    detail::wkb::byte_order_type::enum_t byte_order;
    if (detail::wkb::byte_order_parser::parse(begin, end, byte_order))
    {
        return dispatch::read_wkb
            <
            typename tag<G>::type,
            G
            >::parse(begin, end, geometry, byte_order);
    }

    return false;
}

// TODO: mloskot - Add read_wkb accepting raw pointers.

} // namespace ggl

#endif // GGL_IO_WKB_READ_WKB_HPP
