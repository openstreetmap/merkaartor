// Generic Geometry Library
// Custom triangle template Example


#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>

#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/centroid.hpp>

#include <geometry/io/wkt/streamwkt.hpp>

#include <geometry/geometries/adapted/tuple.hpp>
#include <geometry/geometries/adapted/tuple_cartesian.hpp>


template <typename P>
struct triangle : public boost::array<P, 3>
{
};


namespace geometry
{
	namespace traits
	{

		// specialization to register the triangle as being a ring
		template <typename P>
		struct tag<triangle<P> >
		{
			typedef ring_tag type;
		};
	}


	// Specializations of area algorithm
	namespace dispatch
	{
		template<typename P>
		struct area<ring_tag, triangle<P> >
		{
			static inline double calculate(const triangle<P>& t)
			{
				return 0.5  * (
							(get<0>(t[1]) - get<0>(t[0])) * (get<1>(t[2]) - get<1>(t[0]))
							- (get<0>(t[2]) - get<0>(t[0])) * (get<1>(t[1]) - get<1>(t[0]))
							);
			}
		};
	}

};




int main()
{
	//triangle<geometry::point_xy<double> > t;
	triangle<boost::tuple<double, double> > t;
	t[0] = boost::make_tuple(0, 0);
	t[1] = boost::make_tuple(5, 0);
	t[2] = boost::make_tuple(2.5, 2.5);

	std::cout << "Triangle: " << t << std::endl;
	std::cout << "Area: " << geometry::area(t) << std::endl;

	//geometry::point_xy<double> c;
	boost::tuple<double, double> c;
	geometry::centroid(t, c);
	std::cout << "Centroid: " << c << std::endl;


	return 0;
}
