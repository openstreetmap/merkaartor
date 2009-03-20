// Generic Geometry Library
// Projection example 2, using factory


#include <geometry/geometry.hpp>

#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/geometries/latlong.hpp>

#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/projections/parameters.hpp>
#include <geometry/projections/factory.hpp>

#include <boost/shared_ptr.hpp>

int main()
{
	using namespace geometry;


	// Initialize projection parameters. For construction using a factory the projection name is required.
	projection::parameters par = projection::init("+proj=robin +ellps=WGS84 +units=m");

	// Construct the specified projection, using specified point types
	// Note that this is the only difference from p01_projection_example. It constructs a projection
	// with virtual methods, which can be used polymorphically. Therefore it is a pointer. For
	// convenience we use a boost shared pointer here.
	projection::factory<point_ll_deg, point_2d> fac;
	boost::shared_ptr<projection::projection<point_ll_deg, point_2d> > prj(fac.create_new(par));


	// Define Amsterdam / Barcelona in decimal degrees / degrees/minutes
	point_ll_deg amsterdam(longitude<>(5.9), latitude<>(52.4));
	point_ll_deg barcelona(
		latitude<>(dms<north>(41, 23)),
		longitude<>(dms<east>(2, 11))
		);


	point_2d pa, pb;

	// Do the forward projection
	if (prj->forward(amsterdam, pa) && prj->forward(barcelona, pb))
	{
		std::cout << "Amsterdam: " << pa << std::endl << "Barcelona: " << pb << std::endl;

		std::cout << "Distance (unprojected):" << distance(amsterdam, barcelona) / 1000.0 << " km" << std::endl;
		std::cout << "Distance (  projected):" << distance(pa, pb) / 1000.0 << " km" << std::endl;

		// Get the inverse
		point_ll_deg a1;
		if (prj->inverse(pa, a1))
		{
			std::cout << "Amsterdam (original): " << amsterdam  << std::endl
				<< "Amsterdam (projected, and back):" << a1 << std::endl;
			std::cout << "Distance a-a': " << distance(amsterdam, a1) << " meter" << std::endl;
		}
	}

	return 0;
}
