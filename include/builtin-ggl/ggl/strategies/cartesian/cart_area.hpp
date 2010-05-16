// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGY_CARTESIAN_AREA_HPP
#define GGL_STRATEGY_CARTESIAN_AREA_HPP


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

#include <ggl/geometries/point_xy.hpp>
#include <ggl/geometries/segment.hpp>


namespace ggl
{
namespace strategy
{
    namespace area
    {

        /*!
            \brief Strategy functor for area calculation on point_xy points
            \details Calculates area using well-known triangulation algorithm
            \tparam PS point type of segments
            \par Concepts for PS:
            - specialized point_traits class
        */
        template<typename PS, typename CalculationType = void>
        class by_triangles
        {
            public :
                // If user specified a calculation type, use that type,
                //   whatever it is and whatever the point-type is.
                // Else, use the pointtype, but at least double
                typedef typename
                    boost::mpl::if_c
                    <
                        boost::is_void<CalculationType>::type::value,
                        typename select_most_precise
                        <
                            typename coordinate_type<PS>::type,
                            double
                        >::type,
                        CalculationType
                    >::type return_type;


            private :

                struct summation
                {
                    return_type sum;

                    inline summation() : sum(return_type())
                    {
                        // Currently only 2D areas are supported
                        assert_dimension<PS, 2>();
                    }
                    inline return_type area() const
                    {
                        return_type result = sum;
                        result *= 0.5;
                        return result;
                    }
                };

            public :
                typedef summation state_type;

                inline bool operator()(segment<const PS> const& s,
                            state_type& state) const
                {
                    // SUM += x2 * y1 - x1 * y2;
                    state.sum += get<1, 0>(s) * get<0, 1>(s)
                                - get<0, 0>(s) * get<1, 1>(s);
                    return true;
                }

        };

    } // namespace area
} // namespace strategy


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
template <typename P>
struct strategy_area<cartesian_tag, P>
{
    typedef strategy::area::by_triangles<P> type;
};
#endif

} // namespace ggl


#endif // GGL_STRATEGY_CARTESIAN_AREA_HPP
