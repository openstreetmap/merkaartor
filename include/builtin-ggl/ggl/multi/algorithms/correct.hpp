// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_CORRECT_HPP
#define GGL_MULTI_CORRECT_HPP

#include <vector>

#include <ggl/algorithms/correct.hpp>


//FIX ME it is not yet adapted to tag-dispatching


namespace ggl
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{
    namespace correct
    {
        // correct a multi-polygon
        template <typename O>
        inline void correct_multi_polygon(O& o)
        {
            for (typename O::iterator it = o.begin(); it != o.end(); it++)
            {
                correct_polygon(*it);
            }
        }
    }
}
#endif

template<typename Y,
        template<typename,typename> class V, template<typename> class A>
void correct(multi_polygon<Y, V, A>& mp)
{
    detail::correct::correct_multi_polygon(mp);
}



} // namespace ggl


#endif // GGL_MULTI_CORRECT_HPP
