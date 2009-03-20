// Generic Geometry Library
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <string>
#include <deque>
#include <list>
#include <limits>

#include <cmath>

#include <geometry/geometries/box.hpp>
#include <geometry/geometries/circle.hpp>
#include <geometry/algorithms/envelope.hpp>
#include <geometry/algorithms/within.hpp>
#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/length.hpp>
#include <geometry/overlaps.hpp>
#include <geometry/algorithms/correct.hpp>
#include <geometry/algorithms/centroid.hpp>

#include <geometry/algorithms/intersection_segment.hpp>
#include <geometry/algorithms/intersection_linestring.hpp>
#include <geometry/algorithms/intersection_polygon.hpp>

#include <geometry/astext.hpp>
#include <geometry/point_on_line.hpp>


void test_equals()
{
	geometry::point<double> p1(1, 1), p2(1,1 + std::numeric_limits<double>::epsilon() * 2.0);
	geometry::point<int> p3(1,1), p4(1,1);

	std::cout << (p1 == p2 ? "true" : "false" ) << std::endl;
	std::cout << (p3 == p4 ? "true" : "false" ) << std::endl;
}




void test(double x1, double y1, double x2, double y2)
{
	//geometry::box<geometry::point<double> > e(70,60,   230,150);
	geometry::box<geometry::point<double> > e(0.30, 0.30, 0.70, 0.70);
	geometry::point<double> p1(x1, y1);
	geometry::point<double> p2(x2, y2);

	bool c1, c2;
	geometry::segment<geometry::point<double> >s(p1, p2);
	if (geometry::_clip_segment_liang_barsky(e, s, c1, c2))
	{
		if (c1) std::cout << "p1 clipped ";
		if (c2) std::cout << "p2 clipped ";
		std::cout
			 <<  p1.x()  <<  ", "
			 <<  p1.y()  <<  ", "
			 <<  p2.x()  <<  ", "
			 <<  p2.y()  <<  std::endl;
	}
	else
	{
		std::cout << "not visible" << std::endl;
	}
	std::cout << std::endl;
}

void test_line(const geometry::linestring<geometry::point<double> >& line)
{
	//geometry::box<geometry::point<double> > e(70,60,   230,150);
	geometry::box<geometry::point<double> > e(0.30, 0.30, 0.70, 0.70);

	geometry::multi_linestring<geometry::linestring<geometry::point<double> >> out;

	geometry::clip_linestring(e, line, out);
	for (int i = 0; i < out.size(); i++)
	{
		const geometry::linestring<geometry::point<double> >& ol = out[i];
		for (int j = 0; j < ol.size(); j++)
		{
			const geometry::point<double>& p = ol[j];
			std::cout << "("
				 <<  p.x()  <<  ", "
				 <<  p.y()  <<  ") " ;

		}
		std::cout << std::endl;
	}
}

void test_ring(const geometry::linear_ring<geometry::point<double> >& linear_ring)
{
	std::cout << "linear_ring" << std::endl;
	// normal: geometry::box<geometry::point<double> > e(0.30, 0.25, 0.70, 0.75); -> 0.01, 0.06
	// difficult 1: geometry::box<geometry::point<double> > e(0.30, 0.30, 0.70, 0.70); --> 0.05
	// difficult 2: geometry::box<geometry::point<double> > e(0.50, 0.30, 0.70, 0.70);
	geometry::box<geometry::point<double> > e(0.50, 0.30, 0.70, 0.70);

	std::cout << "linear_ring: " << linear_ring << std::endl;

	std::vector<geometry::linear_ring<geometry::point<double> >> v;
	clip_poly_weiler_atherton(e, linear_ring, v);

	for (int i = 0; i < v.size(); i++)
	{
		//set_ordered(v[i]);
		std::cout << "linear_ring " << i+1 << ": " << v[i] << std::endl;
	}
}



void test_side(double x1, double y1, double x2, double y2, double x, double y)
{
	typedef geometry::point<double> glp ;
	typedef geometry::const_segment<glp> gls;
	glp p1(x1,y1);
	glp p2(x2,y2);
	glp p(x,y);
	double r = geometry::point_side(gls(p1, p2), p);
	std::cout << r << " " << p1 << "-" << p2 << ": " << p << std::endl;
}





int main1(void)
{
	test_equals();

	test_side(0,1, 2,1, 0,0);
	test_side(0,1, 2,1, 1,0);
	test_side(0,1, 2,1, 1,2);
	test_side(0,1, 2,1, 2,1);
	test_side(0,1, 2,1, 3,1);

/*
	test_intersection(0,0, 0,2,  0,1, 1,1,          1,  0,1, 0,0);
	test_intersection(0,1, 1,1,  0,0, 0,2,          1,  0,1, 0,0);
	test_intersection(0,2, 2,2,  1,1, 1,2,          1,  1,2, 0,0);
	test_intersection(1,1, 1,2,  0,2, 2,2,          1,  1,2, 0,0);
	test_intersections();
	*/
	//return 0;


	//geometry::point<double> p1(30,20); geometry::point<double> p2(280,160);
	//geometry::box<geometry::point<double> > e(0.30, 0.30, 0.70, 0.70);
	//geometry::point<double> p1(0.052, 0.674); geometry::point<double> p2(0.902, 0.744);
	//geometry::point<double> p1(0.4, 0.4); geometry::point<double> p2(0.6, 0.6);
	test(0.4, 0.4, 0.6, 0.6); // inside
	test(0.052, 0.674, 0.902, 0.744); // both clipped
	test(0.45, 0.45, 0.15, 0.15); // second clipped
	test(0.5, 0.8, 0.4, 0.4); // first clipped
	test(0.0, 0.0, 0.5, 0.5); // first clipped

	geometry::linestring<geometry::point<double> > l;
	l.push_back(geometry::point<double>(0.0, 0.0));
	l.push_back(geometry::point<double>(0.5, 0.5));
	l.push_back(geometry::point<double>(1.0, 0.0));
	test_line(l);

	l.clear();
	l.push_back(geometry::point<double>(0.1, 0.0));
	l.push_back(geometry::point<double>(0.5, 0.8));
	l.push_back(geometry::point<double>(0.9, 0.0));
	test_line(l);

	l.clear();
	l.push_back(geometry::point<double>(0.5, 0.5));
	l.push_back(geometry::point<double>(0.3, 0.3));
	l.push_back(geometry::point<double>(0.4, 0.7));
	l.push_back(geometry::point<double>(0.7, 0.3));
	test_line(l);

	geometry::linear_ring<geometry::point<double> > r;
	r.push_back(geometry::point<double>(0.5, 0.5));
	r.push_back(geometry::point<double>(0.8, 0.71));
	r.push_back(geometry::point<double>(0.9, 0.6));
	r.push_back(geometry::point<double>(0.6, 0.2));
	r.push_back(geometry::point<double>(0.5, 0.5));
//	test_ring(r);

	r.clear();
	r.push_back(geometry::point<double>(0.5, 0.2));
	r.push_back(geometry::point<double>(0.5, 0.3));
	r.push_back(geometry::point<double>(0.8, 0.3));
	r.push_back(geometry::point<double>(0.8, 0.4));
	r.push_back(geometry::point<double>(0.5, 0.4));
	r.push_back(geometry::point<double>(0.5, 0.8));
	r.push_back(geometry::point<double>(0.9, 0.8));

	// case x
	r.push_back(geometry::point<double>(0.9, 0.7));
	r.push_back(geometry::point<double>(0.6, 0.7));
	r.push_back(geometry::point<double>(0.6, 0.6));
	r.push_back(geometry::point<double>(0.9, 0.6));

	// end case x

	r.push_back(geometry::point<double>(0.9, 0.2));
	r.push_back(geometry::point<double>(0.5, 0.2));
	test_ring(r);

	return 0;
}