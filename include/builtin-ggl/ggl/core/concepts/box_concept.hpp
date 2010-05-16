// Generic Geometry Library
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef GGL_CORE_CONCEPTS_BOX_CONCEPT_HPP
#define GGL_CORE_CONCEPTS_BOX_CONCEPT_HPP


#include <boost/concept_check.hpp>


#include <ggl/core/access.hpp>
#include <ggl/core/point_type.hpp>



namespace ggl { namespace concept {


/*!
    \brief Checks box concept, using Boost Concept Check Library and metafunctions
    \ingroup concepts
*/
template <typename B>
struct Box
{
    private :
        typedef typename point_type<B>::type P;

        /// Internal structure to check if access is OK for all dimensions
        template <size_t C, size_t D, size_t N>
        struct dimension_checker
        {
            static void check()
            {
                B* b;
                ggl::set<C, D>(*b, ggl::get<C, D>(*b));
                dimension_checker<C, D + 1, N>::check();
            }
        };

        template <size_t C, size_t N>
        struct dimension_checker<C, N, N>
        {
            static void check() {}
        };

    public :
        /// BCCL macro to check the Box concept
        BOOST_CONCEPT_USAGE(Box)
        {
            static const size_t N = dimension<B>::value;
            dimension_checker<min_corner, 0, N>::check();
            dimension_checker<max_corner, 0, N>::check();
        }
};


/*!
    \brief Checks Box concept (const version)
    \ingroup concepts
    \details The ConstBox concept check the same as the Box concept,
    but does not check write access.
*/
template <typename B>
struct ConstBox
{
    private :
        typedef typename point_type<B>::type P;
        typedef typename coordinate_type<B>::type T;

        /// Internal structure to check if access is OK for all dimensions
        template <size_t C, size_t D, size_t N>
        struct dimension_checker
        {
            static void check()
            {
                const B* b = 0;
                T coord(ggl::get<C, D>(*b));
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
        /// BCCL macro to check the ConstBox concept
        BOOST_CONCEPT_USAGE(ConstBox)
        {
            static const size_t N = dimension<B>::value;
            dimension_checker<min_corner, 0, N>::check();
            dimension_checker<max_corner, 0, N>::check();
        }
};

}} // namespace ggl::concept


#endif // GGL_CORE_CONCEPTS_BOX_CONCEPT_HPP
