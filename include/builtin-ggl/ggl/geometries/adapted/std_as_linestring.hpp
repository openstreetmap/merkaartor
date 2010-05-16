// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_ADAPTED_STD_AS_LINESTRING_HPP
#define GGL_ADAPTED_STD_AS_LINESTRING_HPP


#ifdef _GEOMETRY_ADAPTED_STD_RANGE_TAG_DEFINED
#error Include only one headerfile to register tag for adapted std:: containers or iterator pair
#endif

#define GGL_ADAPTED_STD_RANGE_TAG_DEFINED


#include <vector>
#include <deque>
#include <list>
#include <utility>


namespace ggl
{
#ifndef DOXYGEN_NO_DETAIL
namespace util
{
    struct std_as_linestring
    {
        typedef linestring_tag type;
    };

}
#endif


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    // specialization for an iterator pair (read only)
    template <typename P> struct tag< std::pair<P, P> > : util::std_as_linestring {};

    // Indicate that std::library is not used to add things to std::pair.
    // Don't implement anything else -> adding points or clearing not possible
    template <typename P> struct use_std< std::pair<P, P> > : boost::mpl::false_ {};

    // specializations for a std:: containers: vector, deque, list
    template <typename P> struct tag< std::vector<P> > : util::std_as_linestring {};
    template <typename P> struct tag< std::deque<P> > : util::std_as_linestring {};
    template <typename P> struct tag< std::list<P> > : util::std_as_linestring {};

}
#endif
}


#endif
