// Generic Geometry Library
// Point Example - showing geographic (latitude longitude) points

#include <geometry/geometry.hpp>

#include <geometry/geometries/latlong.hpp>
#include <geometry/io/wkt/streamwkt.hpp>


int main()
{
	using namespace geometry;

	// Declare a latlong point, using doubles and degrees (= default)
	point_ll_deg paris;

	// Assign coordinates to the latlong point, using the methods lat and lon
	// Paris 48° 52' 0" N, 2° 19' 59" E
	paris.lat(dms<north>(48, 52, 0));
	paris.lon(dms<east>(2, 19, 59));

	std::cout << "Paris: " << paris << std::endl;

	// Constructor using explicit latitude/longitude
	// Lima 12° 2' 36" S, 77° 1' 42" W
	point_ll_deg lima(
			latitude<>(dms<south>(12, 2, 36)),
			longitude<>(dms<west>(77, 1, 42)));

	std::cout << "Lima: " << lima << std::endl;

	// Construction with parse utiity
	point_ll_deg amsterdam = parse<point_ll_deg>("52 22'23\"N", "4 53'32\"E");
	std::cout << "Amsterdam: " << amsterdam << std::endl;


	// Calculate the distance using the default strategy (Andoyer), and Vincenty
	std::cout << "Distance Paris-Lima, Andoyer (default) "
		<< 0.001 * distance(paris, lima)
		<< " km" << std::endl;

	std::cout << "Distance Paris-Lima, Vincenty "
		<< 0.001 * distance(paris, lima,
				strategy::distance::vincenty<point_ll_deg>())
		<< " km" << std::endl;

	// Using great circle (=haversine), this is less precise because earth is not a sphere
	std::cout << "Distance Paris-Lima, great circle "
		<< 0.001 * distance(paris, lima,
				strategy::distance::haversine<point_ll_deg>())
		<< " km" << std::endl;


	// Convert a latlong point to radians. This might be convenient, although algorithms
	// are transparent on degree/radians
	point_ll_rad paris_rad;
	transform(paris, paris_rad);
	std::cout << "Paris in radians: " << paris_rad.lon() << " " << paris_rad.lat() << std::endl;

	point_ll_rad amsterdam_rad;
	transform(amsterdam, amsterdam_rad);
	std::cout << "Amsterdam in radians: " << amsterdam_rad.lon() << " " << amsterdam_rad.lat() << std::endl;

	std::cout << "Distance Paris-Amsterdam, (degree) " << 0.001 * distance(paris, amsterdam) << " km" << std::endl;
	std::cout << "Distance Paris-Amsterdam, (radian) " << 0.001 * distance(paris_rad, amsterdam_rad) << " km" << std::endl;
	std::cout << "Distance Paris-Amsterdam, (mixed) " << 0.001 * distance(paris, amsterdam_rad) << " km" << std::endl;


 	return 0;
}
