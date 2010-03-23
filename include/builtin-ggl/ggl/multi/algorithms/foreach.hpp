// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_FOREACH_HPP
#define GGL_MULTI_FOREACH_HPP

#include <vector>

#include <ggl/algorithms/for_each.hpp>

FIX ME it is not yet adapted to tag-dispatching


namespace ggl
{

template<typename MP, typename F>
inline F for_each_point_multi_point(MP& mp, F f)
{
    return (for_each_point_container(mp, f));
}

template<typename ML, typename F>
inline F for_each_point_multi_linestring(ML& ml, F f)
{
    for (typename ML::iterator it = ml.begin(); it != ml.end(); it++)
    {
        f = for_each_point_linestring(*it, f);
    }
    return (f);
}

template<typename MY, typename F>
inline F for_each_point_multi_polygon(MY& mp, F f)
{
    for (typename MY::iterator it = mp.begin(); it != mp.end(); it++)
    {
        f = for_each_point_polygon(*it, f);
    }
    return (f);
}




template<typename MP, typename F>
inline F for_each_point_multi_point(const MP& mp, F f)
{
    return (for_each_point_container(mp, f));
}


template<typename ML, typename F>
inline F for_each_point_multi_linestring(const ML& ml, F f)
{
    for (typename ML::const_iterator it = ml.begin(); it != ml.end(); it++)
    {
        f = for_each_point_linestring(*it, f);
    }
    return (f);
}

template<typename MY, typename F>
inline F for_each_point_multi_polygon(const MY& mp, F f)
{
    for (typename MY::const_iterator it = mp.begin(); it != mp.end(); it++)
    {
        f = for_each_point_polygon(*it, f);
    }
    return (f);
}



template<typename ML, typename F>
inline F for_each_segment_multi_linestring(ML& ml, F f)
{
    for (typename ML::iterator it = ml.begin(); it != ml.end(); it++)
    {
        f = for_each_segment_linestring(*it, f);
    }
    return (f);
}

template<typename MY, typename F>
inline F for_each_segment_multi_polygon(MY& mp, F f)
{
    for (typename MY::iterator it = mp.begin(); it != mp.end(); it++)
    {
        f = for_each_segment_polygon(*it, f);
    }
    return (f);
}






template<typename ML, typename F>
inline F for_each_segment_multi_linestring(const ML& ml, F f)
{
    for (typename ML::const_iterator it = ml.begin(); it != ml.end(); it++)
    {
        f = for_each_segment_linestring(*it, f);
    }
    return (f);
}

template<typename MY, typename F>
inline F for_each_segment_multi_polygon(const MY& mp, F f)
{
    for (typename MY::const_iterator it = mp.begin(); it != mp.end(); it++)
    {
        f = for_each_segment_polygon(*it, f);
    }
    return (f);
}






template<typename P,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(multi_point<P, V, A>& mp, F f)
{
    return (for_each_point_multi_point(mp, f));
}

template<typename L,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(multi_linestring<L, V, A>& ml, F f)
{
    return (for_each_point_multi_linestring(ml, f));
}

template<typename Y,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(multi_polygon<Y, V, A>& mp, F f)
{
    return (for_each_point_multi_polygon(mp, f));
}




template<typename P,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(const multi_point<P, V, A>& mp, F f)
{
    return (for_each_point_multi_point(mp, f));
}

template<typename L,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(const multi_linestring<L, V, A>& ml, F f)
{
    return (for_each_point_multi_linestring(ml, f));
}


template<typename Y,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_point(const multi_polygon<Y, V, A>& mp, F f)
{
    return (for_each_point_multi_polygon(mp, f));
}




template<typename L,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_segment(multi_linestring<L, V, A>& ml, F f)
{
    return (for_each_segment_multi_linestring(ml, f));
}

template<typename Y,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_segment(multi_polygon<Y, V, A>& mp, F f)
{
    return (for_each_segment_multi_polygon(mp, f));
}




template<typename L,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_segment(const multi_linestring<L, V, A>& ml, F f)
{
    return (for_each_segment_multi_linestring(ml, f));
}

template<typename Y,
        template<typename,typename> class V, template<typename> class A,
        typename F>
inline F for_each_segment(const multi_polygon<Y, V, A>& mp, F f)
{
    return (for_each_segment_multi_polygon(mp, f));
}
} // namespace ggl


#endif // GGL_MULTI_FOREACH_HPP
