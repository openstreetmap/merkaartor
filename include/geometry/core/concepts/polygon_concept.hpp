// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding P.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GEOMETRY_POLYGON_CONCEPT_HPP
#define _GEOMETRY_POLYGON_CONCEPT_HPP


#include <boost/concept_check.hpp>
#include <boost/range/concepts.hpp>

#include <geometry/core/concepts/point_concept.hpp>
#include <geometry/core/concepts/ring_concept.hpp>



#include <geometry/core/access.hpp>
#include <geometry/core/point_type.hpp>
#include <geometry/core/ring_type.hpp>
#include <geometry/core/exterior_ring.hpp>
#include <geometry/core/interior_rings.hpp>


#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>


namespace geometry
{

	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		template <typename P>
		struct PolygonChecker
		{
			typedef typename point_type<P>::type PNT;
			typedef typename ring_type<P>::type R;
			typedef typename interior_type<P>::type I;

			BOOST_CONCEPT_ASSERT( (boost::RandomAccessRangeConcept<I>) );

			void constraints()
			{

				P* poly;
				R& e = exterior_ring(*poly);
				const R& ce = exterior_ring(*poly);

				I& i = interior_rings(*poly);
				const I& ci = interior_rings(*poly);
			}

		};
	}
	#endif // DOXYGEN_NO_IMPL

	/*!
		\brief Checks polygon concept, using Boost Concept Check Library and metafunctions
		\ingroup concepts
	*/
	template <typename P>
	struct Polygon : impl::PolygonChecker<P>
	{
		private :
			typedef typename point_type<P>::type PNT;
			typedef typename ring_type<P>::type R;
			typedef typename interior_type<P>::type I;

			BOOST_CONCEPT_ASSERT( (Point<PNT>) );
			BOOST_CONCEPT_ASSERT( (Ring<R>) );


		public :
			/// BCCL macro to check the Polygon concept
			BOOST_CONCEPT_USAGE(Polygon)
			{
				// Check if it can be modified
				P* poly;
				clear(*poly);
				append(*poly, PNT());

				this->constraints();

			}
	};


	/*!
		\brief Checks Polygon concept (const version)
		\ingroup concepts
		\details The ConstPolygon concept check the same as the Polygon concept,
		but does not check write access.
	*/
	template <typename P>
	struct ConstPolygon : impl::PolygonChecker<P>
	{
		private :
			typedef typename point_type<P>::type PNT;
			typedef typename ring_type<P>::type R;
			typedef typename interior_type<P>::type I;

			BOOST_CONCEPT_ASSERT( (ConstPoint<PNT>) );
			BOOST_CONCEPT_ASSERT( (ConstRing<R>) );


		public :
			/// BCCL macro to check the ConstPolygon concept
			BOOST_CONCEPT_USAGE(ConstPolygon)
			{
				this->constraints();
			}
	};
}


#endif
