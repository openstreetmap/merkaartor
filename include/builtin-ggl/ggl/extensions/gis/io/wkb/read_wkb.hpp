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
#include <ggl/extensions/gis/io/wkb/detail/parser.hpp>

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
        return detail::wkb::point_parser<G>::parse(it, end, geometry, order);
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
        return detail::wkb::linestring_parser<G>::parse(it, end, geometry, order);
    }
};

template <typename G>
struct read_wkb<polygon_tag, G>
{
    template <typename Iterator>
    static inline bool parse(Iterator& it, Iterator end, G& geometry,
        detail::wkb::byte_order_type::enum_t order)
    {
        ggl::clear(geometry);
        return detail::wkb::polygon_parser<G>::parse(it, end, geometry, order);
    }
};

} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template <typename Iterator, typename G>
inline bool read_wkb(Iterator begin, Iterator end, G& geometry)
{
    // Stream of bytes can only be parsed using random access iterator.
    BOOST_STATIC_ASSERT((
        boost::is_convertible
        <
            typename std::iterator_traits<Iterator>::iterator_category,
            const std::random_access_iterator_tag&
        >::value));

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

template <typename ByteType, typename G>
inline bool read_wkb(ByteType const* bytes, std::size_t length, G& geometry)
{
    BOOST_STATIC_ASSERT((boost::is_integral<ByteType>::value));
    BOOST_STATIC_ASSERT((sizeof(boost::uint8_t) == sizeof(ByteType)));

    ByteType const* begin = bytes;
    ByteType const* const end = bytes + length;

    return read_wkb(begin, end, geometry);
}


} // namespace ggl

#endif // GGL_IO_WKB_READ_WKB_HPP
