/***************************************************************************
*   Copyright (C) 2008 by Łukasz Jernaś   *
*   deejay1@srem.org   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef NAMEFINDERWIDGET_RESULT_H_
#define NAMEFINDERWIDGET_RESULT_H_

#include <QList>
#include <QRectF>

namespace NameFinder {
//! Structure holding a single result of a query to the NameFinder service.
    /**
    *	@author Łukasz Jernaś <deejay1@srem.org>
    */
    class NameFinderResult {
public:
        //! Name of the place found
        QString name;
        //! Type of the place found
        QString type;
        //! Category of the place (maps to key concept)
        QString category;
        //! Value of the category
        QString info;
        //! Coord of the POI
        QPointF coord;
        //! BBox of the feature
        QRectF bbox;
        //! Distance from coord to viewport center
        double distance;
//        //! The description as returned by the service.
//        QString description;
//        //! QList holding places which are near the result.
//        QList<NameFinderResult> near;

        bool operator<(const NameFinderResult &other) const
        {
            return distance < other.distance;
        }

    };
}
#endif
