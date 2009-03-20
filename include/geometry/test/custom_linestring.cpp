// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <boost/concept/requires.hpp>

#include <geometry/core/access.hpp>
#include <geometry/algorithms/make.hpp>

#include <geometry/geometries/geometries.hpp>
#include <geometry/geometries/adapted/std_as_linestring.hpp>
#include <geometry/geometries/adapted/boost_array_as_linestring.hpp>

#include <geometry/algorithms/clear.hpp>
#include <geometry/algorithms/append.hpp>


#include <geometry/core/concepts/linestring_concept.hpp>

#include <geometry/io/wkt/streamwkt.hpp>


#include "common.hpp"


// ----------------------------------------------------------------------------
// First custom linestring, requires ONLY one traits: to register itself as a linestring
template <typename P>
struct custom_linestring1 : std::vector<P> {};


namespace geometry { namespace traits {
	template <typename P>
	struct tag< custom_linestring1<P> > { typedef linestring_tag type; };
}}


// ----------------------------------------------------------------------------
// Second custom linestring, decides to implement all edit operations itself
// by specializing the "use_std" traits to false.
// It should therefore implement the traits:: clear / append_point
template <typename P>
struct custom_linestring2 : std::deque<P> {};

namespace geometry { namespace traits {
	template <typename P>
	struct tag< custom_linestring2<P> > { typedef linestring_tag type; };

	template <typename P>
	struct use_std< custom_linestring2<P> >
	{
		static const bool value = false;
	};

	template <typename P>
	struct clear< custom_linestring2<P> >
	{
		static inline void run(custom_linestring2<P>& ls) { ls.resize(0); }
	};

	template <typename P>
	struct append_point< custom_linestring2<P>, P>
	{
		static inline void run(custom_linestring2<P>& geometry,
						const P& point, int ring_index, int multi_index)
		{
			// does not use push-back but something else.
			geometry.insert(geometry.end(), point);
		}
	};
}}

// ----------------------------------------------------------------------------


template <typename G>
void test_linestring()
{
	BOOST_CONCEPT_ASSERT((geometry::Linestring<G>));
	BOOST_CONCEPT_ASSERT((geometry::ConstLinestring<G>));

	G geometry;
	typedef typename geometry::point_type<G>::type P;

	geometry::clear(geometry);
	BOOST_CHECK_EQUAL(boost::size(geometry), 0);

	geometry::append(geometry, geometry::make_zero<P>());
	BOOST_CHECK_EQUAL(boost::size(geometry), 1);

	//std::cout << geometry << std::endl;

	geometry::clear(geometry);
	BOOST_CHECK_EQUAL(boost::size(geometry), 0);


	//P p = boost::range::front(geometry);
}




template <typename P>
void test_all()
{
	test_linestring<geometry::linestring<P> >();
	test_linestring<geometry::linestring<P, std::vector> >();
	test_linestring<geometry::linestring<P, std::deque> >();

	test_linestring<custom_linestring1<P> >();
	test_linestring<custom_linestring2<P> >();

	test_linestring<std::vector<P> >();
	test_linestring<std::deque<P> >();
	//test_linestring<std::list<P> >();
}



int test_main(int, char* [])
{
	test_all<test_point>();
	test_all<boost::tuple<float, float> >();
	test_all<geometry::point<int, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<float, 2, geometry::cs::cartesian> >();
	test_all<geometry::point<double, 2, geometry::cs::cartesian> >();

 	return 0;
}
