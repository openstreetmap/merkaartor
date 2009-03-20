// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_MATRIX_TRANSFORMERS_HPP
#define _GEOMETRY_STRATEGY_MATRIX_TRANSFORMERS_HPP


// Remove the ublas checking, otherwise the inverse might fail (while nothing seems to be wrong)
#define BOOST_UBLAS_TYPE_CHECK 0


#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <boost/numeric/conversion/cast.hpp>


namespace geometry
{
	namespace strategy
	{
		namespace transform
		{


			/*!
				\brief Transformation strategy to do an affine matrix transformation in Cartesian system
				\ingroup transform
				\tparam P1 first point type
				\tparam P2 second point type
			 */
			template <typename P1, typename P2>
			class ublas_transformer
			{
				protected :
					typedef typename select_coordinate_type<P1, P2>::type T;
					typedef boost::numeric::ublas::matrix<T> M;
					M m_matrix;

					inline ublas_transformer(
								const T& m_0_0, const T& m_0_1, const T& m_0_2,
								const T& m_1_0, const T& m_1_1, const T& m_1_2,
								const T& m_2_0, const T& m_2_1, const T& m_2_2)
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


				public :


					inline bool operator()(const P1& p1, P2& p2) const
					{
						assert_dimension<P1, 2>();
						assert_dimension<P2, 2>();

						const T& c1 = get<0>(p1);
						const T& c2 = get<1>(p1);

						typedef typename coordinate_type<P2>::type TP;

						set<0>(p2, boost::numeric_cast<TP>(c1 * m_matrix(0,0) + c2 * m_matrix(0,1) + m_matrix(0,2)));
						set<1>(p2, boost::numeric_cast<TP>(c1 * m_matrix(1,0) + c2 * m_matrix(1,1) + m_matrix(1,2)));
						return true;
					}

					const M& matrix() const { return m_matrix; }
			};


			/*!
				\brief Transformation strategy to translate in Cartesian system
				\ingroup transform
				\tparam P1 first point type
				\tparam P2 second point type
			 */
			template <typename P1, typename P2>
			struct translate_transformer : ublas_transformer<P1, P2>
			{
				typedef typename select_coordinate_type<P1, P2>::type T;

				inline translate_transformer(const T& translate_x, const T& translate_y)
					: ublas_transformer<P1, P2>(
							1, 0, translate_x,
							0, 1, translate_y,
							0, 0, 1)
				{}
			};


			/*!
				\brief Transformation strategy to scale in Cartesian system
				\ingroup transform
				\tparam P1 first point type
				\tparam P2 second point type
			 */
			template <typename P1, typename P2>
			struct scale_transformer : ublas_transformer<P1, P2>
			{
				typedef typename select_coordinate_type<P1, P2>::type T;

				inline scale_transformer(const T& scale_x, const T& scale_y)
					: ublas_transformer<P1, P2>(
							scale_x, 0,       0,
							0,       scale_y, 0,
							0,       0,       1)
				{}
			};



			#ifndef DOXYGEN_NO_IMPL
			namespace impl
			{
				template <typename DR>
				struct as_radian {};

				template <>
				struct as_radian<radian>
				{
					static inline double get(const double& value)
					{
						return value;
					}
				};

				template <>
				struct as_radian<degree>
				{
					static inline double get(const double& value)
					{
						return value * math::d2r;
					}

				};


				template <typename P1, typename P2>
				struct rad_rotate_transformer : ublas_transformer<P1, P2>
				{
					inline rad_rotate_transformer(const double& angle)
						: ublas_transformer<P1, P2>(
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
				\tparam DR degree/or/radian, type of rotation angle specification
			 */
			template <typename P1, typename P2, typename DR>
			struct rotate_transformer : impl::rad_rotate_transformer<P1, P2>
			{
				inline rotate_transformer(const double& angle)
					: impl::rad_rotate_transformer<P1, P2>(impl::as_radian<DR>::get(angle))
				{}

			};


		} // namespace transform

	} // namespace strategy

} // namespace geometry


#endif // _GEOMETRY_STRATEGY_MATRIX_TRANSFORMERS_HPP
