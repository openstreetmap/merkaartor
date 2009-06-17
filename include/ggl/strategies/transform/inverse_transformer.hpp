// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GGL_STRATEGIES_TRANSFORM_INVERSE_TRANSFORMER_HPP
#define GGL_STRATEGIES_TRANSFORM_INVERSE_TRANSFORMER_HPP

#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <ggl/strategies/transform/matrix_transformers.hpp>

namespace ggl
{

namespace strategy { namespace transform {

/*!
    \brief Transformation strategy to do an inverse ransformation in Cartesian system
    \ingroup transform
    \tparam P1 first point type
    \tparam P2 second point type
 */
template <typename P1, typename P2>
struct inverse_transformer 
    : ublas_transformer<P1, P2, dimension<P1>::type::value, dimension<P2>::type::value>
{
    typedef typename select_coordinate_type<P1, P2>::type T;

    template <typename MatrixType>
    inline inverse_transformer(const MatrixType& input)
    {
        typedef boost::numeric::ublas::matrix<double> matrix_type;
        // create a working copy of the input
        matrix_type copy(input.matrix());

        // create a permutation matrix for the LU-factorization
        typedef boost::numeric::ublas::permutation_matrix<> permutation_matrix;
        permutation_matrix pm(copy.size1());

        // perform LU-factorization
        int res = boost::numeric::ublas::lu_factorize<matrix_type>(copy, pm);
        if( res == 0 )
        {
            // create identity matrix
            this->m_matrix.assign(boost::numeric::ublas::identity_matrix<T>(copy.size1()));

            // backsubstitute to get the inverse
            boost::numeric::ublas::lu_substitute(copy, pm, this->m_matrix);
        }
    }


};


}} // namespace strategy::transform


} // namespace ggl

#endif // GGL_STRATEGIES_TRANSFORM_INVERSE_TRANSFORMER_HPP
