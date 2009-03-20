// Generic Geometry Library
// Custom point Example

#include <geometry/algorithms/make.hpp>
#include <geometry/algorithms/within.hpp>
#include <geometry/geometries/register/register_point.hpp>
#include <geometry/geometries/register/register_box.hpp>

#include <geometry/io/wkt/streamwkt.hpp>


struct my_point
{
	double x, y;
};

struct my_int_point
{
	int x, y;
};


struct my_box
{
	my_point ll, ur;
};

struct my_box_ltrb
{
	int left, top, right, bottom;
};

struct my_box_4
{
	double coors[4];
};


template <typename P>
struct my_box_t
{
	P ll, ur;
};


GEOMETRY_REGISTER_POINT_2D(my_point, double, cs::cartesian, x, y)
GEOMETRY_REGISTER_POINT_2D(my_int_point, int, cs::cartesian, x, y)

GEOMETRY_REGISTER_BOX(my_box, my_point, ll, ur)

GEOMETRY_REGISTER_BOX_TEMPLATIZED(my_box_t, P, ll, ur)

GEOMETRY_REGISTER_BOX_2D_4VALUES(my_box_ltrb, my_int_point, left, top, right, bottom)

GEOMETRY_REGISTER_BOX_2D_4VALUES(my_box_4, my_point, coors[0], coors[1], coors[2], coors[3])


int main()
{
	my_point p = geometry::make<my_point>(3.5, 3.5);
	my_box b = geometry::make<my_box>(0, 0, 2, 2);
	my_box_ltrb b1 = geometry::make<my_box_ltrb>(0, 0, 3, 3);
	my_box_4 b4 = geometry::make<my_box_4>(0, 0, 4, 4);
	my_box_t<my_point> bt = geometry::make<my_box_t<my_point> >(0, 0, 5, 5);

	std::cout << p << " IN " << b << " : " << int(geometry::within(p, b)) << std::endl;
	std::cout << p << " IN " << b1 << " : " << int(geometry::within(p, b1)) << std::endl;
	std::cout << p << " IN " << b4 << " : " << int(geometry::within(p, b4)) << std::endl;
	std::cout << p << " IN " << bt << " : " << int(geometry::within(p, bt)) << std::endl;

	return 0;
}
