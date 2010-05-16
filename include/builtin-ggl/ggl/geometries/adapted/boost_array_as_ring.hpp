// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_ADAPTED_BOOST_ARRAY_AS_RING_HPP
#define GGL_ADAPTED_BOOST_ARRAY_AS_RING_HPP


#ifdef _GEOMETRY_ADAPTED_BOOST_ARRAY_RANGE_TAG_DEFINED
#error Include only one headerfile to register tag for adapted boost::array
#endif

#define GGL_ADAPTED_BOOST_ARRAY_RANGE_TAG_DEFINED


#include <boost/array.hpp>

namespace ggl
{
#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

    template <typename T, size_t N>
    struct tag< boost::array<T, N> >
    {
        typedef ring_tag type;
    };

}
#endif
}


#endif
