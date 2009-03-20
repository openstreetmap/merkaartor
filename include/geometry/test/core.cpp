// Generic Geometry Library test file
//
// Copyright Barend Gehrels, 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/test/included/test_exec_monitor.hpp>

#include <boost/concept/assert.hpp>

#include <geometry/core/topological_dimension.hpp>

#include <geometry/geometries/geometries.hpp>

#include <geometry/core/concepts/point_concept.hpp>


#include "common.hpp"

template <typename G, size_t D, size_t TOPDIM>
void test_geometry()
{

	// metafunctions which are there for all types
	typedef typename geometry::tag<G>::type tag;
	typedef typename geometry::coordinate_type<G>::type ctype;
	typedef typename geometry::coordinate_system<G>::type csystem;
	typedef typename geometry::point_type<G>::type ptype;
	static const int dim = geometry::dimension<G>::value;
	static const int topdim = geometry::topological_dimension<G>::value;

	//std::cout << sizeof(ctype) << std::endl;

	// Get the coordinate system tag
	typedef typename geometry::cs_tag<G>::type ctag;

	BOOST_CHECK_EQUAL(dim, D);
	BOOST_CHECK_EQUAL(topdim, TOPDIM);

	BOOST_STATIC_ASSERT((boost::mpl::equal_to<
					geometry::dimension<G>,
					boost::mpl::int_<D> >::type::value));

	BOOST_CONCEPT_ASSERT(( geometry::ConstPoint<ptype> ));
}


template <typename P, size_t D>
void test_all()
{
	test_geometry<P, D, 0>();
	test_geometry<geometry::linestring<P> , D, 1>();
	test_geometry<geometry::linear_ring<P> , D, 2>();
	test_geometry<geometry::polygon<P> , D, 2>();
	test_geometry<geometry::box<P> , D, 2>();
	test_geometry<geometry::segment<P> , D, 1>();
	test_geometry<geometry::segment<const P> , D, 1>();
	test_geometry<geometry::nsphere<P, double> , D, 2>();
}



int test_main(int, char* [])
{
	test_geometry<int[2], 2, 0>();
	test_geometry<float[2], 2, 0>();
	test_geometry<double[2], 2, 0>();

	test_geometry<int[3], 3, 0>();
	test_geometry<float[3], 3, 0>();
	test_geometry<double[3], 3, 0>();

	test_all<test_point, 3>();
	test_all<geometry::point<int, 2, geometry::cs::cartesian>, 2 >();
	test_all<geometry::point<float, 2, geometry::cs::cartesian>, 2 >();
	test_all<geometry::point<double, 2, geometry::cs::cartesian>, 2 >();

	return 0;
}
