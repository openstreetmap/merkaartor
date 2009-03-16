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
#ifndef NAMEFINDERXMLSTREAMREADER_H
#define NAMEFINDERXMLSTREAMREADER_H

#include <QXmlStreamReader>
#include <QIODevice>

#include <NameFinderResult.h>

namespace NameFinder {

    /**	\brief Reads the XML file and places results in a QList of NameFinderResult
    	@author Łukasz Jernaś <deejay1@srem.org>
    */
    class XmlStreamReader {
public:
        XmlStreamReader(QIODevice *buffer);
        ~XmlStreamReader();
        //! Reads the XML result.
        bool read();
        QList<NameFinderResult> getResults();

private:
        NameFinderResult tmpResult;
        QList<NameFinderResult> myResults;
        QXmlStreamReader reader;
        QIODevice *myDevice;

        void readSearchResultsElement();
        void readNamedElement(QList<NameFinderResult> *results);
        void readDescriptionElement(NameFinderResult *result);
        //! Reads nearby places from the query
        void readNearestPlacesElement(NameFinderResult *result);
        void skipElement();
        int i;

    };

}

#endif

