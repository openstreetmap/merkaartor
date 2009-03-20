// Generic Geometry Library
// Custom Linestring Example

#include <geometry/geometry.hpp>

#include <geometry/geometries/register/register_point.hpp>


// Define a GPS point with coordinates in latitude/longitude and some additional values
struct gps_point
{
	double latitude, longitude, height;
	double speed;
	// Date/time, heading, etc could be added


	// The default constructor is required if being used in a vector
	gps_point() {}

	// Define a constructor to create the point in one line. Order of latitude/longitude
	// does not matter as long as "E", "N", etc are included
	gps_point(const std::string& c1, const std::string& c2, double h, double s)
		: height(h)
		, speed(s)
	{
		geometry::parse(*this, c1, c2);
	}
};


// Register this point as being a recognizable point by the GGL
GEOMETRY_REGISTER_POINT_2D(gps_point, double, cs::geographic<degree>, longitude, latitude)


// Declare a custom linestring which will have the GPS points
struct gps_track : std::vector<gps_point> 
{
	std::string owner;
	int route_identifier;
	// etc

	gps_track(int i, const std::string& o)
		: owner(o)
		, route_identifier(i)
	{}
};


// Register the track as well, as being a "linestring" (there is no (not yet) a macro for this)
namespace geometry 
{ 
	namespace traits 
	{
		template <> struct tag<gps_track> { typedef linestring_tag type; };
	}
}



int main()
{
	// Declare a "GPS Track" and add some GPS points
	gps_track track(23, "Mister G");
	track.push_back(gps_point("52 22 23 N", "4 53 32 E", 50, 180));
	track.push_back(gps_point("52 10 00 N", "4 59 59 E", 110, 170));
	track.push_back(gps_point("52 5 20 N", "5 6 56 E", 0, 90));

	std::cout 
		<< "track:  " << track.route_identifier << std::endl
		<< "from:   " << track.owner << std::endl
		<< "as wkt: " << geometry::make_wkt(track) << std::endl
		<< "length: " << geometry::length(track)/1000.0 << " km" << std::endl;

	// Above gives the idea, shows that custom linestrings can be useful.
	// We could of course do anything with this track which the library can handle, e.g.:
	// - simplify it
	// - calculate distance of point-to-line
	// - project it to UTM, then transform it to a GIF image (see p03_example)

	return 0;
}
