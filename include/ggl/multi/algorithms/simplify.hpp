// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_SIMPLIFY_HPP
#define GGL_MULTI_SIMPLIFY_HPP

#include <vector>

#include <ggl/algorithms/simplify.hpp>

FIX ME it is not yet adapted to tag-dispatching

namespace ggl
{


        template<typename ML>
        inline void simplify_multi_linestring(const ML& ml_in, ML& ml_out, double max_distance)
        {
            ml_out.resize(ml_in.size());
            typename ML::const_iterator it_in = ml_in.begin();
            typename ML::iterator it_out = ml_out.begin();
            for (; it_in != ml_in.end(); it_in++, it_out++)
            {
                simplify_linestring(*it_in, *it_out, max_distance);
            }
        }


        template<typename MY>
        inline void simplify_multi_polygon(const MY& mp_in, MY& mp_out, double max_distance)
        {
            mp_out.resize(mp_in.size());
            typename MY::const_iterator it_in = mp_in.begin();
            typename MY::iterator it_out = mp_out.begin();
            for (; it_in != mp_in.end(); it_in++, it_out++)
            {
                simplify(*it_in, *it_out, max_distance);
            }
        }



template<typename L,
        template<typename,typename> class V, template<typename> class A>
inline void simplify(const multi_linestring<L, V, A>& ml_in,
    multi_linestring<L, V, A>& ml_out, double max_distance)
{
    simplify_multi_linestring(ml_in, ml_out, max_distance);
}

template<typename Y,
        template<typename,typename> class V, template<typename> class A>
inline void simplify(const multi_polygon<Y, V, A>& mp_in,
            multi_polygon<Y, V, A>& mp_out, double max_distance)
{
    simplify_multi_polygon(mp_in, mp_out, max_distance);
}


} // namespace ggl


#endif // GGL_MULTI_SIMPLIFY_HPP
