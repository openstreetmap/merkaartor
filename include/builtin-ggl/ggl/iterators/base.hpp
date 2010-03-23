// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ITERATORS_BASE_HPP
#define GGL_ITERATORS_BASE_HPP

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>

#ifndef DOXYGEN_NO_DETAIL
namespace ggl { namespace detail { namespace iterators {

template <typename T, typename Iterator>
struct iterator_base :
    public  boost::iterator_adaptor
    <
        T,
        Iterator,
        boost::use_default,
        typename boost::mpl::if_
        <
            boost::is_convertible
            <
                typename boost::iterator_traversal<Iterator>::type,
                boost::random_access_traversal_tag
            >,
            boost::bidirectional_traversal_tag,
            boost::use_default
        >::type
    >
{
    // Define operator cast to Iterator to be able to write things like Iterator it = myit++
    inline operator Iterator() const
    {
        return this->base();
    }

    /*inline bool operator==(const Iterator& other) const
    {
        return this->base() == other;
    }
    inline bool operator!=(const Iterator& other) const
    {
        return ! operator==(other);
    }*/
};

}}} // namespace ggl::detail::iterators
#endif


#endif // GGL_ITERATORS_BASE_HPP
