// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_ADAPT_TURNS_HPP
#define GGL_ADAPT_TURNS_HPP

#include <algorithm>


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>


#include <ggl/core/coordinate_type.hpp>

#include <ggl/algorithms/equals.hpp>


namespace ggl
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace intersection {



// TEMP: another COPY of side
// TODO: solve this inside SIDE!!!
template <typename P1, typename P2, typename P>
inline int the_side(P1 const& p1, P2 const& p2, P const& p)
{
    typedef typename select_coordinate_type<P, P1>::type T;

    T dx = get<0>(p2) - get<0>(p1);
    T dy = get<1>(p2) - get<1>(p1);
    T dpx = get<0>(p) - get<0>(p1);
    T dpy = get<1>(p) - get<1>(p1);
    T product =  dx * dpy - dy * dpx;
    return product > 0 ? 1 : product < 0 ? -1 : 0;
}

template <typename T>
inline void join_adapt(T& info, int direction)
{
    info.first->direction = direction;
    info.second->direction = direction;
}



template <typename T>
inline void both_same(T const& pi, T & pk, T const& qu, T& qw, int dir_p, int dir_q, int direction)
{
    // Comments are for "both Left"
    if (dir_q == direction) // If (Qw left-of Qu)
    {
        if (dir_p == -direction) // if (Pk right-of Pi)
        {
            // make Pk Right, Qw Left
            join_adapt(pk, -direction);
            join_adapt(qw, direction);
        }
        else if (pk.first->direction == -direction) // else if (Pk right-of Qu)
        {
            // make both Right
            join_adapt(pk, -direction);
            join_adapt(qw, -direction);
        }
    }
    else
    {
        if (dir_p == -direction // if (Pk right-of Pi
            || pk.first->direction == direction) // || Pk left-of Qu)
        {
            //  make both Left
            join_adapt(pk, direction);
            join_adapt(qw, direction);
        }
    }
}

template <typename T>
inline void crossing(T const& pi, T & pk, T const& qu, T& qw, int dir_p, int dir_q, int direction)
{
    if (dir_p == direction) // If (Pk left-of Pi)
    {
        if (pk.first->direction == -direction) // if (Pk right-of Qu)
        {
            // make both Right
            join_adapt(pk, -direction);
            join_adapt(qw, -direction);
        }
        else
        {
            // make Pk Left, Qw Right
            join_adapt(pk, direction);
            join_adapt(qw, -direction);
        }
    }
}



template <typename T>
inline void collinear(T const& pi, T & pk, T const& qu, T& qw, int dir_p, int dir_q, int direction)
{
    if (dir_p == direction // If (Pk left-of Pi
        && dir_q == direction // && Qw left-of Qu
        && pk.first->direction == -direction) // && Pk right-of Qu
    {
        // make both Right
        join_adapt(pk, -direction);
        join_adapt(qw, -direction);
    }
    else if (dir_p == -direction // If (Pk right-of Pi
        && dir_q == -direction) // && Qw right-of Qu
    {
        // make both Left
        join_adapt(pk, direction);
        join_adapt(qw, direction);
    }

}

template <typename T, typename P>
inline void assign_pq(T const& info, P& p, P& q)
{
    if (info.seg_id.source_index == 0)
    {
        if (info.arrival == 1)
        {
            p.first = info.other_point;
        }
        else if (info.arrival == -1)
        {
            p.second = info.other_point;
        }
    }
    else
    {
        if (info.arrival == 1)
        {
            q.first = info.other_point;
        }
        else if (info.arrival == -1)
        {
            q.second = info.other_point;
        }
    }
}

template <typename Info, typename Point>
inline void touch_in_the_middle(Info& info, Point const& point)
{
    typedef typename boost::range_iterator<Info>::type iterator;

    // Determine which is Q, and assign points
    iterator qu, qw, p;
    int count = 0;

    // Find out which one is arriving/departing from the middle
    for (iterator it = boost::begin(info);
        it != boost::end(info);
        ++it)
    {
        // One arrives AND departs in the middle of the other.
        if(it->how == 'm' && it->arrival == 1)
        {
            qu = it;
            count |= 1;
        }
        else if(it->how == 's' && it->arrival == -1)
        {
            qw = it;
            count |= 2;
        }
        else if (it->how == 's') //  or m, does not really matter
        {
            p = it;
            count |= 4;
        }

    }
    // Adapt it
    if (count == 7
        && qu->direction == qw->direction
        && qu->seg_id.source_index == qw->seg_id.source_index)
    {
        int dir_q = the_side(qu->other_point, point, qw->other_point);

#ifdef GGL_DEBUG_INTERSECTION
        std::cout << "Both "
            << (qu->direction == 1 ? "Left" : "Right")
            << ", Turn " << (dir_q == 1 ? "Left" : dir_q == -1 ? "Right" : "?")
            << std::endl;
#endif

        // Let P also be starting
        p->arrival = -1;

        // This is also symmetric. See slides:
        // Qu left-of P && Qw left-of P
        if (qu->direction == 1 && dir_q == -1)
        {
            // make both Left
            p->direction = 1;
            qw->direction = 1;
        }
        // else, symmetric version
        else if (qu->direction == -1 && dir_q == 1)
        {
            p->direction = -1;
            qw->direction = -1;
        }
    }
}




template <typename Info>
inline void arrive_in_the_middle(Info& info)
{
    typedef typename boost::range_iterator<Info>::type iterator;

    // Find out which one is NOT arriving in the middle,
    // and the direction of the one arriving in the middle
    int departing = -1;
    int direction = 0;
    for (iterator it = boost::begin(info);
        it != boost::end(info);
        ++it)
    {
        if(it->how == 'm')
        {
            switch(it->arrival)
            {
                case 1 : direction = it->direction; break;
                default : departing = it->seg_id.source_index;
            }
        }
    }

    // Make this the departing one, following collinear in opposite segment,
    // same direction as established above
    for (iterator it = boost::begin(info);
        it != boost::end(info);
        ++it)
    {
        if (it->how == 'c' && it->seg_id.source_index == departing)
        {
            it->arrival = -1;
            it->direction = direction;
        }
    }
}

template <typename Info>
inline void start_in_the_middle(Info& info, bool opposite)
{
    typedef typename boost::range_iterator<Info>::type iterator;

    for (iterator it = boost::begin(info);
        it != boost::end(info);
        ++it)
    {
        if(it->how == 's')
        {
            if (! opposite)
            {
                // Not opposite, all "start" traversals can also be made "departing"
                it->arrival = -1;
            }
            else if (opposite && it->arrival == 0)
            {
                // Prevent the collinear the "start" from "departing",
                // if it is in opposite direction
                it->arrival = 1;
                it->direction = 0;
                it->flagged = true; // might be deleted
            }
        }
    }
}


template <typename V>
inline void adapt_turns(V& intersection_points)
{
    typedef typename boost::range_iterator<V>::type iterator_type;
    typedef typename boost::range_value<V>::type ip_type;
    typedef typename ip_type::traversal_type info_type;

    typedef typename boost::range_value<V>::type::traversal_vector vector_type;
    typedef typename boost::range_iterator<vector_type>::type tvit_type;

    for (iterator_type it = boost::begin(intersection_points);
         it != boost::end(intersection_points);
         ++it)
    {
        if (! it->trivial)
        {
            if (boost::size(it->info) == 4)
            {
                /*
                    can be ARRIVE/START from the middle (#11)
                        src 0 seg 1 (// 1.0) how m[A R] p // qu
                        src 0 seg 1 (// 1.1) how s[D L] p // qw
                        src 1 seg 0 (// 0.1) how m[A R] qu / p
                        src 1 seg 1 (// 0.1) how s[D R] qw / p

                    or can be ARRIVE at COLLINEARITY (#8, #13)
                        src 0 seg 1 (// 1.1) how m[A L] p // qu
                        src 0 seg 1 (// 1.2) how c[- -] p // qw
                        src 1 seg 1 (// 0.1) how m[A L] qu // p
                        src 1 seg 2 (// 0.1) how c[- -] qw // p

                    or can be START from COLLINEARITY (#8, #13)
                        src 0 seg 1 (// 1.2) how c[- -] p // qu
                        src 0 seg 1 (// 1.0) how s[D R] p // qw
                        src 1 seg 2 (// 0.1) how c[- -] qu // p
                        src 1 seg 0 (// 0.1) how s[D L] qw // p
                */

                 // First detect the case and if it is opposite
                int count_m = 0, count_s = 0, count_c = 0;

                bool opposite = false;
                for (tvit_type tvit = boost::begin(it->info);
                    tvit != boost::end(it->info);
                    ++tvit)
                {
                    switch(tvit->how)
                    {
                        case 'm' : count_m++; break;
                        case 's' : count_s++; break;
                        case 'c' : count_c++; break;
                    }
                    if (tvit->opposite)
                    {
                        opposite = true;
                    }
                }


                if (count_m == 2 && count_s == 2)
                {
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Touching the middle " << std::endl;
#endif

                    touch_in_the_middle(it->info, it->point);
                }
                else if (count_m == 2 && count_c == 2 && opposite)
                {
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Arriving the middle/collinearity, opposite" << std::endl;
#endif

                    arrive_in_the_middle(it->info);
                }
                else if (count_s == 2 && count_c == 2)
                {
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Starting from middle/collinearity"
                        << (opposite ? " , opposite"  : "")
                        << std::endl;
#endif

                    start_in_the_middle(it->info, opposite);
                }
            }

            if (boost::size(it->info) == 8)
            {
                /*
                  src 0 seg 1 (// 1.0) how t[A R]   pi // qu   pi.first
                  src 0 seg 1 (// 1.1) how a[A R]   pi // qw   pi.second
                  src 0 seg 2 (// 1.0) how a[D R]   pk // qu   pk.first
                  src 0 seg 2 (// 1.1) how f[D L]   pk // qw   pk.second

                  src 1 seg 0 (// 0.1) how t[A L]   qu // pi
                  src 1 seg 0 (// 0.2) how a[A R]   qu // pk
                  src 1 seg 1 (// 0.1) how a[D R]   qw // pi
                  src 1 seg 1 (// 0.2) how f[D R]   qw // pk
                */

                std::pair<tvit_type, tvit_type> pi, pk, qu, qw;
                std::pair<typename info_type::point_type, typename info_type::point_type> p, q;


                // Find out which is which
                for (tvit_type tvit = boost::begin(it->info);
                    tvit != boost::end(it->info);
                    ++tvit)
                {
                    assign_pq(*tvit, p, q);
                    if (tvit->seg_id.source_index == 0)
                    {
                        if (tvit->arrival == 1)
                        {
                            if(tvit->how != 'a')
                            {
                                pi.first = tvit;
                            }
                            else
                            {
                                pi.second = tvit;
                            }

                        }
                        else if (tvit->arrival == -1)
                        {
                            if (tvit->how == 'a')
                            {
                                pk.first = tvit;
                            }
                            else
                            {
                                pk.second = tvit;
                            }
                        }
                    }
                    else
                    {
                        if (tvit->arrival == 1)
                        {
                            if(tvit->how != 'a')
                            {
                                qu.first = tvit;
                            }
                            else
                            {
                                qu.second = tvit;
                            }
                        }
                        else if (tvit->arrival == -1)
                        {
                            if (tvit->how == 'a')
                            {
                                qw.first = tvit;
                            }
                            else
                            {
                                qw.second = tvit;
                            }
                        }
                    }
                }

                int dir_p = the_side(p.first, it->point, p.second);
                int dir_q = the_side(q.first, it->point, q.second);

#ifdef GGL_DEBUG_INTERSECTION
                std::cout << "Pi//Qu : " << *pi.first << std::endl;
                std::cout << "Pi//Qw : " << *pi.second << std::endl;
                std::cout << "Pk//Qu : " << *pk.first << std::endl;
                std::cout << "Pk//Qw : " << *pk.second << std::endl;
                std::cout << "Qu//Pi : " << *qu.first << std::endl;
                std::cout << "Qu//Pk : " << *qu.second << std::endl;
                std::cout << "Qw//Pi : " << *qw.first << std::endl;
                std::cout << "Qw//Pk : " << *qw.second << std::endl;
                if (dir_p == 1) std::cout << "P turns left" << std::endl;
                if (dir_p == -1) std::cout << "P turns right" << std::endl;
                if (dir_q == 1) std::cout << "Q turns left" << std::endl;
                if (dir_q == -1) std::cout << "Q turns right" << std::endl;
#endif

                if (qu.first->direction == qw.first->direction)
                {
                    // Both Right or Both Left
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Both "
                        << (qu.first->direction == 1 ? "Left" : "Right")
                        << std::endl;
#endif

                    both_same(pi, pk, qu, qw, dir_p, dir_q, qu.first->direction);
                }
                else if (qu.first->direction == -qw.first->direction)
                {
                    // From Left to Right, or from Right to Left
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Left to/from Right" << std::endl;
#endif
                    crossing(pi, pk, qu, qw, dir_p, dir_q, qu.first->direction);
                }
                else if (qw.first->direction == 1 || qu.first->direction == 1)
                {
                    // Collinear left
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Collinear left" << std::endl;
#endif
                    collinear(pi, pk, qu, qw, dir_p, dir_q, 1);
                }
                else if (qw.first->direction == -1 || qu.first->direction == -1)
                {
                    // Collinear right
#ifdef GGL_DEBUG_INTERSECTION
                    std::cout << "Collinear right" << std::endl;
#endif
                    collinear(pi, pk, qu, qw, dir_p, dir_q, -1);
                }


                for (tvit_type tvit = boost::begin(it->info);
                    tvit != boost::end(it->info);
                    ++tvit)
                {
                    if (tvit->how == 'a')
                    {
                        tvit->direction = 0;
                    }
                }


             }
        }
    }

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Adapted turns: " << std::endl;
    report_ip(intersection_points);
#endif
}



}} // namespace detail::intersection
#endif //DOXYGEN_NO_DETAIL



template <typename V>
inline void adapt_turns(V& intersection_points)
{
    // If there are merges, there might be merged IP's which have right turns
    detail::intersection::adapt_turns(intersection_points);

#ifdef GGL_DEBUG_INTERSECTION
    std::cout << "Merged (2): " << std::endl;
    report_ip(intersection_points);
#endif

}


} // namespace ggl

#endif // GGL_ADAPT_TURNS_HPP
