// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGIES_MATRIX_TRANSFORMERS_HPP
#define GGL_STRATEGIES_MATRIX_TRANSFORMERS_HPP


// Remove the ublas checking, otherwise the inverse might fail (while nothing seems to be wrong)
#define BOOST_UBLAS_TYPE_CHECK 0

#include <boost/numeric/conversion/cast.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <ggl/core/coordinate_dimension.hpp>


namespace ggl
{


namespace strategy { namespace transform {


/*!
    \brief Transformation strategy to do an affine matrix transformation in Cartesian system
    \ingroup transform
    \tparam P1 first point type
    \tparam P2 second point type
    \tparam Dimension1 number of dimensions to transform from first point, optional
    \tparam Dimension1 number of dimensions to transform to second point, optional
 */
template
<
    typename P1, typename P2,
    std::size_t Dimension1,
    std::size_t Dimension2
>
class ublas_transformer
{
};

template <typename P1, typename P2>
class ublas_transformer<P1, P2, 2, 2>
{
protected :
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;
    typedef coordinate_type ct; // Abbreviation
    typedef boost::numeric::ublas::matrix<coordinate_type> matrix_type;
    matrix_type m_matrix;

public :
    inline ublas_transformer(
                ct const& m_0_0, ct const& m_0_1, ct const& m_0_2,
                ct const& m_1_0, ct const& m_1_1, ct const& m_1_2,
                ct const& m_2_0, ct const& m_2_1, ct const& m_2_2)
        : m_matrix(3, 3)
    {
        m_matrix(0,0) = m_0_0;   m_matrix(0,1) = m_0_1;   m_matrix(0,2) = m_0_2;
        m_matrix(1,0) = m_1_0;   m_matrix(1,1) = m_1_1;   m_matrix(1,2) = m_1_2;
        m_matrix(2,0) = m_2_0;   m_matrix(2,1) = m_2_1;   m_matrix(2,2) = m_2_2;
    }

    inline ublas_transformer()
        : m_matrix(3, 3)
    {
    }


    inline bool operator()(const P1& p1, P2& p2) const
    {
        assert_dimension_greater_equal<P1, 2>();
        assert_dimension_greater_equal<P2, 2>();

        const coordinate_type& c1 = get<0>(p1);
        const coordinate_type& c2 = get<1>(p1);

        typedef typename ggl::coordinate_type<P2>::type ct2;

        set<0>(p2, boost::numeric_cast<ct2>(c1 * m_matrix(0,0)
                + c2 * m_matrix(0,1) + m_matrix(0,2)));
        set<1>(p2, boost::numeric_cast<ct2>(c1 * m_matrix(1,0)
                + c2 * m_matrix(1,1) + m_matrix(1,2)));

        return true;
    }

    const matrix_type& matrix() const { return m_matrix; }
};

// It IS possible to go from 3 to 2 coordinates
template <typename P1, typename P2>
struct ublas_transformer<P1, P2, 3, 2>
    : public ublas_transformer<P1, P2, 2, 2>
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;
    typedef coordinate_type ct; // Abbreviation

    inline ublas_transformer(
                ct const& m_0_0, ct const& m_0_1, ct const& m_0_2,
                ct const& m_1_0, ct const& m_1_1, ct const& m_1_2,
                ct const& m_2_0, ct const& m_2_1, ct const& m_2_2)
        : ublas_transformer<P1, P2, 2, 2>(
                    m_0_0, m_0_1, m_0_2,
                    m_1_0, m_1_1, m_1_2,
                    m_2_0, m_2_1, m_2_2)
    {}

    inline ublas_transformer()
        : ublas_transformer<P1, P2, 2, 2>()
    {}
};


template <typename P1, typename P2>
class ublas_transformer<P1, P2, 3, 3>
{
protected :
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;
    typedef coordinate_type ct; // Abbreviation
    typedef boost::numeric::ublas::matrix<coordinate_type> matrix_type;
    matrix_type m_matrix;

    inline ublas_transformer(
                ct const& m_0_0, ct const& m_0_1, ct const& m_0_2, ct const& m_0_3,
                ct const& m_1_0, ct const& m_1_1, ct const& m_1_2, ct const& m_1_3,
                ct const& m_2_0, ct const& m_2_1, ct const& m_2_2, ct const& m_2_3,
                ct const& m_3_0, ct const& m_3_1, ct const& m_3_2, ct const& m_3_3
                )
        : m_matrix(4, 4)
    {
        m_matrix(0,0) = m_0_0; m_matrix(0,1) = m_0_1; m_matrix(0,2) = m_0_2; m_matrix(0,3) = m_0_3;
        m_matrix(1,0) = m_1_0; m_matrix(1,1) = m_1_1; m_matrix(1,2) = m_1_2; m_matrix(1,3) = m_1_3;
        m_matrix(2,0) = m_2_0; m_matrix(2,1) = m_2_1; m_matrix(2,2) = m_2_2; m_matrix(2,3) = m_2_3;
        m_matrix(3,0) = m_3_0; m_matrix(3,1) = m_3_1; m_matrix(3,2) = m_3_2; m_matrix(3,3) = m_3_3;
    }

    inline ublas_transformer()
        : m_matrix(4, 4)
    {
    }


public :

    inline bool operator()(const P1& p1, P2& p2) const
    {
        const coordinate_type& c1 = get<0>(p1);
        const coordinate_type& c2 = get<1>(p1);
        const coordinate_type& c3 = get<2>(p1);

        typedef typename ggl::coordinate_type<P2>::type ct2;

        set<0>(p2, boost::numeric_cast<ct2>(c1 * m_matrix(0,0)
                + c2 * m_matrix(0,1) + c3 * m_matrix(0,2) + m_matrix(0,3)));
        set<1>(p2, boost::numeric_cast<ct2>(c1 * m_matrix(1,0)
                + c2 * m_matrix(1,1) + c3 * m_matrix(1,2) + m_matrix(1,3)));
        set<2>(p2, boost::numeric_cast<ct2>(c1 * m_matrix(2,0)
                + c2 * m_matrix(2,1) + c3 * m_matrix(2,2) + m_matrix(2,3)));

        return true;
    }

    const matrix_type& matrix() const { return m_matrix; }
};




/*!
    \brief Transformation strategy to translate in Cartesian system
    \ingroup transform
    \see http://www.devmaster.net/wiki/Transformation_matrices
    \tparam P1 first point type
    \tparam P2 second point type
    \tparam Dimension1 number of dimensions to transform from first point, optional
    \tparam Dimension1 number of dimensions to transform to second point, optional
 */
template
<
    typename P1, typename P2,
    std::size_t Dimension1 = ggl::dimension<P1>::type::value,
    std::size_t Dimension2 = ggl::dimension<P2>::type::value
>
struct translate_transformer {};


template <typename P1, typename P2>
struct translate_transformer<P1, P2, 2, 2>
        : ublas_transformer<P1, P2, 2, 2>
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;

    // To have translate transformers compatible for 2/3 dimensions, the
    // constructor takes an optional third argument doing nothing.
    inline translate_transformer(coordinate_type const& translate_x,
                coordinate_type const& translate_y,
                coordinate_type const& dummy = 0)
        : ublas_transformer<P1, P2, 2, 2>(
                1, 0, translate_x,
                0, 1, translate_y,
                0, 0, 1)
    {}

};

template <typename P1, typename P2>
struct translate_transformer<P1, P2, 3, 3> : ublas_transformer<P1, P2, 3, 3>
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;

    inline translate_transformer(coordinate_type const& translate_x,
                coordinate_type const& translate_y,
                coordinate_type const& translate_z)
        : ublas_transformer<P1, P2, 3, 3>(
                1, 0, 0, translate_x,
                0, 1, 0, translate_y,
                0, 0, 1, translate_z,
                0, 0, 0, 1)
    {}

};


/*!
    \brief Transformation strategy to scale in Cartesian system
    \ingroup transform
    \see http://www.devmaster.net/wiki/Transformation_matrices
    \tparam P1 first point type
    \tparam P2 second point type
    \tparam Dimension1 number of dimensions to transform from first point, optional
    \tparam Dimension1 number of dimensions to transform to second point, optional
*/
template
<
    typename P1, typename P2,
    std::size_t Dimension1 = ggl::dimension<P1>::type::value,
    std::size_t Dimension2 = ggl::dimension<P2>::type::value
>
struct scale_transformer {};


template <typename P1, typename P2>
struct scale_transformer<P1, P2, 2, 2> : ublas_transformer<P1, P2, 2, 2>
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;

    inline scale_transformer(coordinate_type const& scale_x,
                coordinate_type const& scale_y,
                coordinate_type const& dummy = 0)
        : ublas_transformer<P1, P2, 2, 2>(
                scale_x, 0,       0,
                0,       scale_y, 0,
                0,       0,       1)
    {}
};


template <typename P1, typename P2>
struct scale_transformer<P1, P2, 3, 3> : ublas_transformer<P1, P2, 3, 3>
{
    typedef typename select_coordinate_type<P1, P2>::type coordinate_type;

    inline scale_transformer(coordinate_type const& scale_x,
                coordinate_type const& scale_y,
                coordinate_type const& scale_z)
        : ublas_transformer<P1, P2, 3, 3>(
                scale_x, 0,       0,       0,
                0,       scale_y, 0,       0,
                0,       0,       scale_z, 0,
                0,       0,       0,       1)
    {}
};



#ifndef DOXYGEN_NO_DETAIL
namespace detail {

template <typename DegreeOrRadian>
struct as_radian {};

template <>
struct as_radian<radian>
{
    static inline double get(double const& value)
    {
        return value;
    }
};

template <>
struct as_radian<degree>
{
    static inline double get(double const& value)
    {
        return value * math::d2r;
    }

};


template
<
    typename P1, typename P2,
    std::size_t Dimension1 = ggl::dimension<P1>::type::value,
    std::size_t Dimension2 = ggl::dimension<P2>::type::value
>
struct rad_rotate_transformer
    : ublas_transformer<P1, P2, Dimension1, Dimension2>
{
    inline rad_rotate_transformer(double const& angle)
        : ublas_transformer<P1, P2, Dimension1, Dimension2>(
                 cos(angle), sin(angle), 0,
                -sin(angle), cos(angle), 0,
                 0,          0,          1)
    {}
};

}
#endif


/*!
    \brief Transformation strategy to rotate in Cartesian system
    \ingroup transform
    \tparam P1 first point type
    \tparam P2 second point type
    \tparam DegreeOrRadian degree/or/radian, type of rotation angle specification
    \note Not yet in 3D, the 3D version requires special things to allow for
      rotation around X, Y, Z or arbitrary axis
    \see http://www.devmaster.net/wiki/Transformation_matrices
    \note The 3D version will not compile
 */
template <typename P1, typename P2, typename DegreeOrRadian>
struct rotate_transformer : detail::rad_rotate_transformer<P1, P2>
{
    inline rotate_transformer(double const& angle)
        : detail::rad_rotate_transformer
            <
                P1, P2
            >(detail::as_radian<DegreeOrRadian>::get(angle))
    {}

};


}} // namespace strategy::transform

} // namespace ggl

#endif // GGL_STRATEGIES_MATRIX_TRANSFORMERS_HPP
