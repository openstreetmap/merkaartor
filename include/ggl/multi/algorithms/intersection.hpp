// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_MULTI_INTERSECTION_HPP
#define GGL_MULTI_INTERSECTION_HPP

#include <vector>

#include <ggl/algorithms/intersection_polygon.hpp>

FIX ME it is very old and not yet adapted to tag-dispatching


namespace ggl
{

template<typename PB, typename P,
        template<typename,typename> class V, template<typename> class A>
inline void clip(const box<PB>& cb, const polygon<P, V, A>& poly, polygon<P, V, A>& poly_out)
{
    typedef polygon<P, V, A> POLY;
    std::vector<POLY> v;
    intersection(cb, poly, std::back_inserter<std::vector<POLY> > (v));
    poly_out.clear();
    poly_out = v.front();

}


template<typename PB, typename Y,
        template<typename,typename> class V, template<typename> class A>
inline void clip(const box<PB>& cb, const multi_polygon<Y, V, A>& mp_in, multi_polygon<Y, V, A>& mp_out)
{
    typedef multi_polygon<Y, V, A> MY;
    std::vector<Y> v;

    std::stringstream str;
    str << cb << std::endl;


#define CLIP_ALL
#ifdef CLIP_ALL

    for (typename MY::const_iterator it = mp_in.begin(); it != mp_in.end(); it++)
    {
        str << *it << std::endl;
        intersection(cb, *it, std::back_inserter<std::vector<Y> > (v));
    }
#else

    // TEMP take only the first one
    Y front = mp_in.front();
    str << front << std::endl;

    intersection(cb, front, std::back_inserter<std::vector<Y> > (v));
#endif


    str << "CLIPPED:" << std::endl;
    for (typename std::vector<Y>::const_iterator pit = v.begin(); pit != v.end(); pit++)
    {
        str << *pit << std::endl << std::endl;
        mp_out.push_back(*pit);
    }

    FILE* fp = fopen("c:/temp/poly.txt", "wt");
    fprintf(fp, "%s", str.str().c_str());
    fclose(fp);
}



} // namespace ggl


#endif // GGL_MULTI_INTERSECTION_HPP