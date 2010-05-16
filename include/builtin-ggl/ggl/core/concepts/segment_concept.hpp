// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_CONCEPTS_SEGMENT_CONCEPT_HPP
#define GGL_CORE_CONCEPTS_SEGMENT_CONCEPT_HPP


#include <boost/concept_check.hpp>

#include <ggl/core/concepts/point_concept.hpp>



#include <ggl/core/access.hpp>
#include <ggl/core/point_type.hpp>



namespace ggl { namespace concept {


/*!
    \brief Checks segment concept, using Boost Concept Check Library and metafunctions
    \ingroup concepts
*/
template <typename S>
struct Segment
{
    private :
        typedef typename point_type<S>::type P;

        BOOST_CONCEPT_ASSERT( (concept::Point<P>) );


        /// Internal structure to check if access is OK for all dimensions
        template <size_t C, size_t D, size_t N>
        struct dimension_checker
        {
            static void check()
            {
                S* s;
                ggl::set<C, D>(*s, ggl::get<C, D>(*s));
                dimension_checker<C, D + 1, N>::check();
            }
        };

        template <size_t C, size_t N>
        struct dimension_checker<C, N, N>
        {
            static void check() {}
        };

    public :
        /// BCCL macro to check the Segment concept
        BOOST_CONCEPT_USAGE(Segment)
        {
            static const size_t N = dimension<P>::value;
            dimension_checker<0, 0, N>::check();
            dimension_checker<1, 0, N>::check();
        }
};


/*!
    \brief Checks Segment concept (const version)
    \ingroup concepts
    \details The ConstSegment concept check the same as the Segment concept,
    but does not check write access.
*/
template <typename S>
struct ConstSegment
{
    private :
        typedef typename point_type<S>::type P;
        typedef typename coordinate_type<S>::type T;

        BOOST_CONCEPT_ASSERT( (concept::ConstPoint<P>) );


        /// Internal structure to check if access is OK for all dimensions
        template <size_t C, size_t D, size_t N>
        struct dimension_checker
        {
            static void check()
            {
                const S* s = 0;
                T coord(ggl::get<C, D>(*s));
                boost::ignore_unused_variable_warning(coord);
                dimension_checker<C, D + 1, N>::check();
            }
        };

        template <size_t C, size_t N>
        struct dimension_checker<C, N, N>
        {
            static void check() {}
        };

    public :
        /// BCCL macro to check the ConstSegment concept
        BOOST_CONCEPT_USAGE(ConstSegment)
        {
            static const size_t N = dimension<P>::value;
            dimension_checker<0, 0, N>::check();
            dimension_checker<1, 0, N>::check();
        }
};


}} // namespace ggl::concept


#endif // GGL_CORE_CONCEPTS_SEGMENT_CONCEPT_HPP
