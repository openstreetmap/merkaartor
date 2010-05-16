// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_ADAPTED_STD_AS_RING_HPP
#define GGL_ADAPTED_STD_AS_RING_HPP


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
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    // specialization for an iterator pair
    template <typename T> struct tag< std::pair<T, T> > { typedef ring_tag type; };

    // specialization for a std:: containers: vector, deque, list
    template <typename T> struct tag< std::vector<T> > { typedef ring_tag type; };
    template <typename T> struct tag< std::deque<T> > { typedef ring_tag type; };
    template <typename T> struct tag< std::list<T> > { typedef ring_tag type; };
}
#endif
}


#endif
