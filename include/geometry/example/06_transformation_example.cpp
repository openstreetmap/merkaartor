// Generic Geometry Library
// Transformation Example

#include <geometry/geometry.hpp>
#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/geometries/cartesian2d.hpp>



int main()
{
	using namespace geometry;

	point_2d p(1, 1);
	point_2d p2;

	// Example: translate a point over (5,5)
	strategy::transform::translate_transformer<point_2d, point_2d> translate(5, 5);

	transform(p, p2, translate);
	std::cout << "transformed point " << p2 << std::endl;

	// Transform a polygon
	polygon_2d poly, poly2;
	from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
	transform(poly, poly2, translate);

	std::cout << "transformed polygon " << poly2 << std::endl;

	// Many more transformations are possible:
	// - from Cartesian to Spherical coordinate systems and back
	// - from Cartesian to Cartesian (mapping, affine transformations) and back (inverse)
	// - Map Projections 
	// - from Degree to Radian and back in spherical or geographic coordinate systems

	return 0;
}
