// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_MULTI_ITERATORS_POINT_CONST_ITERATOR_HPP
#define GGL_MULTI_ITERATORS_POINT_CONST_ITERATOR_HPP


#include <boost/type_traits/remove_const.hpp>


#include <ggl/iterators/point_const_iterator.hpp>


namespace ggl
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename MultiPolygon>
struct point_const_iterator<multi_polygon_tag, MultiPolygon>
{
    typedef typename boost::range_value<MultiPolygon>::type polygon_type;
    typedef typename boost::range_const_iterator
        <
            typename ring_type<polygon_type>::type
        >::type type;
};




} // namespace dispatch
#endif



}


#endif // GGL_MULTI_ITERATORS_POINT_CONST_ITERATOR_HPP
