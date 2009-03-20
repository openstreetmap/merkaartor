// Generic Geometry Library
// Point Example - showing Cartesian points

#include <geometry/geometry.hpp>

#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/io/wkt/streamwkt.hpp>


int main()
{
	using namespace geometry;

	point_2d world(1, 2);
	std::cout << "Hello " << world << std::endl;
	std::cout << "Hello (" << world.x() << "," << world.y() << ")" << std::endl;

	// Construct two points using constructor with two values
	point_2d a(1, 2);
	point_2d b(5, 6);

	// The distance between them can be calculated
	std::cout << "distance a-b is " << distance(a,b) << std::endl;

	// Cartesian points have .x() and .y() for getting and setting coordinate values
	a.x(3);
	a.y(4);
	b.x(7);
	b.y(8);
	std::cout << "a:" << world.x() << " " << world.y() << std::endl;
	std::cout << "distance a-b is " << distance(a,b) << std::endl;

	// Several ways of construction:
	// 1: default constructor, required for every point
	point_2d c1;

	// 2: for Cartesian points: constructor with two values
	point_2d c2(1,1);

	// Until here, we assumed Cartesian points, having x,y coordinates. However, the Geometry Library
	// uses a coordinate agnostic approach. This means that internally there is never referred
	// to the name of coordinate axes. Points might also have more than 2 dimensions. Spherical
	// points (which normally have theta/phi) and geographic points (which are referred to by
	// latitude/longitude) are also possible.

	// Below we do not show all kind of points, but we show the coordinate agnostic ways of
	// constructing points, assigning values and retrieving values without .x/.y
	// Developers using Cartesian points can of course still use .x() and .y(), which is more readable.

	// 3: for any point, and other geometry objects: the "make" object generator
	//    (this one requires to specify the point-type). We say this is coordinate agnostic
	point_2d c3 = make<point_2d>(1,1);

	// 4: there is also an assign algorithm, in the same way as make, but not returning
	point_2d c4;
	assign(c4, 1, 1);

	// 5: there is also a parse algorithm which takes strings (not so useful here, but
	//    convenient for geographic points using "32N", "18E" etc).
	//point_2d c5 = parse<point_2d>("1", "1");

	// The coordinate agnostic way of getting and setting
	// without referring to .x() and .y()
	set<0>(a, 2); // set first coordinate to 2
	set<1>(a, 3); // set second coordinate to 3
	std::cout << get<0>(a) << "," << get<1>(a) << std::endl;

	// set<2>(a, 4); try to set third coordinate, this won't compile for a 2D point

	// Other examples show other types of points, geometries and more algorithms

	return 0;
}
