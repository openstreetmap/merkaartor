// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_ITERATORS_POINT_CONST_ITERATOR_HPP
#define GGL_ITERATORS_POINT_CONST_ITERATOR_HPP


#include <boost/type_traits/remove_const.hpp>


#include <ggl/core/tag.hpp>
#include <ggl/core/tags.hpp>


#include <boost/range/metafunctions.hpp>

namespace ggl
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry>
struct point_const_iterator
{
    // The default: meta-forward this to boost
    // This enables calling this function using std::vector as well, even if they
    // are not registered.
    // It also requires less specializations
    typedef typename boost::range_const_iterator<Geometry>::type type;
};


template <typename Polygon>
struct point_const_iterator<polygon_tag, Polygon>
{
    typedef typename boost::range_const_iterator
        <
            typename ring_type<Polygon>::type
        >::type type;
};




} // namespace dispatch
#endif


/*!
    \brief Meta-function which defines point-const-iterator type
    \ingroup iterators
*/
template <typename Geometry>
struct point_const_iterator
{
    typedef typename boost::remove_const<Geometry>::type ncg;
    typedef typename dispatch::point_const_iterator<
        typename tag<Geometry>::type, ncg>::type type;
};




}


#endif // GGL_ITERATORS_POINT_CONST_ITERATOR_HPP
