/*
 * sk_effects.h - Classical Three-Dimensional Artwork for Qt 4
 *
 * Copyright (c) 2008 Christoph Feck <christoph@maxiom.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef SKULPTURE_EFFECTS_H
#define SKULPTURE_EFFECTS_H 1


/*-----------------------------------------------------------------------*/

#include <QtGui/QRgb>

#define F_SHIFT 11

void filterRgbPixels(QRgb *rgb, int w, int h, int stride, int f);


/*-----------------------------------------------------------------------*/

#include <QtGui/QImage>
#include <cmath>

static inline void filterImage(QImage &im, double f)
{
	filterRgbPixels((QRgb *) im.bits(), im.width(), im.height(), im.bytesPerLine() / sizeof(QRgb), int((1 << F_SHIFT) * f));
}


static inline void blurImage(QImage &im, int radius)
{
	if (radius >= 1) {
		double f = 1.0 - exp(-2.3 / (radius + 1.0));
		filterImage(im, f);
	}
}


/*-----------------------------------------------------------------------*/

#endif


