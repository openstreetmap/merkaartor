// Generic Geometry Library
// Projection example 3, combined with shapelib and GD



#include "shapefil.h"
#include "gd.h"

#include <geometry/geometry.hpp>

#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/geometries/latlong.hpp>

#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/geometries/adapted/std_as_ring.hpp>

#include <geometry/geometries/register/register_point.hpp>

#include <projections/project_transformer.hpp>


#include "example/shapelib_common.hpp"



// Register a GD gdPoint such that it is recognized by the geometry library
// and can therefore be used in transformations
GEOMETRY_REGISTER_POINT_2D(gdPoint, int, cs::cartesian, x, y)





int main(int argc, char** argv)
{
	using namespace geometry;

	try
	{
		std::string filename("c:/data/spatial/shape/world_free/world.shp");


		// All our latlong rings will be collected in a vector
		std::vector<ring_ll_deg> ll_rings;


		// Open a shapefile, for example world countries. This is worked out in example x01
		read_shapefile(filename, ll_rings);


		// Our latlong ring collection will be projected into this vector
		// (Of course it is also possible to do this while reading and have one vector)
		std::vector<ring_2d> xy_rings;


		// We will use the Mollweide projection by default, in the command line other choices are possible
		std::string par("+proj=moll +ellps=clrk66");
		if (argc > 1)
		{
			par = argv[1];
		}

		// Declare transformation strategy which contains a projection
		projection::project_transformer<point_ll_deg, point_2d> project(par);

		// Project the rings, and at the same time get the bounding box (in xy)
		// Note that there is, or will be, library utilities to project a geometry
		box_2d bbox;
		assign_inverse(bbox);
		for (std::vector<ring_ll_deg>::const_iterator it = ll_rings.begin(); it != ll_rings.end(); it++)
		{
			ring_2d xy_ring;

			if (transform(*it, xy_ring, project))
			{
				// Update bbox with box of this projected ring
				combine(bbox, make_envelope<box_2d>(xy_ring));

				// Add projected ring 
				xy_rings.push_back(xy_ring);
			}
		}

		// Create a GD image.
		gdImagePtr im = gdImageCreateTrueColor(720, 360);

		// Use a transformation matrix which maps from world coordinates (bbox) to the GD Image (width,height)
		// while also indicating that the GD Image is mirrored in y-direction, coordinates are downwards
		// (i.e. 0 is top, height = bottom, while in GIS coordinates are upwards)
		strategy::transform::map_transformer<point_2d, gdPoint, true> matrix(bbox, im->sx, im->sy);

		// Allocate three colors
		int blue = gdImageColorResolve(im, 0, 0, 255);
		int green = gdImageColorResolve(im, 0, 255, 0);
		int black = gdImageColorResolve(im, 0, 0, 0);

		// Paint background
		gdImageFilledRectangle(im, 0, 0, im->sx, im->sy, blue);

		// Paint all rings
		for (std::vector<ring_2d>::const_iterator it = xy_rings.begin(); it != xy_rings.end(); it++)
		{
			const ring_2d& ring = *it;

			int n = num_points(ring);
			gdPoint* points = new gdPoint[n];

			for (int i = 0; i < n; i++)
			{
				geometry::transform(ring[i], points[i], matrix);
			}

			// Draw the polygon and the outline...
			gdImageFilledPolygon(im, points, n, green);
			gdImagePolygon(im, points, n, black);

			delete[] points;
		}


		// Use GD to create a GIF file
		FILE* out = fopen("p03_example.gif", "wb");
		if (out != NULL)
		{
			gdImageGif(im, out);
			fclose(out);
		}

		gdImageDestroy(im);

	}
	catch(const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}
	catch(const std::string& s)
	{
		std::cout << "Exception: " << s << std::endl;
		return 1;
	}

	return 0;
}
