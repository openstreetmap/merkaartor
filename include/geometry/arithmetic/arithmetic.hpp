// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_ARITHMETIC_HPP
#define _GEOMETRY_ARITHMETIC_HPP


#include <functional>

#include <boost/call_traits.hpp>
#include <boost/concept/requires.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/util/for_each_coordinate.hpp>


/*!
\defgroup arithmetic arithmetic: arithmetic operations on points
*/



namespace geometry
{
	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		template <typename P>
		struct param
		{
			typedef typename boost::call_traits
			                 <typename coordinate_type<P>::type>
			                 ::param_type type;
		};


		template <typename C, template <typename> class Function>
		struct value_operation
		{
			C m_value;
			value_operation(C value): m_value(value) {}

			template <typename P, int I>
			void run(P& point) const
			{ set<I>(point, Function<C>()(get<I>(point), m_value)); }
		};

		template <typename PS, template <typename> class Function>
		struct point_operation
		{
			typedef typename coordinate_type<PS>::type T;
			const PS& m_source_point;
			point_operation(const PS& point): m_source_point(point) {}

			template <typename PD, int I>
			void run(PD& dest_point) const
			{
				set<I>(dest_point, Function<T>()(get<I>(dest_point), get<I>(m_source_point)));
			}
		};

	} // namespace impl
	#endif


	/*!
		\brief Adds a value to each coordinate of a point
		\ingroup arithmetic
		\details
		\param p point
		\param value value to add
	 */
	template <typename P>
	BOOST_CONCEPT_REQUIRES(((Point<P>)),
	(void)) add_value(P& p, typename impl::param<P>::type value)
	{ for_each_coordinate(p, impl::value_operation<typename coordinate_type<P>::type, std::plus>(value)); }

	/*!
		\brief Adds a point to another
		\ingroup arithmetic
		\details The coordinates of the second point will be added to those of the first point. The second point is not modified.
		\param p1 first point
		\param p2 second point
	 */
	template <typename P1, typename P2>
	BOOST_CONCEPT_REQUIRES(((Point<P1>)) ((ConstPoint<P2>)),
	(void)) add_point(P1& p1, const P2& p2)
	{ for_each_coordinate(p1, impl::point_operation<P2, std::plus>(p2)); }



	/*!
		\brief Subtracts a value to each coordinate of a point
		\ingroup arithmetic
		\details
		\param p point
		\param value value to subtract
	 */
	template <typename P>
	BOOST_CONCEPT_REQUIRES(((Point<P>)),
	(void)) subtract_value(P& p, typename impl::param<P>::type value)
	{ for_each_coordinate(p, impl::value_operation<typename coordinate_type<P>::type, std::minus>(value)); }

	/*!
		\brief Subtracts a point to another
		\ingroup arithmetic
		\details The coordinates of the second point will be subtracted to those of the first point. The second point is not modified.
		\param p1 first point
		\param p2 second point
	 */
	template <typename P1, typename P2>
	BOOST_CONCEPT_REQUIRES(((Point<P1>)) ((ConstPoint<P2>)),
	(void)) subtract_point(P1& p1, const P2& p2)
	{ for_each_coordinate(p1, impl::point_operation<P2, std::minus>(p2)); }



	/*!
		\brief Multiplies each coordinate of a point by a value
		\ingroup arithmetic
		\details
		\param p point
		\param value value to multiply by
	 */
	template <typename P>
	BOOST_CONCEPT_REQUIRES(((Point<P>)),
	(void)) multiply_value(P& p, typename impl::param<P>::type value)
	{ for_each_coordinate(p, impl::value_operation<typename coordinate_type<P>::type, std::multiplies>(value)); }

	/*!
		\brief Multiplies a point by another
		\ingroup arithmetic
		\details The coordinates of the second point will be multiplied by those of the first point. The second point is not modified.
		\param p1 first point
		\param p2 second point
		\note This is *not* a dot, cross or wedge product. It is a mere field-by-field multiplication.
	 */
	template <typename P1, typename P2>
	BOOST_CONCEPT_REQUIRES(((Point<P1>)) ((ConstPoint<P2>)),
	(void)) multiply_point(P1& p1, const P2& p2)
	{ for_each_coordinate(p1, impl::point_operation<P2, std::multiplies>(p2)); }



	/*!
		\brief Divides each coordinate of a point by a value
		\ingroup arithmetic
		\details
		\param p point
		\param value value to divide by
	 */
	template <typename P>
	BOOST_CONCEPT_REQUIRES(((Point<P>)),
	(void)) divide_value(P& p, typename impl::param<P>::type value)
	{ for_each_coordinate(p, impl::value_operation<typename coordinate_type<P>::type, std::divides>(value)); }

	/*!
		\brief Divides a point by another
		\ingroup arithmetic
		\details The coordinates of the second point will be divided by those of the first point. The second point is not modified.
		\param p1 first point
		\param p2 second point
	 */
	template <typename P1, typename P2>
	BOOST_CONCEPT_REQUIRES(((Point<P1>)) ((ConstPoint<P2>)),
	(void)) divide_point(P1& p1, const P2& p2)
	{ for_each_coordinate(p1, impl::point_operation<P2, std::divides>(p2)); }


} // namespace geometry


#endif
