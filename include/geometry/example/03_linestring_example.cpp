// Generic Geometry Library
// Linestring Example

#include <algorithm> // for reverse, unique

#include <geometry/geometry.hpp>
#include <geometry/geometries/cartesian2d.hpp>

// Optional includes to handle c-arrays as points, std::vectors as linestrings
#include <geometry/geometries/adapted/c_array_cartesian.hpp>
#include <geometry/geometries/adapted/std_as_linestring.hpp>

// Optional include to stream as WKT
#include <geometry/io/wkt/streamwkt.hpp>


template<typename P>
inline void translate_function(P& p)
{
		p.x(p.x() + 100.0);
}


template<typename P>
struct scale_functor
{
	inline void operator()(P& p)
	{
		p.x(p.x() * 1000.0);
		p.y(p.y() * 1000.0);
	}
};



int main(void)
{
	using namespace geometry;

	// Define a linestring, which is a vector of points, and add some points
	// (we add them deliberately in different ways)
	linestring_2d ls;

	// points can be created using "make" and added to a linestring using the std:: "push_back"
	ls.push_back(make<point_2d>(1.1, 1.1));

	// points can also be assigned using "assign" and added to a linestring using "append"
	point_2d lp;
	assign(lp, 2.5, 2.1);
	append(ls, lp);


	// Lines can be streamed as Well Known Text (OGC WKT)
	std::cout << ls << std::endl;

	// The bounding box of linestrings can be calculated
	box_2d b;
	envelope(ls, b);
	std::cout << b << std::endl;

	// The length of the line can be calulated
	std::cout << "length: " << length(ls) << std::endl;

	// All things from std::vector can be called, because a linestring is a vector
	std::cout << "number of points 1: " << ls.size() << std::endl;

	// All things from boost ranges can be called because a linestring is considered as a range
	std::cout << "number of points 2: " << boost::size(ls) << std::endl;

	// Generic function from geometry/OGC delivers the same value
	std::cout << "number of points 3: " << num_points(ls) << std::endl;

	// The distance from a point to a linestring can be calculated
	point_2d p(1.9, 1.2);
	std::cout << "distance of " << p << " to line: " << distance(p, ls) << std::endl;

	// A linestring is a vector. However, some algorithms consider "segments",
	// which are the line pieces between two points of a linestring.
	double d = distance(p, segment<point_2d >(ls.front(), ls.back()));

	// Add some three points more, let's do it using a classic array.
	// (See documentation for picture of this linestring)
	const double c[][2] = { {3.1, 3.1}, {4.9, 1.1}, {3.1, 1.9} };
	append(ls, c);
	std::cout << "appended: " << ls << std::endl;

	// Output as iterator-pair on a vector
	{
		std::vector<point_2d> v;
		std::copy(ls.begin(), ls.end(), std::back_inserter(v));
		std::cout
			<< "as it-pair via make-wkt: "
			<< make_wkt(std::make_pair(v.begin(), v.end()))
			<< std::endl;

		std::cout
			<< "as vector: "
			<< v
			<< std::endl;

		std::cout
			<< "as it-pair: "
			<< std::make_pair(v.begin(), v.end())
			<< std::endl;
	}


	// All algorithms from std can be used: a linestring is a vector
	std::reverse(ls.begin(), ls.end());
	std::cout << "reversed: " << ls << std::endl;
	std::reverse(boost::begin(ls), boost::end(ls));

	// The other way, using a vector instead of a linestring, is also possible
	std::vector<point_2d> pv(ls.begin(), ls.end());
	std::cout << "length: " << length(pv) << std::endl;

	// If there are double points in the line, you can use unique to remove them
	// So we add the last point, print, make a unique copy and print
	ls.push_back(ls.back());
	std::cout << "extra point, last point=double: " << ls << std::endl;

	{
		linestring_2d ls_copy;
		std::unique_copy(ls.begin(), ls.end(), std::back_inserter(ls_copy));
		ls = ls_copy;
		std::cout << "uniquecopy: " << ls << std::endl;
	}

	// Lines can be simplified using e.g. Douglas Peucker
	linestring_2d ls_simplified;
	simplify(ls, std::back_inserter(ls_simplified), 0.5);
	std::cout << "simplified: " << ls_simplified << std::endl;


	// Lines can be read from Well-Known Text (WKT)
	linestring_2d wline;
	from_wkt("LINESTRING(1 1,2 2, 3 3)", wline);

	// for_each:
	// 1) Lines can be visited with std::for_each
	// 2) for_each_point is also defined for all geometries
	// 3) for_each_segment is defined for all geometries to all segments
	// 4) loop is defined for geometries to visit segments
	//    with state apart, and to be able to break out (not shown here)
	{
		linestring_2d lscopy = ls;
		std::for_each(lscopy.begin(), lscopy.end(), translate_function<point_2d>);
		for_each_point(lscopy, scale_functor<point_2d>());
		for_each_point(lscopy, translate_function<point_2d>);
		std::cout << "modified line: " << lscopy << std::endl;
	}

	// Lines can be clipped using a clipping box. Clipped lines are added to the output iterator
	box_2d cb(point_2d(1.5, 1.5), point_2d(4.5, 2.5));

	std::vector<linestring_2d> clipped;
	intersection(cb, ls, std::back_inserter(clipped));

	// Also possible: clip-output to a vector of vectors
	std::vector<std::vector<point_2d> > vector_out;
	intersection(cb, ls, std::back_inserter(vector_out));

	std::cout << "clipped output as vector:" << std::endl;
	for (size_t i = 0; i < vector_out.size(); i++)
	{
		std::copy(vector_out[i].begin(), vector_out[i].end(),
						std::ostream_iterator<point_2d>(std::cout, " "));
		std::cout << std::endl;
	}

	// Calculate the convex hull of the linestring
	polygon_2d hull;
	convex_hull(ls, std::back_inserter(hull.outer()));
	std::cout << "Convex hull:" << hull << std::endl;

	// All the above assumed 2d Cartesian linestrings. There is more.
	// Let's define a 3d float point ourselves
	typedef point<float, 3, cs::cartesian> P;
	typedef linestring<P> L;
	L line3d;
	line3d.push_back(make<P>(1,2,3));
	line3d.push_back(make<P>(4,5,6));
	line3d.push_back(make<P>(7,8,9));

	// Not all algorithms work on 3d lines. For example convex hull does NOT.
	// But for example length, distance, simplify, envelope and stream do.
	std::cout << length(line3d) << " " << line3d << std::endl;

	// There are also lines in latlong, linestring_deg and linestring_rad

	return 0;
}
