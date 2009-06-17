// Generic Geometry Library
//
// Copyright Mateusz Loskot <mateusz@loskot.net> 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_IO_WKB_DETAIL_PARSER_HPP
#define GGL_IO_WKB_DETAIL_PARSER_HPP

#include <cassert>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <limits>

#include <boost/cstdint.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>
#include <boost/concept/requires.hpp>

#include <ggl/core/access.hpp>
#include <ggl/core/coordinate_dimension.hpp>
#include <ggl/core/coordinate_type.hpp>
#include <ggl/core/concepts/point_concept.hpp>
#include <ggl/detail/endian.hpp>
#include <ggl/io/wkb/detail/ogc.hpp>

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace detail { namespace wkb {

template <typename T>
struct value_parser
{
    typedef T value_type;

    template <typename Iterator>
    static bool parse(Iterator& it, Iterator end, T& value, byte_order_type::enum_t order)
    {
        BOOST_STATIC_ASSERT(boost::is_integral<Iterator::value_type>::value);
        BOOST_STATIC_ASSERT(sizeof(Iterator::value_type) == sizeof(boost::uint8_t));

        std::size_t const required_size = sizeof(T);
        if (it != end && std::distance(it, end) >= required_size)
        {
            endian::endian_value<T> parsed_value;

            // Decide on direcion of endianness translation, detault to native
            if (byte_order_type::xdr == order) 
            {
                parsed_value.load<endian::big_endian_tag>(it);
            }
            else if (byte_order_type::ndr == order) 
            {
                parsed_value.load<endian::little_endian_tag>(it);
            }
            else
            {
                parsed_value.load<endian::native_endian_tag>(it);
            }

            value = parsed_value;
            std::advance(it, required_size);
            return true;
        }

        return false;
    }
};

struct byte_order_parser
{
    template <typename Iterator>
    static bool parse(Iterator& it, Iterator end, byte_order_type::enum_t& order)
    {
        boost::uint8_t value;
        if (value_parser<boost::uint8_t>::parse(it, end, value, byte_order_type::unknown))
        {
            if (byte_order_type::unknown > value)
            {
                order = byte_order_type::enum_t(value);
            }
            return true;
        }
        return false;
    }
};

struct geometry_type_parser
{
    template <typename Iterator>
    static bool parse(Iterator& it, Iterator end, geometry_type::enum_t& type,
        byte_order_type::enum_t order)
    {
        boost::uint32_t value;
        if (value_parser<boost::uint32_t>::parse(it, end, value, order))
        {
            // TODO: Refine the test when multi* geometries are supported

            boost::uint32_t id = value & 0xff;
            if (geometry_type::polygon >= id)
            {
                type = geometry_type::enum_t(id);
                return true;
            }
        }
        return false;
    }
};

template <typename P, int I, int N>
struct parsing_assigner
{
    template <typename Iterator>
    static void run(Iterator& it, Iterator end, P& point, byte_order_type::enum_t order)
    {
        typedef typename coordinate_type<P>::type coordinate_type;

        // coordinate type in WKB is always double
        double value(0);
        if (value_parser<coordinate_type>::parse(it, end, value, order))
        {
            // actual coordinate type of point may be different
            set<I>(point, static_cast<coordinate_type>(value));
        }
        else
        {
            // TODO: mloskot - Report premature termination at coordinate level
            //throw failed to read coordinate value

            // default initialized value as fallback
            set<I>(point, coordinate_type());
        }
        parsing_assigner<P, I+1, N>::run(it, end, point, order);
    }
};

template <typename P, int N>
struct parsing_assigner<P, N, N>
{
    template <typename Iterator>
    static void run(Iterator& it, Iterator end, P& point, byte_order_type::enum_t order)
    {
        // terminate
    }
};

template <typename P>
struct point_parser
{
    template <typename Iterator>
    static bool parse(Iterator& it, Iterator end, P& point, byte_order_type::enum_t order)
    {
        // TODO: mloskot - Add assert on point dimension, 2d only

        geometry_type::enum_t type;
        if (geometry_type_parser::parse(it, end, type, order))
        {
            if (geometry_type::point == type)
            {
                parsing_assigner<P, 0, dimension<P>::value>::run(it, end, point, order);
            }
            return true;
        }
        return false;
    }
};

template <typename P>
struct linestring_parser
{
    template <typename Iterator, typename OutputIterator>
    static bool parse(Iterator& it, Iterator end, OutputIterator out, byte_order_type::enum_t order)
    {
        geometry_type::enum_t type;
        if (!geometry_type_parser::parse(it, end, type, order))
        {
            return false;
        }

        boost::uint32_t num_points(0);
        if (geometry_type::linestring != type ||
            !value_parser<boost::uint32_t>::parse(it, end, num_points, order))
        {
            return false;
        }

        typedef typename std::iterator_traits<Iterator>::difference_type size_type;
        assert(num_points <= boost::uint32_t(std::numeric_limits<size_type>::max()));

        size_type const linestring_size = static_cast<size_type>(num_points);
        size_type const point_size = dimension<P>::value * sizeof(double);

        if (std::distance(it, end) >= (linestring_size * point_size))
        {
            P point_buffer;
            for (size_type i = 0; i < linestring_size; ++i)
            {  
                parsing_assigner<P, 0, dimension<P>::value>::run(it, end, point_buffer, order);
                out = point_buffer;
                ++out;
            }
            return true;
        }
        return false;
    }
};

template <typename Iterator, typename P>
inline
BOOST_CONCEPT_REQUIRES(((concept::Point<P>)), (bool))
parse_point(Iterator& it, Iterator end, P& point, byte_order_type::enum_t order)
{
    return point_parser<P>::parse(it, end, point, order);
}

template <typename P, typename Iterator, typename OutputIterator>
inline bool parse_linestring(Iterator& it, Iterator end, OutputIterator out,
                             byte_order_type::enum_t order)
{
    return linestring_parser<P>::parse(it, end, out, order);
}

template <typename Iterator, typename P>
inline bool parse_polygon(Iterator& it, Iterator end, P& polygon,
                          byte_order_type::enum_t order)
{
    assert(!"to be implemented");
    return false;
}

}}} // namespace ggl::detail::wkb
#endif // DOXYGEN_NO_IMPL

#endif // GGL_IO_WKB_DETAIL_PARSER_HPP
