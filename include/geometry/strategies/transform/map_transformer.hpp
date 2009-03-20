// Generic Geometry Library
//
// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _GEOMETRY_STRATEGY_MAP_TRANSFORMER_HPP
#define _GEOMETRY_STRATEGY_MAP_TRANSFORMER_HPP


#include <geometry/strategies/transform/matrix_transformers.hpp>


namespace geometry
{
	namespace strategy
	{
		namespace transform
		{

			/*!
				\brief Transformation strategy to do map from one to another Cartesian system
				\ingroup transform
				\tparam P1 first point type
				\tparam P2 second point type
			 */
			template <typename P1, typename P2, bool MIRROR>
			struct map_transformer : ublas_transformer<P1, P2>
			{
				typedef typename select_coordinate_type<P1, P2>::type T;
				typedef boost::numeric::ublas::matrix<T> M;

				template <typename B, typename D>
				explicit inline map_transformer(const B& box, const D& width, const D& height)
				{
					set_transformation(
							get<min_corner, 0>(box), get<min_corner, 1>(box),
							get<max_corner, 0>(box), get<max_corner, 1>(box),
							width, height);
				}

				template <typename W, typename D>
				explicit inline map_transformer(const W& wx1, const W& wy1, const W& wx2, const W& wy2,
									const D& width, const D& height)
				{
					set_transformation(wx1, wy1, wx2, wy2, width, height);
				}




				private :
					void set_transformation_point(double wx, double wy, double px, double py, double scale)
					{

						// Translate to a coordinate system centered on world coordinates (-wx, -wy)
						M t1(3,3);
						t1(0,0) = 1;   t1(0,1) = 0;   t1(0,2) = -wx;
						t1(1,0) = 0;   t1(1,1) = 1;   t1(1,2) = -wy;
						t1(2,0) = 0;   t1(2,1) = 0;   t1(2,2) = 1;

						// Scale the map
						M s(3,3);
						s(0,0) = scale;   s(0,1) = 0;   s(0,2) = 0;
						s(1,0) = 0;    s(1,1) = scale;  s(1,2) = 0;
						s(2,0) = 0;    s(2,1) = 0;      s(2,2) = 1;

						// Translate to a coordinate system centered on the specified pixels (+px, +py)
						M t2(3, 3);
						t2(0,0) = 1;   t2(0,1) = 0;   t2(0,2) = px;
						t2(1,0) = 0;   t2(1,1) = 1;   t2(1,2) = py;
						t2(2,0) = 0;   t2(2,1) = 0;   t2(2,2) = 1;

						// Calculate combination matrix in two steps
						this->m_matrix = boost::numeric::ublas::prod(s, t1);
						this->m_matrix = boost::numeric::ublas::prod(t2, this->m_matrix);
					}


					template <typename W, typename D>
					void set_transformation(const W& wx1, const W& wy1, const W& wx2, const W& wy2,
									const D& width, const D& height)
					{
						D px1 = 0;
						D py1 = 0;
						D px2 = width;
						D py2 = height;


						// Calculate appropriate scale, take min because whole box must fit
						// Scale is in PIXELS/MAPUNITS (meters)
						double s1 = (px2 - px1) / (wx2 - wx1);
						double s2 = (py2 - py1) / (wy2 - wy1);

// TEMP, undefine MS "min" macro, todo: solve this better
#undef min
						double scale = std::min(s1, s2);

						// Calculate centerpoints
						double wmx = (wx1 + wx2) / 2.0;
						double wmy = (wy1 + wy2) / 2.0;
						double pmx = (px1 + px2) / 2.0;
						double pmy = (py1 + py2) / 2.0;

						set_transformation_point(wmx, wmy, pmx, pmy, scale);

						if (MIRROR)
						{
							// Mirror in y-direction
							M m(3,3);
							m(0,0) = 1;   m(0,1) = 0;   m(0,2) = 0;
							m(1,0) = 0;   m(1,1) = -1;  m(1,2) = 0;
							m(2,0) = 0;   m(2,1) = 0;   m(2,2) = 1;

							// Translate in y-direction such that it fits again
							M y(3, 3);
							y(0,0) = 1;   y(0,1) = 0;   y(0,2) = 0;
							y(1,0) = 0;   y(1,1) = 1;   y(1,2) = height;
							y(2,0) = 0;   y(2,1) = 0;   y(2,2) = 1;

							// Calculate combination matrix in two steps
							this->m_matrix = boost::numeric::ublas::prod(m, this->m_matrix);
							this->m_matrix = boost::numeric::ublas::prod(y, this->m_matrix);
						}
					}

			};


		} // namespace transform


	} // namespace strategy


} // namespace geometry


#endif // _GEOMETRY_STRATEGY_MAP_TRANSFORMER_HPP
