// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_ALGORITHMS_DETAIL_MODIFY_WITH_PREDICATE_HPP
#define GGL_MULTI_ALGORITHMS_DETAIL_MODIFY_WITH_PREDICATE_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


namespace ggl {


#ifndef DOXYGEN_NO_IMPL
namespace impl {

template <typename MultiPolygon, typename Predicate, typename Policy>
struct multi_modify_with_predicate
{
    static inline void modify(MultiPolygon& multi, Predicate const& predicate)
    {
        typedef typename boost::range_iterator<MultiPolygon>::type iterator;
        for (iterator it = boost::begin(multi);
            it != boost::end(multi);
            ++it)
        {
            Policy::modify(*it, predicate);
        }
    }
};


} // namespace impl
#endif


} // namespace ggl


#endif // GGL_MULTI_ALGORITHMS_DETAIL_MODIFY_WITH_PREDICATE_HPP
