// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding R.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_LINEAR_RING_CONCEPT_HPP
#define _GEOMETRY_LINEAR_RING_CONCEPT_HPP


#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <geometry/core/concepts/point_concept.hpp>



#include <geometry/core/access.hpp>
#include <geometry/core/point_type.hpp>

#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>



namespace geometry
{


	/*!
		\brief Checks linestring concept, using Boost Concept Check Library and metafunctions
		\ingroup concepts
	*/
	template <typename R>
	struct Ring
	{
		private :
			typedef typename point_type<R>::type P;

			BOOST_CONCEPT_ASSERT( (Point<P>) );
			BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<R>) );


		public :
			/// BCCL macro to check the Ring concept
			BOOST_CONCEPT_USAGE(Ring)
			{
				// Check if it can be modified
				R* ls;
				clear(*ls);
				append(*ls, P());
			}
	};


	/*!
		\brief Checks Ring concept (const version)
		\ingroup concepts
		\details The ConstLinearRing concept check the same as the Ring concept,
		but does not check write access.
	*/
	template <typename R>
	struct ConstRing
	{
		private :
			typedef typename point_type<R>::type P;

			BOOST_CONCEPT_ASSERT( (ConstPoint<P>) );
			BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<R>) );

		public :
			/// BCCL macro to check the ConstLinearRing concept
			BOOST_CONCEPT_USAGE(ConstRing)
			{
			}
	};
}


#endif
