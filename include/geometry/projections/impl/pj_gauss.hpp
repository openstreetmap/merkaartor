#ifndef _PROJECTIONS_PJ_GAUSS_HPP
#define _PROJECTIONS_PJ_GAUSS_HPP

// Generic Geometry Library - projections (based on PROJ4)
// This file is manually converted from PROJ4

// Copyright Barend Gehrels 1995-2009, Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Geometry Library by Barend Gehrels (Geodan, Amsterdam)

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

namespace projection
{
	namespace impl
	{
		namespace gauss
		{
			static const int MAX_ITER = 20;

			struct GAUSS
			{
				double C;
				double K;
				double e;
				double ratexp;
			};

			static const double DEL_TOL = 1e-14;
			inline double srat(double esinp, double exp)
			{
				return(pow((1.-esinp)/(1.+esinp), exp));
			}


			GAUSS gauss_ini(double e, double phi0, double &chi, double &rc)
			{
				double sphi, cphi, es;
				GAUSS en;
				es = e * e;
				en.e = e;
				sphi = sin(phi0);
				cphi = cos(phi0);  cphi *= cphi;
				rc = sqrt(1. - es) / (1. - es * sphi * sphi);
				en.C = sqrt(1. + es * cphi * cphi / (1. - es));
				chi = asin(sphi / en.C);
				en.ratexp = 0.5 * en.C * e;
				en.K = tan(.5 * chi + impl::FORTPI) / (
					pow(tan(.5 * phi0 + impl::FORTPI), en.C) *
					srat(en.e * sphi, en.ratexp)  );
				return en;
			}


			template <typename T>
			inline void gauss(const GAUSS& en, T& lam, T& phi)
			{
				phi = 2. * atan( en.K *
					pow(tan(.5 * phi + FORTPI), en.C) *
					srat(en.e * sin(phi), en.ratexp) ) - HALFPI;
				lam *= en.C;
			}

			template <typename T>
			inline void inv_gauss(const GAUSS& en, T& lam, T& phi)
			{
				lam /= en.C;
				double num = pow(tan(.5 * phi + FORTPI)/en.K, 1./en.C);
				int i;
				for (i = MAX_ITER; i; --i)
				{
					double elp_phi = 2. * atan(num * srat(en.e * sin(phi), -.5 * en.e))
						- HALFPI;
					if (fabs(elp_phi - phi) < DEL_TOL) break;
					phi = elp_phi;
				}
				/* convergence failed */
				if (!i)
					throw proj_exception(-17);
			}

		}
	}



}

#endif
