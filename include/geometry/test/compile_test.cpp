// OLD!


// Generic Geometry Library
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <deque>
#include <list>


#include <string>

#include <limits>

#include <iostream>
#include <sstream>

#include <geometry/algorithms/area.hpp>
//#include <geometry/astext.hpp>
#include <geometry/geometries/box.hpp>
#include <geometry/algorithms/centroid.hpp>
#include <geometry/geometries/circle.hpp>
#include <geometry/algorithms/correct.hpp>
#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/envelope.hpp>
#include <geometry/algorithms/foreach.hpp>
#include <geometry/algorithms/intersection.hpp>
#include <geometry/algorithms/length.hpp>
#include <geometry/overlaps.hpp>
#include <geometry/algorithms/simplify.hpp>
#include <geometry/algorithms/within.hpp>
#include <geometry/util/side.hpp>


template<typename P>
struct modifying_functor
{
	double sum;
	modifying_functor() : sum(0)
	{}
	inline void operator()(P& p)
	{
		p.x(1);
	}

	inline void operator()(geometry::segment<P>& s)
	{
		s.first.x(1);
	}
};

template<typename P>
struct const_functor
{
	double sum;
	const_functor() : sum(0)
	{}
	inline void operator()(const P& p)
	{
		sum += p.x();
	}

	inline void operator()(const geometry::const_segment<P>& s)
	{
		sum += s.first.x() - s.second.x();
	}
};


template <typename T, template<typename,typename> class V>
void check_linestring()
{
	typedef geometry::point<T> P;
	typedef geometry::linestring<P, V, std::allocator> L;
	L line;
	line.push_back(P(0,0));
	line.push_back(P(1,1));

	typedef geometry::multi_linestring<L, V, std::allocator> ML;
	ML multi;
	multi.push_back(line);

	double len = geometry::length(line);
	len = geometry::length(multi);
	double d = geometry::distance(P(0,1), line);
	//d = geometry::distance(P(0,1), multi); not defined yet!

	L simp;
	geometry::simplify(line, simp, 3);
	ML simpm;
	geometry::simplify(multi, simpm, 3);

	typedef geometry::box<P> B;
	B b = geometry::envelope(line);
	b = geometry::envelope(multi);

	std::stringstream out;
	out << line << std::endl;
	//out << multi << std::endl;

	// For each, const
	const_functor<P> cf;
	std::for_each(line.begin(), line.end(), cf);

	const L& cl = line;
	const ML& cm = multi;

	geometry::for_each_point(cl, cf);
	geometry::for_each_point(cm, cf);
	geometry::for_each_segment(cl, cf);
	geometry::for_each_segment(cm, cf);

	// For each, modifying
	modifying_functor<P> mf;
	L& ml = line;
	ML& mm = multi;
	std::for_each(line.begin(), line.end(), mf);
	geometry::for_each_point(ml, mf);
	geometry::for_each_point(mm, mf);
	geometry::for_each_segment(ml, mf);
	geometry::for_each_segment(mm, mf);
}


template <typename T, template<typename,typename> class VP,
			template<typename,typename> class VR>
void check_polygon()
{
	typedef geometry::point<T> P;
	typedef geometry::polygon<P, VP, VR, std::allocator, std::allocator> Y;
	Y poly;
	poly.outer().push_back(P(0,0));
	poly.outer().push_back(P(2,0));
	poly.outer().push_back(P(2,2));
	poly.outer().push_back(P(0,2));

	geometry::correct(poly);

	// multi
	typedef geometry::multi_polygon<Y, VP, std::allocator> MY;
	MY multi;
	multi.push_back(poly);

	double a = geometry::area(poly);
	a = geometry::area(multi);

	//double d = geometry::distance(P(0,1), poly);

	Y simp;
	geometry::simplify(poly, simp, 3);
	MY msimp;
	geometry::simplify(multi, msimp, 3);

	typedef geometry::box<P> B;
	B b = geometry::envelope(poly);
	b = geometry::envelope(multi);
	P ctr = geometry::centroid(poly);

	// within
	typedef geometry::circle<P, T> C;
	C circ(P(10,10), 10);

	bool w = geometry::within(P(1, 1), poly);
	w = geometry::within(poly, circ);
	//w = geometry::within(poly, b); tbd
	w = geometry::within(P(1, 1), multi);
	w = geometry::within(multi, circ);
	//w = geometry::within(multi, b); tbd

	// For each, const
	const_functor<P> cf;
	std::for_each(poly.outer().begin(), poly.outer().end(), cf);

	const Y& cp = poly;
	const MY& cm = multi;

	geometry::for_each_point(cp, cf);
	geometry::for_each_point(cm, cf);
	geometry::for_each_segment(cp, cf);
	geometry::for_each_segment(cm, cf);

	// For each, modifying
	modifying_functor<P> mf;
	Y& mp = poly;
	MY& mm = multi;
	std::for_each(poly.outer().begin(), poly.outer().end(), mf);
	geometry::for_each_point(mp, mf);
	geometry::for_each_point(mm, mf);
	geometry::for_each_segment(mp, mf);
	geometry::for_each_segment(mm, mf);
}


int main()
{
	check_linestring<double, std::vector>();
	check_linestring<float, std::vector>();
	check_linestring<int, std::vector>();
	check_linestring<char, std::vector>();

	check_linestring<double, std::list>();
	check_linestring<double, std::deque>();

	check_polygon<double, std::vector, std::vector>();
	check_polygon<float, std::vector, std::vector>();
	check_polygon<int, std::vector, std::vector>();
	check_polygon<char, std::vector, std::vector>();

	check_polygon<double, std::list, std::vector>();
	check_polygon<double, std::deque, std::vector>();
	check_polygon<double, std::list, std::list>();
	check_polygon<double, std::deque, std::deque>();

	return 0;
}

