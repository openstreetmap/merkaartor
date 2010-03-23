// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_AGNOSTIC_SIMPLIFY_HPP
#define GGL_STRATEGY_AGNOSTIC_SIMPLIFY_HPP

#include <boost/range/functions.hpp>

#include <ggl/core/cs.hpp>


//#define GL_DEBUG_SIMPLIFY

#ifdef GL_DEBUG_SIMPLIFY
#include <ggl/extensions/gis/io/wkt/write_wkt.hpp>
#include <ggl/extensions/gis/io/wkt/stream_wkt.hpp>
#include <iostream>
#endif


namespace ggl
{
namespace strategy
{
    namespace simplify
    {
        #ifndef DOXYGEN_NO_DETAIL
        namespace detail
        {

            /*!
                \brief Small wrapper around a point, with an extra member "included"
                \details
                - We could implement a complete point as well but that is not necessary
                - We could derive from ggl::point but we need the original later on, including extra members;
                    besides that it is not necessary to copy points during the algorithm
                \tparam the enclosed point type
            */
            template<typename P>
            struct douglas_peucker_point
            {
                const P& p;
                bool included;

                inline douglas_peucker_point(const P& ap)
                    : p(ap)
                    , included(false)
                {}

                inline douglas_peucker_point<P> operator=(const douglas_peucker_point<P>& other)
                {
                    return douglas_peucker_point<P>(*this);
                }
            };
        }
        #endif // DOXYGEN_NO_DETAIL


        /*!
            \brief Implements the simplify algorithm.
            \ingroup simplify
            \details The douglas_peucker strategy simplifies a linestring, ring or vector of points
            using the well-known Douglas-Peucker algorithm. For the algorithm, see for example:
            \see http://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm
            \see http://www2.dcs.hull.ac.uk/CISRG/projects/Royal-Inst/demos/dp.html
            \tparam R boost range
            \tparam O_IT output iterator
            \tparam PSDS point-segment distance strategy to be used
            \note This strategy uses itself a point-segment-distance strategy which can be specified
            \author Barend and Maarten, 1995/1996
            \author Barend, revised for Generic Geometry Library, 2008
        */
        template<typename R, typename O_IT, typename PSDS>
        class douglas_peucker
        {
            typedef typename point_type<R>::type P;
            typedef detail::douglas_peucker_point<P> DP;
            typedef typename std::vector<DP>::iterator DIT;

            typedef typename PSDS::return_type RET;

            inline void consider(DIT begin, DIT end, const RET& max_dist, int& n,
                            const PSDS& ps_distance_strategy) const
            {
                size_t size = end - begin;
                // size must be at least 3 here: we want to consider a candidate point between begin and end
                if (size <= 2)
                {
#ifdef GL_DEBUG_SIMPLIFY
                    if (begin != end)
                    {
                        std::cout << "ignore between " << begin->p << " and " << (end - 1)->p << " size=" << size << std::endl;
                    }
                    std::cout << "return because size=" << size << std::endl;
#endif
                    return;
                }

                DIT last = end - 1;

#ifdef GL_DEBUG_SIMPLIFY
                std::cout << "find between " << begin->p << " and " << last->p << " size=" << size << std::endl;
#endif


                // Find most distance point, compare to the current segment
                ggl::segment<const P> s(begin->p, last->p);
                RET md(-1.0); // any value < 0
                DIT candidate;
                for(DIT it = begin + 1; it != last; it++)
                {
                    RET dist = ps_distance_strategy(it->p, s);

#ifdef GL_DEBUG_SIMPLIFY
                    std::cout << "consider " << it->p << " at " << dist.value() << (dist.value() > max_dist.value() ? " maybe" : " no") << std::endl;
#endif
                    if (dist > md)
                    {
                        md = dist;
                        candidate = it;
                    }
                }

                // If a point is found, set the include flag and handle segments in between recursively
                if (md > max_dist)
                {
#ifdef GL_DEBUG_SIMPLIFY
                    std::cout << "use " << candidate->p << std::endl;
#endif

                    candidate->included = true;
                    n++;

                    consider(begin, candidate + 1, max_dist, n, ps_distance_strategy);
                    consider(candidate, end, max_dist, n, ps_distance_strategy);
                }
            }


        public :

            typedef PSDS distance_strategy_type;

            /*!
                \brief Call simplification on an iterator pair
            */
            inline void simplify(const R& range, O_IT out, double max_distance) const
            {
                PSDS strategy;
                // Init the output, a vector of references to all points

                // Note Geometry Algorithms suggest here
                // http://geometryalgorithms.com/Archive/algorithm_0205/algorithm_0205.htm
                // to "STAGE 1: Vertex Reduction within max_distance of prior vertex cluster"
                // However, that is not correct: a vertex within the specified distance might be
                // excluded here, but might be a better candidate for final inclusion than the point before.

                std::vector<DP> ref_candidates(boost::begin(range), boost::end(range));

                // Include first and last point of line, they are always part of the line
                int n = 2;
                ref_candidates.front().included = true;
                ref_candidates.back().included = true;

                // Get points, recursively, including them if they are further away than the specified distance
                typedef typename PSDS::return_type RET;

                consider(boost::begin(ref_candidates), boost::end(ref_candidates),
                    make_distance_result<RET>(max_distance), n, strategy);

                // Copy included elements to the output  (might be changed using copy_if)
                for(typename std::vector<DP>::const_iterator it = boost::begin(ref_candidates);
                    it != boost::end(ref_candidates); it++)
                {
                    if (it->included)
                    {
                        *out = it->p;
                        out++;
                    }
                }
            }

        };

    }
}


} // namespace ggl

#endif // GGL_STRATEGY_AGNOSTIC_SIMPLIFY_HPP
