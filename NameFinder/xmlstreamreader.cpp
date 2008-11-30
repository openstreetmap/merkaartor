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
#include "xmlstreamreader.h"

namespace NameFinder {

    XmlStreamReader::XmlStreamReader(QIODevice *device) {
        myDevice = device;
    }


    XmlStreamReader::~XmlStreamReader() {
    }

    bool XmlStreamReader::read() {
        myDevice->open(QIODevice::ReadOnly | QIODevice::Text);
        reader.setDevice(myDevice);
        reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isStartElement()) {
                if (reader.name() == "searchresults") {
                    readSearchResultsElement();
                } else {
                    reader.raiseError(QObject::tr("Not a proper results stream!"));
                }
            } else {
                reader.readNext();
            }
        }
        myDevice->close();
// Implement error handling
        return true;
    }
    void XmlStreamReader::readSearchResultsElement() {
        reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            }

            if (reader.isStartElement()) {
                if (reader.name() == "named") {
                    readNamedElement(&myResults);
                } else {
                    skipElement();
                }
            } else {
                reader.readNext();
            }
        }

    }
    void XmlStreamReader::readNamedElement(QList<NameFinderResult> *results) {
        NameFinderResult myResult;
        myResult.name = reader.attributes().value("name").toString();
        myResult.type = reader.attributes().value("type").toString();
        myResult.zoom = reader.attributes().value("zoom").toString().toInt();
        myResult.lon = reader.attributes().value("lon").toString().toDouble();
        myResult.lat = reader.attributes().value("lat").toString().toDouble();
        myResult.category = reader.attributes().value("category").toString();
        myResult.info = reader.attributes().value("info").toString();

        reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            }

            if (reader.isStartElement()) {
                if (reader.name() == "named") {
                    readNamedElement(results);
                } else if (reader.name() == "description") {
                    readDescriptionElement(&myResult);
                } else if (reader.name() == "nearestplaces") {
                    readNearestPlacesElement(&myResult);
                } else {
                    skipElement();
                }
            } else {
                reader.readNext();
            }

        }
        results->append(myResult);
    }

    void XmlStreamReader::skipElement() {
        reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            }
            if (reader.isStartElement()) {
                skipElement();
            } else {
                reader.readNext();
            }
        }
    }

    void XmlStreamReader::readDescriptionElement(NameFinderResult *result) {
        result->description = reader.readElementText();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            } else {
                skipElement();
            }
        }
    }

    void XmlStreamReader::readNearestPlacesElement(NameFinderResult *result) {
        reader.readNext();
        while (!reader.atEnd()) {
            if (reader.isEndElement()) {
                reader.readNext();
                break;
            }
            if (reader.isStartElement()) {
                if (reader.name() == "named") {
                    QList<NameFinderResult> nearResults;
                    readNamedElement(&nearResults);
                    result->near = nearResults;
                } else {
                    skipElement();
                }
            } else {
                reader.readNext();
            }
        }
    }

    QList<NameFinderResult> XmlStreamReader::getResults() {
        return myResults;
    }
}
