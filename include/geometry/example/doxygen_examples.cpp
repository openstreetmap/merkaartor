// Generic Geometry Library
// Doxygen Examples, referred to from the sources

#include <boost/tuple/tuple.hpp>

#if defined(_MSC_VER)
// We deliberately mix float/double's here so turn off warning
#pragma warning( disable : 4244 )
#endif // defined(_MSC_VER)

#include <geometry/geometry.hpp>
#include <geometry/geometries/geometries.hpp>
#include <geometry/geometries/adapted/std_as_linestring.hpp>
#include <geometry/io/wkt/streamwkt.hpp>



// All functions below are referred to in the source of the Geometry Library.
// Don't rename them.

void example_area_polygon()
{
	geometry::polygon<geometry::point_xy<double> > poly;
	geometry::from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
	std::cout << "Polygon area is "
		<< geometry::area(poly)
		<< " square units" << std::endl;

	geometry::polygon<geometry::point_ll<float, geometry::cs::geographic<geometry::degree> > > poly_ll;
	geometry::from_wkt("POLYGON((4 51,4 52,5 52,5 51,4 51))", poly_ll);
	std::cout << "Polygon area is "
		<< geometry::area(poly_ll)/(1000*1000)
		<< " square kilometers " << std::endl;
}

void example_as_wkt_point()
{
	typedef geometry::point_xy<double> P;
	P p(5.12, 6.34);
	// Points can be streamed like this:
	std::cout << geometry::wkt<P>(p) << std::endl;

	// or like this:
	std::cout << geometry::make_wkt(p) << std::endl;

	// or, including streamwkt, like this:
	std::cout << p << std::endl;
}

void example_as_wkt_vector()
{
	std::vector<geometry::point_xy<int> > v;
	geometry::from_wkt<geometry::point_xy<int> >("linestring(1 1,2 2,3 3,4 4)", std::back_inserter(v));

	std::cout << geometry::make_wkt(std::make_pair(v.begin(), v.end())) << std::endl;
}


void example_centroid_polygon()
{
	geometry::polygon<geometry::point_xy<double> > poly;
	geometry::from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
	// Center of polygon might have different type then points of polygon
	geometry::point_xy<float> center;
	geometry::centroid(poly, center);
	std::cout << "Centroid: " << center.x() << "," << center.y() << std::endl;
}


void example_distance_point_point()
{
	geometry::point_xy<double> p1(1, 1);
	geometry::point_xy<double> p2(2, 3);
	std::cout << "Distance p1-p2 is "
		<< geometry::distance(p1, p2)
		<< " units" << std::endl;

	// Read 2 Dutch cities from WKT texts (in decimal degrees)
	geometry::point_ll<double, geometry::cs::geographic<geometry::degree> >  a, r;
	geometry::from_wkt("POINT(4.89222 52.3731)", a);
	geometry::from_wkt("POINT(4.47917 51.9308)", r);

	std::cout << "Distance Amsterdam-Rotterdam is "
		<< geometry::distance(a, r) / 1000.0
		<< " kilometers " << std::endl;
}

void example_distance_point_point_strategy()
{
	typedef geometry::point_ll<double, geometry::cs::geographic<geometry::degree> > LL;
	LL a, r;
	geometry::from_wkt("POINT(4.89222 52.3731)", a);
	geometry::from_wkt("POINT(4.47917 51.9308)", r);

	std::cout << "Distance Amsterdam-Rotterdam is "
		<< geometry::distance(a, r,
				geometry::strategy::distance::vincenty<LL>() )
				 / 1000.0
		<< " kilometers " << std::endl;
}

void example_from_wkt_point()
{
	geometry::point_xy<int> point;
	geometry::from_wkt("Point(1 2)", point);
	std::cout << point.x() << "," << point.y() << std::endl;
}

void example_from_wkt_output_iterator()
{
	std::vector<geometry::point_xy<int> > v;
	geometry::from_wkt<geometry::point_xy<int> >("linestring(1 1,2 2,3 3,4 4)", std::back_inserter(v));
	std::cout << "vector has " << v.size() << " coordinates" << std::endl;
}

void example_from_wkt_linestring()
{
	geometry::linestring<geometry::point_xy<double> > line;
	geometry::from_wkt("linestring(1 1,2 2,3 3,4 4)", line);
	std::cout << "linestring has " << line.size() << " coordinates" << std::endl;
}

void example_from_wkt_polygon()
{
	geometry::polygon<geometry::point_xy<double> > poly;
	geometry::from_wkt("POLYGON((0 0,0 1,1 1,1 0,0 0))", poly);
	std::cout << "Polygon has " << poly.outer().size() << " coordinates in outer ring" << std::endl;
}

void example_point_ll_convert()
{
	geometry::point_ll<double, geometry::cs::geographic<geometry::degree> > deg(geometry::latitude<>(33.0), geometry::longitude<>(-118.0));
	geometry::point_ll<double, geometry::cs::geographic<geometry::radian> > rad;
	geometry::transform(deg, rad);

	std::cout << "point in radians: " << rad << std::endl;
}

void example_intersection_linestring1()
{
	typedef geometry::point_xy<double> P;
	geometry::linestring<P> line;
	geometry::from_wkt("linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)", line);
	geometry::box<P> cb(P(1.5, 1.5), P(4.5, 2.5));
	std::cout << "Clipped linestring(s) " << std::endl;
	geometry::intersection(cb, line,
			std::ostream_iterator<geometry::linestring<P> >(std::cout, "\n"));
}

void example_intersection_linestring2()
{
	typedef geometry::point_xy<double> P;
	std::vector<P> vector_in;
	geometry::from_wkt<P>("linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)",
					std::back_inserter(vector_in));

	geometry::box<P> cb(P(1.5, 1.5), P(4.5, 2.5));
	typedef std::vector<std::vector<P> > VV;
	VV vector_out;
	geometry::intersection(cb, vector_in, std::back_inserter(vector_out));

	std::cout << "Clipped vector(s) " << std::endl;
	for (VV::const_iterator it = vector_out.begin(); it != vector_out.end(); it++)
	{
		std::copy(it->begin(), it->end(), std::ostream_iterator<P>(std::cout, " "));
		std::cout << std::endl;
	}
}



void example_intersection_segment1()
{
	typedef geometry::point_xy<double> P;
	P a(0, 2);
	P b(4, 2);
	P c(3, 0);
	P d(3, 4);
	geometry::segment<P> s1(a, b);
	geometry::segment<P> s2(c, d);

	std::cout << "Intersection point(s): ";
	geometry::intersection_result r = geometry::intersection_segment<P>(s1, s2,
		std::ostream_iterator<P>(std::cout, "\n"));
	std::cout << "Intersection result: " << int(r.is_type) << std::endl;
}


void example_intersection_polygon1()
{
	typedef geometry::point_xy<double> P;
	typedef std::vector<geometry::polygon<P> > PV;

	geometry::box<P> cb(P(1.5, 1.5), P(4.5, 2.5));
	geometry::polygon<P> poly;
	geometry::from_wkt("POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
			",(4 2,4.2 1.4,4.8 1.9,4.4 2.2,4 2))", poly);

	PV v;
	geometry::intersection(cb, poly, std::back_inserter(v));

	std::cout << "Clipped polygon(s) " << std::endl;
	for (PV::const_iterator it = v.begin(); it != v.end(); it++)
	{
		std::cout << *it << std::endl;
	}
}

void example_simplify_linestring1()
{
	geometry::linestring<geometry::point_xy<double> > line, simplified;
	geometry::from_wkt("linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)", line);
	geometry::simplify(line, std::back_inserter(simplified), 0.5);
	std::cout
		<< "  original line: " << line << std::endl
		<< "simplified line: " << simplified << std::endl;
}

void example_simplify_linestring2()
{
	typedef geometry::point_xy<double> P;
	typedef geometry::linestring<P> L;
	L line;

	geometry::from_wkt("linestring(1.1 1.1, 2.5 2.1, 3.1 3.1, 4.9 1.1, 3.1 1.9)", line);

	typedef geometry::strategy::distance::xy_point_segment<P, geometry::segment<const P> > DS;
	typedef std::ostream_iterator<P> OUT;
	typedef geometry::strategy::simplify::douglas_peucker<L, OUT, DS> simplification;
	geometry::simplify(line, OUT(std::cout, "\n"), 0.5, simplification());
}



void example_within()
{
	geometry::polygon<geometry::point_xy<double> > poly;
	geometry::from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
	geometry::point_xy<float> point(3, 3);
	std::cout << "Point is "
		<< (geometry::within(point, poly) ? "IN" : "NOT in")
		<< " polygon"
		<< std::endl;
}

/*
void example_within_strategy()
{
	// TO BE UPDATED/FINISHED
	typedef geometry::point_xy<double> P;
	typedef geometry::polygon<P> POLY;
	P p;
	std::cout << within(p, poly, strategy::within::cross_count<P>) << std::endl;
}
*/

void example_length_linestring()
{
	using namespace geometry;
	linestring<point_xy<double> > line;
	from_wkt("linestring(0 0,1 1,4 8,3 2)", line);
	std::cout << "linestring length is "
		<< length(line)
		<< " units" << std::endl;

	// Linestring in latlong, filled with
	// explicit degree-minute-second values
	typedef point_ll<float, geometry::cs::geographic<geometry::degree> > LL;
	linestring<LL> line_ll;
	line_ll.push_back(LL(
		latitude<float>(dms<north, float>(52, 22, 23)),
		longitude<float>(dms<east, float>(4, 53, 32))));
	line_ll.push_back(LL(
		latitude<float>(dms<north, float>(51, 55, 51)),
		longitude<float>(dms<east, float>(4, 28, 45))));
	line_ll.push_back(LL(
		latitude<float>(dms<north, float>(52, 4, 48)),
		longitude<float>(dms<east, float>(4, 18, 0))));
	std::cout << "linestring length is "
		<< length(line_ll) / 1000
		<< " kilometers " << std::endl;
}

void example_length_linestring_iterators1()
{
	geometry::linestring<geometry::point_xy<double> > line;
	geometry::from_wkt("linestring(0 0,1 1,4 8,3 2)", line);
	std::cout << "linestring length is "
		<< geometry::length(line)
		<< " units" << std::endl;
}

void example_length_linestring_iterators2()
{
	std::vector<geometry::point_xy<double> > line;
	geometry::from_wkt<geometry::point_xy<double> >("linestring(0 0,1 1,4 8,3 2)", std::back_inserter(line));
	std::cout << "linestring length is "
		<< geometry::length(line)
		<< " units" << std::endl;
}

void example_length_linestring_iterators3()
{
	using namespace geometry;
	typedef point_ll<float, geometry::cs::geographic<geometry::degree> > LL;
	std::deque<LL> line;
	geometry::from_wkt<LL>("linestring(0 51,1 51,2 52)", std::back_inserter(line));
	std::cout << "linestring length is "
		<< 0.001 * geometry::length(line, geometry::strategy::distance::vincenty<LL>())
		<< " kilometers" << std::endl;
}


void example_length_linestring_strategy()
{
	using namespace geometry;
	typedef point_ll<float, geometry::cs::geographic<geometry::degree> > LL;
	linestring<LL> line_ll;
	line_ll.push_back(LL(latitude<float>(dms<north, float>(52, 22, 23)), longitude<float>(dms<east, float>(4, 53, 32))));
	line_ll.push_back(LL(latitude<float>(dms<north, float>(51, 55, 51)), longitude<float>(dms<east, float>(4, 28, 45))));
	line_ll.push_back(LL(latitude<float>(dms<north, float>(52, 4, 48)), longitude<float>(dms<east, float>(4, 18, 0))));
	std::cout << "linestring length is "
		<< length(line_ll, strategy::distance::vincenty<LL, LL>() )/(1000)
		<< " kilometers " << std::endl;
}


void example_envelope_linestring()
{
	geometry::linestring<geometry::point_xy<double> > line;
	geometry::from_wkt("linestring(0 0,1 1,4 8,3 2)", line);
	geometry::box<geometry::point_xy<double> > box;
	geometry::envelope(line, box);

	std::cout << "envelope is " << box << std::endl;
}

void example_envelope_polygon()
{
	using namespace geometry;
	typedef point_ll<double, geometry::cs::geographic<geometry::degree> >  LL;

	// Wrangel island, 180 meridian crossing island above Siberia.
	polygon<LL> wrangel;
	wrangel.outer().push_back(LL(latitude<>(dms<north>(70, 47, 7)), longitude<>(dms<west>(178, 47, 9))));
	wrangel.outer().push_back(LL(latitude<>(dms<north>(71, 14, 0)), longitude<>(dms<east>(177, 28, 33))));
	wrangel.outer().push_back(LL(latitude<>(dms<north>(71, 34, 24)), longitude<>(dms<east>(179, 44, 37))));
	// Close it
	wrangel.outer().push_back(wrangel.outer().front());

	geometry::box<LL> box;
	geometry::envelope(wrangel, box);

	dms<cd_lat> minlat(box.min_corner().lat());
	dms<cd_lon> minlon(box.min_corner().lon());

	dms<cd_lat> maxlat(box.max_corner().lat());
	dms<cd_lon> maxlon(box.max_corner().lon());

	std::cout << wrangel << std::endl;
	std::cout << "min: " << minlat.get_dms() << " , " << minlon.get_dms() << std::endl;
	std::cout << "max: " << maxlat.get_dms() << " , " << maxlon.get_dms() << std::endl;
}


void example_dms()
{
	// Construction with degree/minute/seconds
	geometry::dms<geometry::east> d1(4, 53, 32.5);

	// Explicit conversion to double.
	std::cout << d1.as_value() << std::endl;

	// Conversion to string, with optional strings
	std::cout << d1.get_dms(" deg ", " min ", " sec") << std::endl;

	// Combination with latitude/longitude and cardinal directions
	{
		using namespace geometry;
		point_ll<double, geometry::cs::geographic<geometry::degree> > canberra(
			latitude<>(dms<south>(35, 18, 27)),
			longitude<>(dms<east>(149, 7, 27.9)));
		std::cout << canberra << std::endl;
	}
}

void example_point_ll_construct()
{
	using namespace geometry;
	typedef point_ll<double, geometry::cs::geographic<geometry::degree> > ll;

	// Constructions in both orders possible
	ll juneau(
		latitude<>(dms<north>(58, 21, 5)),
		longitude<>(dms<west>(134, 30, 42)));
	ll wladiwostok(
		longitude<>(dms<east>(131, 54)),
		latitude<>(dms<north>(43, 8))
		);
}

namespace example_loop1
{
	// Class functor
	template <typename P>
	struct perimeter
	{
		struct summation
		{
			double sum;
			summation() : sum(0) {}
		};

		bool operator()(const geometry::segment<const P>& segment, summation& s) const
		{
			std::cout << "from " << segment.first << " to " << segment.second << std::endl;
			s.sum += geometry::distance(segment.first, segment.second);
			return true;
		}
	};

	void example()
	{
		typedef geometry::point_xy<double> P;
		geometry::polygon<P> poly;
		geometry::from_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
		perimeter<P>::summation peri;
		geometry::loop(poly.outer(), perimeter<P>(), peri);
		std::cout << "Perimeter: " << peri.sum << std::endl;
	}
} //:\\


namespace example_loop2
{
	struct summation
	{
		double sum;
		summation() : sum(0) {}
	};

	// Function functor
	template <typename P>
	bool perimeter(const geometry::segment<const P>& segment, summation& s)
	{
		std::cout << "from " << segment.first << " to " << segment.second << std::endl;
		s.sum += geometry::distance(segment.first, segment.second);
		return true;
	}

	void example()
	{
		typedef geometry::point_ll<double, geometry::cs::geographic<geometry::degree> > P;
		geometry::polygon<P> poly;
		geometry::from_wkt("POLYGON((-178.786 70.7853,177.476 71.2333,179.744 71.5733,-178.786 70.7853))", poly);
		summation peri;
		geometry::loop(poly.outer(), perimeter<P>, peri);
		std::cout << "Perimeter: " << peri.sum/1000.0 << " km" << std::endl;
	}
} //:\\



struct example_point_1
{
	// Example point, for example a legacy point defining somewhere an x and an y coordinate
	double x, y;
};


namespace geometry
{
	namespace traits
	{
		template <int I> struct accessor;

		template <> struct accessor<0>
		{
			inline static double get(const example_point_1& p) { return p.x; }
			inline static void set(example_point_1& p, const double& value) { p.x = value; }
		};

		template <> struct accessor<1>
		{
			inline static double get(const example_point_1& p) { return p.y; }
			inline static void set(example_point_1& p, const double& value) { p.y = value; }
		};

		// For legacy points, define the necessary structs coordinate (with typedef),
		// dimension (with value) and access (with get function).
		// Be sure to define them within the namespace geometry::traits
		template <> struct tag<example_point_1> { typedef point_tag type; };
		template <> struct coordinate_type<example_point_1> { typedef double type; };
		template <> struct coordinate_system<example_point_1> { typedef cs::cartesian type; };
		template <> struct dimension<example_point_1>: boost::mpl::int_<2> {};
		template <> struct access<example_point_1>
		{
			template <int I>
			static double get(const example_point_1& p)
			{ return accessor<I>::get(p); }

			template <int I>
			static void set(example_point_1& p, const double& value)
			{ accessor<I>::set(p, value); }
		};
	}
}

namespace example_own_point1
{
	// The first way to check a concept at compile time: checking if the input is parameter
	// or return type is OK.
	template <typename P>
	BOOST_CONCEPT_REQUIRES(((geometry::Point<P>)), (void))
	test1(P& p)
	{
	}

	// The second way to check a concept at compile time: checking if the provided type,
	// inside the function, if OK
	template <typename P>
	void test2(P& p)
	{
		BOOST_CONCEPT_ASSERT((geometry::Point<P>));
	}


	void example()
	{
		example_point_1 p;
		test1(p);
		test2(p);
	}
} //:\\



namespace example_own_point2
{
	struct example_point_2: boost::tuple<float, float>
	{
		example_point_2(float x, float y)
		{
			get<0>() = x;
			get<1>() = y;
		}
	};
}

// WILL BE CONVERTED TO MACRO
namespace geometry
{
	namespace traits
	{
		using namespace example_own_point2;

		template <> struct tag<example_point_2  > { typedef point_tag type; };
		template <> struct coordinate_type<example_point_2 > { typedef float type; };
		template <> struct coordinate_system<example_point_2 > { typedef geometry::cs::cartesian type; };
		template <> struct dimension<example_point_2 > : boost::mpl::int_<2> {};
		template <> struct access<example_point_2 >
		{
			template <int I>
			static inline float get(const example_point_2& p) { return p.get<I>(); }

			template <int I>
			static inline void set(example_point_2& p, const float& value) { p.get<I>() = value; }
		};

		// The library user has
		// 1) either to specify the coordinate system
		// 2) or include <geometry/geometries/adapted/tuple_@.hpp> where @=cartesian,geographic,...
	}
}


namespace example_own_point2
{


	template <typename P>
	BOOST_CONCEPT_REQUIRES(((geometry::ConstPoint<P>)), (float))
	test3(P& p)
	{
		return geometry::get<0>(p);
	}

	void example()
	{
		example_point_2 p(1,2);
		test3(p);
	}
} //:\\



int main(void)
{
	example_area_polygon();

	example_centroid_polygon();

	example_distance_point_point();
	example_distance_point_point_strategy();

	example_from_wkt_point();
	example_from_wkt_output_iterator();
	example_from_wkt_linestring();
	example_from_wkt_polygon();

	example_as_wkt_point();

	example_intersection_linestring1();
	example_intersection_linestring2();
	example_intersection_polygon1();
	example_intersection_segment1();

	example_simplify_linestring1();
	example_simplify_linestring2();

	example_length_linestring();
	example_length_linestring_iterators1();
	example_length_linestring_iterators2();
	example_length_linestring_iterators3();
	example_length_linestring_strategy();

	example_envelope_linestring();
	example_envelope_polygon();

	example_within();

	example_point_ll_convert();
	example_point_ll_construct();
	example_dms();

	example_loop1::example();
	example_loop2::example();
	example_own_point1::example();
	example_own_point2::example();

	return 0;
}
