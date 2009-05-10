#ifndef _PROJECTIONS_PJ_TRANSFORM_HPP
#define _PROJECTIONS_PJ_TRANSFORM_HPP

namespace projection
{
	namespace impl
	{

		#include <geometry/projections/parameters.hpp>
		#include "geometry/projections/impl/projects.hpp"
		#include "geometry/projections/impl/geocent.h"

		#ifndef SRS_WGS84_SEMIMAJOR
		#define SRS_WGS84_SEMIMAJOR 6378137.0
		#endif

		#ifndef SRS_WGS84_ESQUARED
		#define SRS_WGS84_ESQUARED 0.0066943799901413165
		#endif

		#define Dx_BF (defn->params().datum_params[0])
		#define Dy_BF (defn->params().datum_params[1])
		#define Dz_BF (defn->params().datum_params[2])
		#define Rx_BF (defn->params().datum_params[3])
		#define Ry_BF (defn->params().datum_params[4])
		#define Rz_BF (defn->params().datum_params[5])
		#define M_BF  (defn->params().datum_params[6])

		/* datum system errors */
		#define PJD_ERR_GEOCENTRIC -45
		typedef struct { double u, v; } projUV;
		typedef void *projPJ;
		#define projXY projUV
		#define projLP projUV

		/* 
		** This table is intended to indicate for any given error code in 
		** the range 0 to -44, whether that error will occur for all locations (ie.
		** it is a problem with the coordinate system as a whole) in which case the
		** value would be 0, or if the problem is with the point being transformed
		** in which case the value is 1. 
		**
		** At some point we might want to move this array in with the error message
		** list or something, but while experimenting with it this should be fine. 
		*/

		static const int transient_error[45] = {
			/*             0  1  2  3  4  5  6  7  8  9   */
			/* 0 to 9 */   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
			/* 10 to 19 */ 0, 0, 0, 0, 1, 1, 0, 1, 1, 1,  
			/* 20 to 29 */ 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
			/* 30 to 39 */ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 
			/* 40 to 44 */ 0, 0, 0, 0, 0 };

		//int pj_apply_gridshift( const char *nadgrids, int inverse, 
		//						long point_count, int point_offset,
		//						double *x, double *y, double *z )

		//{
		//	int grid_count = 0;
		//	PJ_GRIDINFO   **tables;
		//	int  i;
		//	int debug_flag = getenv( "PROJ_DEBUG" ) != NULL;
		//	static int debug_count = 0;

		//	pj_errno = 0;

		//	tables = pj_gridlist_from_nadgrids( nadgrids, &grid_count);
		//	if( tables == NULL || grid_count == 0 )
		//		return pj_errno;

		//	for( i = 0; i < point_count; i++ )
		//	{
		//		long io = i * point_offset;
		//		LP   input, output;
		//		int  itable;

		//		input.phi = y[io];
		//		input.lam = x[io];
		//		output.phi = HUGE_VAL;
		//		output.lam = HUGE_VAL;

		//		/* keep trying till we find a table that works */
		//		for( itable = 0; itable < grid_count; itable++ )
		//		{
		//			PJ_GRIDINFO *gi = tables[itable];
		//			struct CTABLE *ct = gi->ct;

		//			/* skip tables that don't match our point at all.  */
		//			if( ct->ll.phi > input.phi || ct->ll.lam > input.lam
		//				|| ct->ll.phi + (ct->lim.phi-1) * ct->del.phi < input.phi
		//				|| ct->ll.lam + (ct->lim.lam-1) * ct->del.lam < input.lam )
		//				continue;

		//			/* If we have child nodes, check to see if any of them apply. */
		//			if( gi->child != NULL )
		//			{
		//				PJ_GRIDINFO *child;

		//				for( child = gi->child; child != NULL; child = child->next )
		//				{
		//					struct CTABLE *ct1 = child->ct;

		//					if( ct1->ll.phi > input.phi || ct1->ll.lam > input.lam
		//					  || ct1->ll.phi+(ct1->lim.phi-1)*ct1->del.phi < input.phi
		//					  || ct1->ll.lam+(ct1->lim.lam-1)*ct1->del.lam < input.lam)
		//						continue;

		//					break;
		//				}

		//				/* we found a more refined child node to use */
		//				if( child != NULL )
		//				{
		//					gi = child;
		//					ct = child->ct;
		//				}
		//			}

		//			/* load the grid shift info if we don't have it. */
		//			if( ct->cvs == NULL && !pj_gridinfo_load( gi ) )
		//			{
		//				pj_errno = -38;
		//				return pj_errno;
		//			}
		//            
		//			output = nad_cvt( input, inverse, ct );
		//			if( output.lam != HUGE_VAL )
		//			{
		//				if( debug_flag && debug_count++ < 20 )
		//					fprintf( stderr,
		//							 "pj_apply_gridshift(): used %s\n",
		//							 ct->id );
		//				break;
		//			}
		//		}

		//		if( output.lam == HUGE_VAL )
		//		{
		//			if( debug_flag )
		//			{
		//				fprintf( stderr, 
		//						 "pj_apply_gridshift(): failed to find a grid shift table for\n"
		//						 "                      location (%.7fdW,%.7fdN)\n",
		//						 x[io] * RAD_TO_DEG, 
		//						 y[io] * RAD_TO_DEG );
		//				fprintf( stderr, 
		//						 "   tried: %s\n", nadgrids );
		//			}
		//        
		//			pj_errno = -38;
		//			return pj_errno;
		//		}
		//		else
		//		{
		//			y[io] = output.phi;
		//			x[io] = output.lam;
		//		}
		//	}

		//	return 0;
		//}

		/************************************************************************/
		/*                            pj_transform()                            */
		/*                                                                      */
		/*      Currently this function doesn't recognise if two projections    */
		/*      are identical (to short circuit reprojection) because it is     */
		/*      difficult to compare PJ structures (since there are some        */
		/*      projection specific components).                                */
		/************************************************************************/

		template <typename PRJF, typename PRJT>
		inline int pj_transform( PRJF *srcdefn, PRJT *dstdefn, long point_count, int point_offset, double *x, double *y, double *z )

		{
			long      i;
			int       need_datum_shift;

			int pj_errno = 0;

			if( point_offset == 0 )
				point_offset = 1;

		/* -------------------------------------------------------------------- */
		/*      Transform geocentric source coordinates to lat/long.            */
		/* -------------------------------------------------------------------- */
			if( srcdefn->params().is_geocent )
			{
				if( z == NULL )
				{
					pj_errno = PJD_ERR_GEOCENTRIC;
					return PJD_ERR_GEOCENTRIC;
				}

				if( srcdefn->params().to_meter != 1.0 )
				{
					for( i = 0; i < point_count; i++ )
					{
						if( x[point_offset*i] != HUGE_VAL )
						{
							x[point_offset*i] *= srcdefn->params().to_meter;
							y[point_offset*i] *= srcdefn->params().to_meter;
						}
					}
				}

				if( pj_geocentric_to_geodetic( srcdefn->params().a_orig, srcdefn->params().es_orig,
											   point_count, point_offset, 
											   x, y, z ) != 0) 
					return pj_errno;
			}

		/* -------------------------------------------------------------------- */
		/*      Transform source points to lat/long, if they aren't             */
		/*      already.                                                        */
		/* -------------------------------------------------------------------- */
			else if( !srcdefn->params().is_latlong )
			{
				for( i = 0; i < point_count; i++ )
				{
					projXY         projected_loc;
					projLP	       geodetic_loc;

					projected_loc.u = x[point_offset*i];
					projected_loc.v = y[point_offset*i];

					if( projected_loc.u == HUGE_VAL )
						continue;

					point_2d in(projected_loc.u, projected_loc.v);
					point_ll_deg out;

					if (!srcdefn->inverse(in, out)) {
						fprintf( stderr, 
							   "pj_transform(): source projection not invertable\n" );
						return -17;
					}
					geodetic_loc.u = out.lon()*PI/180.;
					geodetic_loc.v = out.lat()*PI/180.;

					if( pj_errno != 0 )
					{
						if( (pj_errno != 33 /*EDOM*/ && pj_errno != 34 /*ERANGE*/ )
							&& (pj_errno > 0 || pj_errno < -44 || point_count == 1
								|| transient_error[-pj_errno] == 0 ) )
							return pj_errno;
						else
						{
							geodetic_loc.u = HUGE_VAL;
							geodetic_loc.v = HUGE_VAL;
						}
					}

					x[point_offset*i] = geodetic_loc.u;
					y[point_offset*i] = geodetic_loc.v;
				}
			}
		/* -------------------------------------------------------------------- */
		/*      But if they are already lat long, adjust for the prime          */
		/*      meridian if there is one in effect.                             */
		/* -------------------------------------------------------------------- */
			if( srcdefn->params().from_greenwich != 0.0 )
			{
				for( i = 0; i < point_count; i++ )
				{
					if( x[point_offset*i] != HUGE_VAL )
						x[point_offset*i] += srcdefn->params().from_greenwich;
				}
			}

		/* -------------------------------------------------------------------- */
		/*      Convert datums if needed, and possible.                         */
		/* -------------------------------------------------------------------- */
			if( pj_datum_transform( srcdefn, dstdefn, point_count, point_offset, 
									x, y, z ) != 0 )
				return pj_errno;

		/* -------------------------------------------------------------------- */
		/*      But if they are staying lat long, adjust for the prime          */
		/*      meridian if there is one in effect.                             */
		/* -------------------------------------------------------------------- */
			if( dstdefn->params().from_greenwich != 0.0 )
			{
				for( i = 0; i < point_count; i++ )
				{
					if( x[point_offset*i] != HUGE_VAL )
						x[point_offset*i] -= dstdefn->params().from_greenwich;
				}
			}


		/* -------------------------------------------------------------------- */
		/*      Transform destination latlong to geocentric if required.        */
		/* -------------------------------------------------------------------- */
			if( dstdefn->params().is_geocent )
			{
				if( z == NULL )
				{
					pj_errno = PJD_ERR_GEOCENTRIC;
					return PJD_ERR_GEOCENTRIC;
				}

				pj_geodetic_to_geocentric( dstdefn->params().a_orig, dstdefn->params().es_orig,
										   point_count, point_offset, x, y, z );

				if( dstdefn->params().fr_meter != 1.0 )
				{
					for( i = 0; i < point_count; i++ )
					{
						if( x[point_offset*i] != HUGE_VAL )
						{
							x[point_offset*i] *= dstdefn->params().fr_meter;
							y[point_offset*i] *= dstdefn->params().fr_meter;
						}
					}
				}
			}

		/* -------------------------------------------------------------------- */
		/*      Transform destination points to projection coordinates, if      */
		/*      desired.                                                        */
		/* -------------------------------------------------------------------- */
			else if( !dstdefn->params().is_latlong )
			{
				for( i = 0; i < point_count; i++ )
				{
					projXY         projected_loc;
					projLP	       geodetic_loc;

					geodetic_loc.u = x[point_offset*i];
					geodetic_loc.v = y[point_offset*i];

					if( geodetic_loc.u == HUGE_VAL )
						continue;

					point_ll_deg in(longitude<>(geodetic_loc.u*180/PI), latitude<>(geodetic_loc.v*180/PI));
					point_2d out;

					dstdefn->forward(in, out);
					projected_loc.u = out.x();
					projected_loc.v = out.y();

					if( pj_errno != 0 )
					{
						if( (pj_errno != 33 /*EDOM*/ && pj_errno != 34 /*ERANGE*/ )
							&& (pj_errno > 0 || pj_errno < -44 || point_count == 1
								|| transient_error[-pj_errno] == 0 ) )
							return pj_errno;
						else
						{
							projected_loc.u = HUGE_VAL;
							projected_loc.v = HUGE_VAL;
						}
					}

					x[point_offset*i] = projected_loc.u;
					y[point_offset*i] = projected_loc.v;
				}
			}

		/* -------------------------------------------------------------------- */
		/*      If a wrapping center other than 0 is provided, rewrap around    */
		/*      the suggested center (for latlong coordinate systems only).     */
		/* -------------------------------------------------------------------- */
			else if( dstdefn->params().is_latlong && dstdefn->params().long_wrap_center != 0 )
			{
				for( i = 0; i < point_count; i++ )
				{
					if( x[point_offset*i] == HUGE_VAL )
						continue;

					while( x[point_offset*i] < dstdefn->params().long_wrap_center - HALFPI )
						x[point_offset*i] += PI;
					while( x[point_offset*i] > dstdefn->params().long_wrap_center + HALFPI )
						x[point_offset*i] -= PI;
				}
			}

			return 0;
		}

		/************************************************************************/
		/*                     pj_geodetic_to_geocentric()                      */
		/************************************************************************/

		int pj_geodetic_to_geocentric( double a, double es, 
									   long point_count, int point_offset,
									   double *x, double *y, double *z )

		{
			double b;
			int    i;
			GeocentricInfo gi;

			int pj_errno = 0;

			if( es == 0.0 )
				b = a;
			else
				b = a * sqrt(1-es);

			if( pj_Set_Geocentric_Parameters( &gi, a, b ) != 0 )
			{
				pj_errno = PJD_ERR_GEOCENTRIC;
				return pj_errno;
			}

			for( i = 0; i < point_count; i++ )
			{
				long io = i * point_offset;

				if( x[io] == HUGE_VAL  )
					continue;

				if( pj_Convert_Geodetic_To_Geocentric( &gi, y[io], x[io], z[io], 
													   x+io, y+io, z+io ) != 0 )
				{
					pj_errno = -14;
					x[io] = y[io] = HUGE_VAL;
					/* but keep processing points! */
				}
			}

			return pj_errno;
		}

		/************************************************************************/
		/*                     pj_geodetic_to_geocentric()                      */
		/************************************************************************/

		int pj_geocentric_to_geodetic( double a, double es, 
									   long point_count, int point_offset,
									   double *x, double *y, double *z )

		{
			double b;
			int    i;
			GeocentricInfo gi;

			if( es == 0.0 )
				b = a;
			else
				b = a * sqrt(1-es);

			if( pj_Set_Geocentric_Parameters( &gi, a, b ) != 0 )
			{
				int pj_errno = PJD_ERR_GEOCENTRIC;
				return pj_errno;
			}

			for( i = 0; i < point_count; i++ )
			{
				long io = i * point_offset;

				if( x[io] == HUGE_VAL )
					continue;

				pj_Convert_Geocentric_To_Geodetic( &gi, x[io], y[io], z[io], 
												   y+io, x+io, z+io );
			}

			return 0;
		}

		/************************************************************************/
		/*                         pj_compare_datums()                          */
		/*                                                                      */
		/*      Returns TRUE if the two datums are identical, otherwise         */
		/*      FALSE.                                                          */
		/************************************************************************/

		template <typename PRJF, typename PRJT>
		int pj_compare_datums( PRJF *srcdefn, PRJT *dstdefn )

		{
			if( srcdefn->params().datum_type != dstdefn->params().datum_type )
			{
				return 0;
			}
			else if( srcdefn->params().a_orig != dstdefn->params().a_orig 
					 || abs(srcdefn->params().es_orig - dstdefn->params().es_orig) > 0.000000000050 )
			{
				/* the tolerence for es is to ensure that GRS80 and WGS84 are
				   considered identical */
				return 0;
			}
			else if( srcdefn->params().datum_type == PJD_3PARAM )
			{
				return (srcdefn->params().datum_params[0] == dstdefn->params().datum_params[0]
						&& srcdefn->params().datum_params[1] == dstdefn->params().datum_params[1]
						&& srcdefn->params().datum_params[2] == dstdefn->params().datum_params[2]);
			}
			else if( srcdefn->params().datum_type == PJD_7PARAM )
			{
				return (srcdefn->params().datum_params[0] == dstdefn->params().datum_params[0]
						&& srcdefn->params().datum_params[1] == dstdefn->params().datum_params[1]
						&& srcdefn->params().datum_params[2] == dstdefn->params().datum_params[2]
						&& srcdefn->params().datum_params[3] == dstdefn->params().datum_params[3]
						&& srcdefn->params().datum_params[4] == dstdefn->params().datum_params[4]
						&& srcdefn->params().datum_params[5] == dstdefn->params().datum_params[5]
						&& srcdefn->params().datum_params[6] == dstdefn->params().datum_params[6]);
			}
			else if( srcdefn->params().datum_type == PJD_GRIDSHIFT )
			{
				return strcmp( pj_param(srcdefn->params().params,"snadgrids").s.c_str(),
							   pj_param(dstdefn->params().params,"snadgrids").s.c_str() ) == 0;
			}
			else
				return 1;
		}

		/************************************************************************/
		/*                       pj_geocentic_to_wgs84()                        */
		/************************************************************************/

		template <typename PJ>
		int pj_geocentric_to_wgs84( PJ *defn, 
									long point_count, int point_offset,
									double *x, double *y, double *z )

		{
			int       i;

			int pj_errno = 0;

			if( defn->params().datum_type == PJD_3PARAM )
			{
				for( i = 0; i < point_count; i++ )
				{
					long io = i * point_offset;
		            
					if( x[io] == HUGE_VAL )
						continue;

					x[io] = x[io] + Dx_BF;
					y[io] = y[io] + Dy_BF;
					z[io] = z[io] + Dz_BF;
				}
			}
			else if( defn->params().datum_type == PJD_7PARAM )
			{
				for( i = 0; i < point_count; i++ )
				{
					long io = i * point_offset;
					double x_out, y_out, z_out;

					if( x[io] == HUGE_VAL )
						continue;

					x_out = M_BF*(       x[io] - Rz_BF*y[io] + Ry_BF*z[io]) + Dx_BF;
					y_out = M_BF*( Rz_BF*x[io] +       y[io] - Rx_BF*z[io]) + Dy_BF;
					z_out = M_BF*(-Ry_BF*x[io] + Rx_BF*y[io] +       z[io]) + Dz_BF;

					x[io] = x_out;
					y[io] = y_out;
					z[io] = z_out;
				}
			}

			return 0;
		}

		/************************************************************************/
		/*                      pj_geocentic_from_wgs84()                       */
		/************************************************************************/

		template <typename PJ>
		int pj_geocentric_from_wgs84( PJ *defn, 
									  long point_count, int point_offset,
									  double *x, double *y, double *z )

		{
			int       i;

			int pj_errno = 0;

			if( defn->params().datum_type == PJD_3PARAM )
			{
				for( i = 0; i < point_count; i++ )
				{
					long io = i * point_offset;

					if( x[io] == HUGE_VAL )
						continue;
		            
					x[io] = x[io] - Dx_BF;
					y[io] = y[io] - Dy_BF;
					z[io] = z[io] - Dz_BF;
				}
			}
			else if( defn->params().datum_type == PJD_7PARAM )
			{
				for( i = 0; i < point_count; i++ )
				{
					long io = i * point_offset;
					double x_tmp, y_tmp, z_tmp;

					if( x[io] == HUGE_VAL )
						continue;

					x_tmp = (x[io] - Dx_BF) / M_BF;
					y_tmp = (y[io] - Dy_BF) / M_BF;
					z_tmp = (z[io] - Dz_BF) / M_BF;

					x[io] =        x_tmp + Rz_BF*y_tmp - Ry_BF*z_tmp;
					y[io] = -Rz_BF*x_tmp +       y_tmp + Rx_BF*z_tmp;
					z[io] =  Ry_BF*x_tmp - Rx_BF*y_tmp +       z_tmp;
				}
			}

			return 0;
		}

		/************************************************************************/
		/*                         pj_datum_transform()                         */
		/*                                                                      */
		/*      The input should be long/lat/z coordinates in radians in the    */
		/*      source datum, and the output should be long/lat/z               */
		/*      coordinates in radians in the destination datum.                */
		/************************************************************************/

		template <typename PRJF, typename PRJT>
		int pj_datum_transform( PRJF *srcdefn, PRJT *dstdefn, 
								long point_count, int point_offset,
								double *x, double *y, double *z )

		{
			double      src_a, src_es, dst_a, dst_es;
			int         z_is_temp = FALSE;

			int pj_errno = 0;

		/* -------------------------------------------------------------------- */
		/*      We cannot do any meaningful datum transformation if either      */
		/*      the source or destination are of an unknown datum type          */
		/*      (ie. only a +ellps declaration, no +datum).  This is new        */
		/*      behavior for PROJ 4.6.0.                                        */
		/* -------------------------------------------------------------------- */
			if( srcdefn->params().datum_type == PJD_UNKNOWN
				|| dstdefn->params().datum_type == PJD_UNKNOWN )
				return 0;

		/* -------------------------------------------------------------------- */
		/*      Short cut if the datums are identical.                          */
		/* -------------------------------------------------------------------- */
			if( pj_compare_datums( srcdefn, dstdefn ) )
				return 0;

			src_a = srcdefn->params().a_orig;
			src_es = srcdefn->params().es_orig;

			dst_a = dstdefn->params().a_orig;
			dst_es = dstdefn->params().es_orig;

		/* -------------------------------------------------------------------- */
		/*      Create a temporary Z array if one is not provided.              */
		/* -------------------------------------------------------------------- */
			if( z == NULL )
			{
				int	bytes = sizeof(double) * point_count * point_offset;
				z = (double *) malloc(bytes);
				memset( z, 0, bytes );
				z_is_temp = TRUE;
			}

		#define CHECK_RETURN {if( pj_errno != 0 && (pj_errno > 0 || transient_error[-pj_errno] == 0) ) { if( z_is_temp ) free(z); return pj_errno; }}

		/* -------------------------------------------------------------------- */
		/*	If this datum requires grid shifts, then apply it to geodetic   */
		/*      coordinates.                                                    */
		/* -------------------------------------------------------------------- */
			//if( srcdefn->params().datum_type == PJD_GRIDSHIFT )
			//{
			//	pj_apply_gridshift( pj_param(srcdefn->params().params,"snadgrids").s, 0, 
			//						point_count, point_offset, x, y, z );
			//	CHECK_RETURN;

			//	src_a = SRS_WGS84_SEMIMAJOR;
			//	src_es = SRS_WGS84_ESQUARED;
			//}

			//if( dstdefn->params().datum_type == PJD_GRIDSHIFT )
			//{
			//	dst_a = SRS_WGS84_SEMIMAJOR;
			//	dst_es = SRS_WGS84_ESQUARED;
			//}
		        
		/* ==================================================================== */
		/*      Do we need to go through geocentric coordinates?                */
		/* ==================================================================== */
			if( src_es != dst_es || src_a != dst_a
				|| srcdefn->params().datum_type == PJD_3PARAM 
				|| srcdefn->params().datum_type == PJD_7PARAM
				|| dstdefn->params().datum_type == PJD_3PARAM 
				|| dstdefn->params().datum_type == PJD_7PARAM)
			{
		/* -------------------------------------------------------------------- */
		/*      Convert to geocentric coordinates.                              */
		/* -------------------------------------------------------------------- */
				pj_geodetic_to_geocentric( src_a, src_es,
										   point_count, point_offset, x, y, z );
				CHECK_RETURN;

		/* -------------------------------------------------------------------- */
		/*      Convert between datums.                                         */
		/* -------------------------------------------------------------------- */
				if( srcdefn->params().datum_type == PJD_3PARAM 
					|| srcdefn->params().datum_type == PJD_7PARAM )
				{
					pj_geocentric_to_wgs84( srcdefn, point_count, point_offset,x,y,z);
					CHECK_RETURN;
				}

				if( dstdefn->params().datum_type == PJD_3PARAM 
					|| dstdefn->params().datum_type == PJD_7PARAM )
				{
					pj_geocentric_from_wgs84( dstdefn, point_count,point_offset,x,y,z);
					CHECK_RETURN;
				}

		/* -------------------------------------------------------------------- */
		/*      Convert back to geodetic coordinates.                           */
		/* -------------------------------------------------------------------- */
				pj_geocentric_to_geodetic( dst_a, dst_es,
										   point_count, point_offset, x, y, z );
				CHECK_RETURN;
			}

		/* -------------------------------------------------------------------- */
		/*      Apply grid shift to destination if required.                    */
		/* -------------------------------------------------------------------- */
			//if( dstdefn->params().datum_type == PJD_GRIDSHIFT )
			//{
			//	pj_apply_gridshift( pj_param(dstdefn->params().params,"snadgrids").s, 1,
			//						point_count, point_offset, x, y, z );
			//	CHECK_RETURN;
			//}

			if( z_is_temp )
				free( z );

			return 0;
		}
	}
}

#endif