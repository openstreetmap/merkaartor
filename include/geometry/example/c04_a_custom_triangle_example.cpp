// Generic Geometry Library
// Custom Triangle Example


#include <iostream>

#include <boost/array.hpp>


#include <geometry/algorithms/area.hpp>
#include <geometry/algorithms/centroid.hpp>

#include <geometry/io/wkt/streamwkt.hpp>


struct triangle : public boost::array<geometry::point_xy<double>, 4>
{
	inline void close()
	{
		(*this)[3] = (*this)[0];
	}
};


namespace geometry
{
	namespace traits
	{
		// specialization to register the triangle as being a ring
		template <>
		struct tag<triangle>
		{
			typedef ring_tag type;
		};
	}


	// Specializations of algorithms, where useful. If not specialized the default ones
	// (for linear rings) will be used for triangle. Which is OK as long as the triangle is closed,
	// that means, has 4 points (the last one being the first)
	template<>
	inline double area<triangle>(const triangle& t)
	{
		  return 0.5  * (
						(t[1].x() - t[0].x()) * (t[2].y() - t[0].y())
						- (t[2].x() - t[0].x()) * (t[1].y() - t[0].y())
						);
	}
};



int main()
{
	triangle t;

	t[0].x(0);
	t[0].y(0);
	t[1].x(5);
	t[1].y(0);
	t[2].x(2.5);
	t[2].y(2.5);

	t.close();

	std::cout << "Triangle: " << t << std::endl;
	std::cout << "Area: " << geometry::area(t) << std::endl;

	geometry::point_xy<double> c;
	geometry::centroid(t, c);
	std::cout << "Centroid: " << c.x() << "," << c.y() << std::endl;

	return 0;
}
