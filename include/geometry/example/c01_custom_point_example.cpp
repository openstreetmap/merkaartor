// Generic Geometry Library
// Custom point Example

#include <geometry/algorithms/distance.hpp>
#include <geometry/algorithms/make.hpp>
#include <geometry/geometries/register/register_point.hpp>

#include <geometry/io/wkt/streamwkt.hpp>


// Sample point, defining three color values
struct my_color_point
{
	double red, green, blue;
};


// Sample point, having an int array defined
struct my_array_point
{
	int c[3];
};


// Sample point, having x/y
struct my_2d
{
	float x,y;
};

// Sample class, protected and construction-time-only x/y,
// Can (of course) only used in algorithms which take const& points
class my_class_ro
{
	public :
		my_class_ro(double x, double y) : _x(x), _y(y) {}
		double x() const { return _x; }
		double y() const { return _y; }
	private :
		double _x, _y;
};


// Sample class using references for read/write
class my_class_rw
{
	public :
		const double& x() const { return _x; }
		const double& y() const { return _y; }
		double& x() { return _x; }
		double& y() { return _y; }
	private :
		double _x, _y;
};

// Sample class using getters / setters
class my_class_gs
{
	public :
		const double get_x() const { return _x; }
		const double get_y() const { return _y; }
		void set_x(double v) { _x = v; }
		void set_y(double v) { _y = v; }
	private :
		double _x, _y;
};




GEOMETRY_REGISTER_POINT_3D(my_color_point, double, cs::cartesian, red, green, blue)

GEOMETRY_REGISTER_POINT_3D(my_array_point, int, cs::cartesian, c[0], c[1], c[2])

GEOMETRY_REGISTER_POINT_2D(my_2d, float, cs::cartesian, x, y)

GEOMETRY_REGISTER_POINT_2D_CONST(my_class_ro, double, cs::cartesian, x(), y())

GEOMETRY_REGISTER_POINT_2D(my_class_rw, double, cs::cartesian, x(), y())

GEOMETRY_REGISTER_POINT_2D_GET_SET(my_class_gs, double, cs::cartesian, get_x, get_y, set_x, set_y)




int main()
{
	// Create 2 instances of our custom color point
	my_color_point c1 = geometry::make<my_color_point>(255, 3, 233);
	my_color_point c2 = geometry::make<my_color_point>(0, 50, 200);

	// The distance between them can be calculated using the cartesian method (=pythagoras)
	// provided with the library, configured by the coordinate_system type of the point
	std::cout << "color distance " << c1 << " to " << c2 << " is " << geometry::distance(c1,c2) << std::endl;

	my_array_point a1 = {0};
	my_array_point a2 = {0};
	geometry::assign(a1, 1, 2, 3);
	geometry::assign(a2, 3, 2, 1);

	std::cout << "color distance " << a1 << " to " << a2 << " is " << geometry::distance(a1,a2) << std::endl;

	my_2d p1 = {1, 5};
	my_2d p2 = {3, 4};
	std::cout << "float distance " << p1 << " to " << p2 << " is " << geometry::distance(p1,p2) << std::endl;


	my_class_ro cro1(1, 2);
	my_class_ro cro2(3, 4);
	std::cout << "class ro distance " << cro1 << " to " << cro2 << " is " << geometry::distance(cro1,cro2) << std::endl;

	my_class_rw crw1;
	my_class_rw crw2;
	geometry::assign(crw1, 1, 2);
	geometry::assign(crw2, 3, 4);
	std::cout << "class r/w distance " << crw1 << " to " << crw2 << " is " << geometry::distance(crw1,crw2) << std::endl;

	my_class_gs cgs1;
	my_class_gs cgs2;
	geometry::assign(cgs1, 1, 2);
	geometry::assign(cgs2, 3, 4);
	std::cout << "class g/s distance " << crw1 << " to " << crw2 << " is " << geometry::distance(cgs1,cgs2) << std::endl;

	return 0;
}
