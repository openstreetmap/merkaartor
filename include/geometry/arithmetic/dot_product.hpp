// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_DOT_PRODUCT_HPP
#define _GEOMETRY_DOT_PRODUCT_HPP


#include <boost/concept/requires.hpp>
#include <geometry/core/concepts/point_concept.hpp>


namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		template <int I, int N>
		struct dot_product_maker
		{
			template <typename P1, typename P2>
			static typename coordinate_type<P1>::type
			run(const P1& p1, const P2& p2)
			{
				return get<I>(p1)*get<I>(p2)
				     + dot_product_maker<I+1, N>::run(p1, p2);
			}
		};

		template <int N>
		struct dot_product_maker<N, N>
		{
			template <typename P1, typename P2>
			static typename coordinate_type<P1>::type
			run(const P1& p1, const P2& p2)
			{
				return get<N>(p1)*get<N>(p2);
			}
		};

	} // namespace impl
	#endif


	/*!
		\brief Computes the dot product of 2 points
		\ingroup arithmetic
		\param p1 first point
		\param p2 second point
		\return the dot product
	 */
	template <typename P1, typename P2>
	BOOST_CONCEPT_REQUIRES(((ConstPoint<P1>)) ((ConstPoint<P2>)),
	(typename coordinate_type<P1>::type)) dot_product(const P1& p1, const P2& p2)
	{
		return impl::dot_product_maker<0, dimension<P1>::value - 1>::run(p1, p2);
	}

} // namespace geometry


#endif
