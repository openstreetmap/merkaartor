// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_UTIL_COPY_HPP
#define GGL_UTIL_COPY_HPP

#include <cstddef>

#include <boost/concept/requires.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/concept_check.hpp>

#include <ggl/core/concepts/point_concept.hpp>

// TODO: merge with "assign"

namespace ggl
{

#ifndef DOXYGEN_NO_IMPL
namespace impl { namespace copy {

template <typename Src, typename Dst, std::size_t D, std::size_t N>
struct copy_coordinates
{
    static inline void copy(const Src& source, Dst& dest)
    {
        typedef typename coordinate_type<Dst>::type coordinate_type;

        set<D>(dest, boost::numeric_cast<coordinate_type>(get<D>(source)));
        copy_coordinates<Src, Dst, D + 1, N>::copy(source, dest);
    }
};

template <typename Src, typename Dst, std::size_t N>
struct copy_coordinates<Src, Dst, N, N>
{
    static inline void copy(const Src& source, Dst& dest)
    {
        boost::ignore_unused_variable_warning(source);
        boost::ignore_unused_variable_warning(dest);
	}
};

}} // namespace impl::copy
#endif // DOXYGEN_NO_IMPL


/*!
    \brief Copies coordinates from source to destination point
    \ingroup assign
    \details The function copy_coordinates copies coordinates from one point to another point.
    Source point and destination point might be of different types.
    \param source Source point
    \param dest Destination point
    \note If destination type differs from source type, they must have the same coordinate count
 */
template <typename Src, typename Dst>
BOOST_CONCEPT_REQUIRES(((concept::ConstPoint<Src>)) ((concept::Point<Dst>)),
(void)) copy_coordinates(const Src& source, Dst& dest)
{
    //assert_dimension_equal<Dst, Src>();
    impl::copy::copy_coordinates<Src, Dst, 0, dimension<Src>::value>::copy(source, dest);
}

} // namespace ggl

#endif // GGL_UTIL_COPY_HPP
