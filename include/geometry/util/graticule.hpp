// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_GRATICULE_HPP
#define _GEOMETRY_GRATICULE_HPP

#include <string>
#include <sstream>

#include <boost/numeric/conversion/cast.hpp>



namespace geometry
{

	/*!
		\brief Cardinal directions.
		\ingroup cs
		\details They are used in the dms-class. When specified by the library user,
		north/east/south/west is, in general, enough. When parsed or received by an algorithm,
		the user knows it it is lat/long but not more
	*/
	enum cd_selector {/*cd_none, */ north, east, south, west, cd_lat, cd_lon};

	/*!
		\brief Utility class to assign poinst with degree,minute,second
		\ingroup cs
		\note Normally combined with latitude and longitude classes
		\tparam CD selects if it is north/south/west/east
		\tparam coordinate value, double/float
		\par Example:
		Example showing how to use the dms class
		\dontinclude doxygen_examples.cpp
		\skip example_dms
		\line {
		\until }
	*/
	template <cd_selector CD, typename T = double>
	class dms
	{
		public :
			/// Constructs with a value
			inline explicit dms(T v)
				: m_value(v)
			{}
			/// Constructs with a degree, minute, optional second
			inline explicit dms(int d, int m, T s = 0.0)
			{
				double v = ((CD == west || CD == south) ? -1.0 : 1.0)
					* (double(d) + (m / 60.0) + (s / 3600.0));

				m_value = boost::numeric_cast<T>(v);

			}

			// Prohibit automatic conversion to T
			// because this would enable lon(dms<south>)
			// inline operator T() const { return m_value; }

			/// Explicit conversion to T (double/float)
			inline const T& as_value() const
			{
				return m_value;
			}

			/// Get degrees as integer, minutes as integer, seconds as double.
			inline void get_dms(int& d, int& m, double& s,
				bool& positive, char& cardinal) const
			{
				double value = m_value;

				// Set to normal earth latlong coordinates
				while (value < -180)
				{
					value += 360;
				}
				while (value > 180)
				{
					value -= 360;
				}
				// Make positive and indicate this
				positive = value > 0;

				// Todo: we might implment template/specializations here
				// Todo: if it is "west" and "positive", make east? or keep minus sign then?

				cardinal = ((CD == cd_lat && positive) ? 'N'
					:  (CD == cd_lat && !positive) ? 'S'
					:  (CD == cd_lon && positive) ? 'E'
					:  (CD == cd_lon && !positive) ? 'W'
					:  (CD == east) ? 'E'
					:  (CD == west) ? 'W'
					:  (CD == north) ? 'N'
					:  (CD == south) ? 'S'
					: ' ');

				value = fabs(value);

				// Calculate the values
				double fraction, integer;
				fraction = modf(value, &integer);
				d = int(integer);
				s = 60.0 * modf(fraction * 60.0, &integer);
				m = int(integer);
			}

			/// Get degrees, minutes, seconds as a string, separators can be specified optionally
			inline std::string get_dms(const std::string& ds = " ",
				const std::string& ms = "'",
				const std::string& ss = "\"") const
			{
				double s;
				int d, m;
				bool positive;
				char cardinal;
				get_dms(d, m, s, positive, cardinal);
				std::ostringstream out;
				out << d << ds << m << ms << s << ss << " " << cardinal;


				return out.str();
			}

		private :
			T m_value;
	};


	#ifndef DOXYGEN_NO_IMPL
	namespace impl
	{
		/*!
			\brief internal base class for latitude and longitude classes
			\details The latitude longitude classes define different types for lat and lon. This is convenient
			to construct latlong class without ambiguity.
			\note It is called graticule, after <em>"This latitude/longitude "webbing" is known as the common
			graticule" (http://en.wikipedia.org/wiki/Geographic_coordinate_system)</em>
			\tparam S latitude/longitude
			\tparam T coordinate type, double float or int
		*/
		template <typename T>
		class graticule
		{
			public :
				inline explicit graticule(T v) : m_v(v) {}
				inline operator T() const { return m_v; }
			private :
				T m_v;
		};
	}
	#endif

	/*!
		\brief Utility class to assign points with latitude value (north/south)
		\ingroup cs
		\tparam T coordinate type, double / float
		\note Often combined with dms class
	*/
	template <typename T = double> class latitude : public impl::graticule<T>
	{
		public :
			/// Can be constructed with a value
			inline explicit latitude(T v) : impl::graticule<T>(v) {}
			/// Can be constructed with a NORTH dms-class
			inline explicit latitude(const dms<north,T>& v) : impl::graticule<T>(v.as_value()) {}
			/// Can be constructed with a SOUTH dms-class
			inline explicit latitude(const dms<south,T>& v) : impl::graticule<T>(v.as_value()) {}
	};
	/*!
		\brief Utility class to assign points with longitude value (west/east)
		\ingroup cs
		\tparam T coordinate type, double / float
		\note Often combined with dms class
	*/
	template <typename T = double> class longitude : public impl::graticule<T>
	{
		public :
			/// Can be constructed with a value
			inline explicit longitude(T v) : impl::graticule<T>(v) {}
			/// Can be constructed with a WEST dms-class
			inline explicit longitude(const dms<west, T>& v) : impl::graticule<T>(v.as_value()) {}
			/// Can be constructed with an EAST dms-class
			inline explicit longitude(const dms<east, T>& v) : impl::graticule<T>(v.as_value()) {}
	};

} // namespace geometry


#endif // _GEOMETRY_GRATICULE_HPP
