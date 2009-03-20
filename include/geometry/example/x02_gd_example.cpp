// Generic Geometry Library
// GD example

// GD is a well-known and often used graphic library to write GIF (and other formats)

// To build and run this example:
// 1) This example uses also shapelib, see 08_shapelib_example for necessary proceudre
// 2) download GD from http://www.libgd.org
// 3) add 11 files mentionded below to the project or makefile
// 4) for windows, add define NONDLL to indicate GD is not used as a DLL

// GD FILES necessary: gd.c, gd_gd.c, gd_gif_out.c, gd_io*.c, gd_security.c, gd_topal.c, gdhelpers.c, gdtables.c


#include "gd.h"

#include <vector>

#include <geometry/geometry.hpp>
#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/geometries/latlong.hpp>

#include "example/shapelib_common.hpp"

using namespace geometry;

int main()
{
	std::string filename = "c:/data/spatial/shape/world_free/world.shp";

	// The world is measured in latlong (degrees), so latlong (point_ll_deg, ring_ll_deg) is appropriate.
	// (It is possible to change this in point_xy)
	// We ignore holes in this sample.
	// So declare a ring_ll_deg (= closed ring, polygon without holes)
	std::vector<ring_ll_deg> rings;

	// Open a shapefile, for example world countries
	try
	{
		read_shapefile(filename, rings);
	}
	catch(const std::string& s)
	{
		std::cout << s << std::endl;
		return 1;
	}


	// Create a GD image.
	// A world map, as world.shp, is usually mapped in latitude-longitude (-180..180 and -90..90)
	// For this sample we use a very simple "transformation"
	// mapping to 0..720 and 0..360
	double factor = 2.0;
	gdImagePtr im = gdImageCreateTrueColor(int(360 * factor), int(180 * factor));

	// Allocate three colors
	int blue = gdImageColorResolve(im, 0, 52, 255);
	int green = gdImageColorResolve(im, 0, 255, 0);
	int black = gdImageColorResolve(im, 0, 0, 0);

	// Paint background
	gdImageFilledRectangle(im, 0, 0, im->sx, im->sy, blue);

	// Paint all rings
	for (std::vector<ring_ll_deg>::const_iterator it = rings.begin(); it != rings.end(); it++)
	{
		const ring_ll_deg& ring = *it;

		// If wished, suppress too small polygons.
		// (Note that even in latlong, area is calculated in square meters)
		if (fabs(area(ring)) > 5000.0e6)
		{
			int n = ring.size();
			gdPoint* points = new gdPoint[n];

			for (int i = 0; i < n; i++)
			{
				// Translate lon/lat or x/y to GD x/y points
				points[i].x = int(factor * (get<0>(ring[i]) + 180.0));
				points[i].y = im->sy - int(factor * (get<1>(ring[i]) + 90.0));
			}

			// Draw the polygon...
			gdImageFilledPolygon(im, points, n, green);
			// .. and the outline...
			gdImagePolygon(im, points, n, black);

			delete[] points;
		}
	}

	// Use GD to create a GIF file
	FILE* out = fopen("world.gif", "wb");
	if (out != NULL)
	{
		gdImageGif(im, out);
		fclose(out);
	}

	gdImageDestroy(im);

	return 0;
}
