//
// C++ Implementation: WMSServersList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>

#include "WmsServersList.h"
#include "Preferences/MerkaartorPreferences.h"

WmsServer::WmsServer()
{
    WmsServer(QApplication::translate("MerkaartorPreferences","New Server"), "", "", "", "", "", "");
}

WmsServer::WmsServer(const WmsServer& other)
    : WmsName(other.WmsName), WmsAdress(other.WmsAdress), WmsPath(other.WmsPath), WmsLayers(other.WmsLayers),
        WmsProjections(other.WmsProjections), WmsStyles(other.WmsStyles), WmsImgFormat(other.WmsImgFormat)
        , WmsIsTiled(other.WmsIsTiled)
        , WmsCLayer(other.WmsCLayer)
        , deleted(other.deleted)
{
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat
                     , int IsTiled
                     , WmscLayer CLayer
                     , bool Deleted
                                    )
    : WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles), WmsImgFormat(ImgFormat)
    , WmsIsTiled(IsTiled)
    , WmsCLayer(CLayer)
    , deleted(Deleted)
{
    if (Name == "") {
        WmsName = QApplication::translate("MerkaartorPreferences","New Server");
    }
}

void WmsServer::toXml(QDomElement parent)
{
    QDomElement p = parent.ownerDocument().createElement("WmsServer");
    parent.appendChild(p);
    p.setAttribute("name", WmsName);
    p.setAttribute("address", WmsAdress);
    p.setAttribute("path", WmsPath);
    p.setAttribute("layers", WmsLayers);
    p.setAttribute("projections", WmsProjections);
    p.setAttribute("styles", WmsStyles);
    p.setAttribute("format", WmsImgFormat);
    if (deleted)
        p.setAttribute("deleted", "true");
    if (WmsIsTiled > 0) {
        QDomElement c;
        switch (WmsIsTiled) {
        case 1:
            c = parent.ownerDocument().createElement("WMS-C");
            break;
        case 2:
            c = parent.ownerDocument().createElement("Tiling");
            break;
        }
        p.appendChild(c);

        c.setAttribute("TileWidth", WmsCLayer.TileWidth);
        c.setAttribute("TileHeight", WmsCLayer.TileHeight);

        QDomElement bb = parent.ownerDocument().createElement("BoundingBox");
        c.appendChild(bb);
        bb.setAttribute("minx", WmsCLayer.BoundingBox.left());
        bb.setAttribute("miny", WmsCLayer.BoundingBox.top());
        bb.setAttribute("maxx", WmsCLayer.BoundingBox.right());
        bb.setAttribute("maxy", WmsCLayer.BoundingBox.bottom());

        QDomElement r = parent.ownerDocument().createElement("Resolutions");
        c.appendChild(r);
        QStringList slr;
        foreach(qreal z, WmsCLayer.Resolutions) {
            slr << QString::number(z);
        }
        r.appendChild(parent.ownerDocument().createTextNode(slr.join(" ")));
    }
}

WmsServer WmsServer::fromXml(QDomElement parent)
{
    WmsServer theServer;

    if (parent.tagName() == "WmsServer") {
        theServer.WmsName = parent.attribute("name");
        theServer.WmsAdress = parent.attribute("address");
        theServer.WmsPath = parent.attribute("path");
        theServer.WmsLayers = parent.attribute("layers");
        theServer.WmsProjections = parent.attribute("projections");
        theServer.WmsStyles = parent.attribute("styles");
        theServer.WmsImgFormat = parent.attribute("format");
        theServer.deleted = (parent.attribute("deleted") == "true" ? true : false);

        theServer.WmsIsTiled = 0;
        QDomElement wmscElem = parent.firstChildElement("WMS-C");
        if (wmscElem.isNull()) {
            wmscElem = parent.firstChildElement("Tiling");
            if (!wmscElem.isNull())
                theServer.WmsIsTiled = 2;
        } else
            theServer.WmsIsTiled = 1;

        if (!wmscElem.isNull()) {
            theServer.WmsCLayer.LayerName = theServer.WmsLayers;
            theServer.WmsCLayer.Projection = theServer.WmsProjections;
            theServer.WmsCLayer.Styles = theServer.WmsStyles;

            theServer.WmsCLayer.TileHeight = wmscElem.attribute("TileHeight").toInt();
            theServer.WmsCLayer.TileWidth = wmscElem.attribute("TileWidth").toInt();

            QDomElement bb = wmscElem.firstChildElement("BoundingBox");
            if (!bb.isNull()) {
                qreal minx = bb.attribute("minx").toDouble();
                qreal miny = bb.attribute("miny").toDouble();
                qreal maxx = bb.attribute("maxx").toDouble();
                qreal maxy = bb.attribute("maxy").toDouble();
                theServer.WmsCLayer.BoundingBox = QRectF(QPointF(minx, miny), QPointF(maxx, maxy));
            }

            QDomElement r = wmscElem.firstChildElement("Resolutions");
            if (!r.isNull()) {
                QStringList resL;
                resL = r.firstChild().toText().nodeValue().split(" ", QString::SkipEmptyParts);
                foreach(QString res, resL)
                    theServer.WmsCLayer.Resolutions << res.toDouble();
            }
        }
    }

    return theServer;
}

/** **/

void WmsServersList::add(WmsServersList aWmsServersList)
{
    QMapIterator <QString, WmsServer> it(*(aWmsServersList.getServers()));
    while (it.hasNext()) {
        it.next();

        WmsServer anItem = it.value();
        theServers.insert(anItem.WmsName, anItem);
    }
}

WmsServerList* WmsServersList::getServers()
{
    return &theServers;
}

void WmsServersList::addServer(WmsServer aServer)
{
    theServers.insert(aServer.WmsName, aServer);
}

bool WmsServersList::contains(QString name) const
{
    if (theServers.contains(name))
        return true;
    else {
        WmsServerListIterator it(theServers);
        while (it.hasNext()) {
            it.next();

            if (it.key().contains(name, Qt::CaseInsensitive))
                return true;
        }
    }
    return false;
}

WmsServer WmsServersList::getServer(QString name) const
{
    if (theServers.contains(name))
        return theServers.value(name);
    else {
        WmsServerListIterator it(theServers);
        while (it.hasNext()) {
            it.next();

            if (it.key().contains(name, Qt::CaseInsensitive))
                return it.value();
        }
    }
    return WmsServer();
}

void WmsServersList::toXml(QDomElement parent)
{
    QDomElement rt = parent.ownerDocument().createElement("WmsServers");
    parent.appendChild(rt);
    rt.setAttribute("creator", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));

    WmsServerListIterator it(theServers);
    while (it.hasNext()) {
        it.next();

        WmsServer i = it.value();
        i.toXml(rt);
    }
}

WmsServersList WmsServersList::fromXml(QDomElement parent)
{
    WmsServersList theServersList;

    if (parent.nodeName() == "WmsServers") {
        QDomElement c = parent.firstChildElement();
        while(!c.isNull()) {
            if (c.tagName() == "WmsServer") {
                theServersList.addServer(WmsServer::fromXml(c));
            }

            c = c.nextSiblingElement();
        }
    }

    return theServersList;
}
