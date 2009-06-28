// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP
#define GGL_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP

#include <utility>
#include <cmath>
#include <limits>
#include <iostream>

namespace ggl
{

/*!
    \brief Encapsulate the results of distance calculation
    \ingroup distance
    \details Distance calculation for xy points or xyz points is done by taking the square
    root. However, for distance comparison drawing the square root is not necessary.
    Therefore the distance strategies are allowed to return the squares of the distance.
    This structure contains the distance, and a boolean to indicate if it is squared.
    It has an automatic conversion to a double value, which does the square root if necessary.
    \note Thanks to Phil Endecott for his suggestion to change the pair to the double-convertable
    http://article.gmane.org/gmane.comp.lib.boost.devel/172709/match=greatcircle_distance
    \note It might be templatized with a T
*/
struct cartesian_distance
{
    private :
        typedef double T;
        T m_squared_distance;

    public :


        /// Constructor with a value
        explicit cartesian_distance(const T& v) : m_squared_distance(v) {}

        /// Automatic conversion to double, taking squareroot if necessary
        inline operator double() const
        {
            return sqrt(m_squared_distance);
        }

        // Compare squared values
        inline bool operator<(const cartesian_distance& other) const
        {
            return this->m_squared_distance < other.m_squared_distance;
        }
        inline bool operator>(const cartesian_distance& other) const
        {
            return this->m_squared_distance > other.m_squared_distance;
        }
        inline bool operator==(const cartesian_distance& other) const
        {
            return this->m_squared_distance == other.m_squared_distance;
        }

        // Compare just with a corresponding POD value
        inline bool operator<(const T& value) const
        {
            return this->m_squared_distance < (value * value);
        }
        inline bool operator>(const T& value) const
        {
            return this->m_squared_distance > (value * value);
        }
        inline bool operator==(const T& value) const
        {
            return this->m_squared_distance == (value * value);
        }

        // Utility method to compare without SQRT, but not with method above because for epsilon that
        // makes no sense...
        inline bool very_small() const
        {
            return m_squared_distance <= std::numeric_limits<T>::epsilon();
        }

        /// The "value" method returns the internal value, here: the squared value
        /// inline T value() const { return m_squared_distance; }


        /// Make streamable to enable std::cout << ggl::distance( )
        template <typename CH, typename TR>
        inline friend std::basic_ostream<CH, TR>& operator<<(std::basic_ostream<CH, TR>& os,
                        const cartesian_distance& d)
        {
            os << sqrt(d.m_squared_distance);
            return os;
        }

};



/*

    From Phil Endecott, on the list:

    You can go further.  If I'm searching through a long list of points to
    find the closest to P then I'll avoid the squaring (and conversion to
    double if my co-ordinates are integers) whenever possible.  You can
    achieve this with a more complex distance proxy:

    class distance_proxy {
       double dx;
       double dy;
       distance_proxy(double dx_, double dy_): dx(dx_), dy(dy_) {}
       friend pythag_distance(point,point);
    public:
       operator double() { return sqrt(dx*dx+dy*dy); }
       bool operator>(double d) {
         return dx>d
             || dy>d
             || (dx*dx+dy*dy > d*d);
       }
    };

    So this is convertible to double, but can be compared to a distance
    without any need for sqrt() and only multiplication in some cases.
    Further refinement is possible.


    Barend:
    feasable, needs to be templatized by the number of dimensions. For distance it
    results in a nice "delayed calculation".
    For searching you might take another approach, first calculate dx, if OK then dy,
    if OK then the sqrs. So as above but than distance does not need to be calculated.
    So it is in fact another strategy.


*/



#ifndef DOXYGEN_NO_DETAIL
namespace detail
{
    namespace distance
    {
        template <typename R, typename T>
        struct distance_result_maker
        {
        };

        template <typename T>
        struct distance_result_maker<cartesian_distance, T>
        {
            static inline cartesian_distance make(const T& value)
            {
                return cartesian_distance(value * value);
            }
        };

        template <typename T>
        struct distance_result_maker<double, T>
        {
            static inline double make(const T& value)
            {
                return value;
            }
        };

    }
}
#endif


/*!
    \brief Object generator to create instance which can be compared
    \ingroup distance
    \details If distance results have to be compared to a certain value it makes sense to use
    this function to generate a distance result of a certain value, and compare the distance
    result with this instance. SQRT calculations are then avoided
    \tparam R distance result type
    \tparam T arithmetic type, e.g. double
    \param value the distance to compare with
    \return the distance result
*/
template <typename R, typename T>
inline R make_distance_result(const T& value)
{
    return detail::distance::distance_result_maker<R, T>::make(value);
}


/*!
    \brief Utility function to check if a distance is very small
    \ingroup distance
    \details Depending on the "distance result" type it checks if it is smaller than epsilon,
    or (for Cartesian distances) if the square is smaller than epsilon
    \tparam R the distance result type, either arithmetic or cartesian distance
    \param value the distance result to check
*/
template <typename R>
inline bool close_to_zero(const R& value)
{
    return value <= std::numeric_limits<R>::epsilon();
}

// Specialization to do not take the square
#ifndef DOXYGEN_NO_SPECIALIZATIONS
template <>
inline bool close_to_zero<cartesian_distance>(const cartesian_distance& value)
{
    return value.very_small();
}
#endif

} // namespace ggl


#endif // GGL_GEOMETRY_STRATEGIES_DISTANCE_RESULT_HPP
