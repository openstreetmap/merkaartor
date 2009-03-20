// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding L.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_LINESTRING_CONCEPT_HPP
#define _GEOMETRY_LINESTRING_CONCEPT_HPP


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
	template <typename L>
	struct Linestring
	{
		private :
			typedef typename point_type<L>::type P;

			BOOST_CONCEPT_ASSERT( (Point<P>) );
			BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<L>) );

		public :
			/// BCCL macro to check the Linestring concept
			BOOST_CONCEPT_USAGE(Linestring)
			{

				// Check if it can be modified
				L* ls;
				clear(*ls);
				append(*ls, P());
			}
	};


	/*!
		\brief Checks Linestring concept (const version)
		\ingroup concepts
		\details The ConstLinestring concept check the same as the Linestring concept,
		but does not check write access.
	*/
	template <typename L>
	struct ConstLinestring
	{
		private :
			typedef typename point_type<L>::type P;

			BOOST_CONCEPT_ASSERT( (ConstPoint<P>) );
			BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<L>) );


		public :
			/// BCCL macro to check the ConstLinestring concept
			BOOST_CONCEPT_USAGE(ConstLinestring)
			{
			}
	};
}


#endif
