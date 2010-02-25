#include "MapView.h"
#include "PaintStyle/MasPaintStyle.h"
#include "Maps/Painting.h"
#include "Maps/Projection.h"
#include "Features.h"
#include "Layer.h"
#include "PaintStyle/TagSelector.h"
#include "Utils/LineF.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>


#include <math.h>
#include <utility>

//#define LOCALZOOM		0.05
//#define REGIONALZOOM	0.01
//#define GLOBALZOOM		0.002

MasPaintStyle* MasPaintStyle::m_EPSInstance = 0;

#define ALWAYS 10e6

/* Zoom boundaries : expressed in Pixel per Meter

   eg 0.01->ALWAYS means show a feature from a zoom level of 0.01 Pixel Per M,
   or 100 Meter per Pixel. For a screen of 1000px wide this is when viewing
   100km or less across.

   eg 0.2->ALWAYS means show a feature from a zoom level 0.2 Px/M or 5M/Px which
   is viewing 5km or less across a screen of 1000Px. */

/* EDITPAINTSTYLE */

MasPaintStyle::MasPaintStyle()
{
}

MasPaintStyle::~MasPaintStyle(void)
{
}

void MasPaintStyle::savePainters(const QString& filename)
{
    QFile data(filename);
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        out << "<mapStyle>\n";
        out << globalPainter.toXML();
        for (int i=0; i<Painters.size(); ++i)
        {
            QString s = Painters[i].toXML(filename);
            out << s;
        }
        out << "</mapStyle>\n";
    }
}

void MasPaintStyle::loadPainters(const QString& filename)
{
    QDomDocument doc;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();
    GlobalPainter gp;
    globalPainter = gp;
    Painters.clear();
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull() && e.tagName() == "global")
        {
            globalPainter = GlobalPainter::fromXML(e);
        } else
        if(!e.isNull() && e.tagName() == "painter")
        {
            FeaturePainter FP = FeaturePainter::fromXML(e, filename);
            Painters.push_back(FP);
        }
        n = n.nextSibling();
    }
}

int MasPaintStyle::painterSize()
{
    return Painters.size();
}

const GlobalPainter& MasPaintStyle::getGlobalPainter() const
{
    return globalPainter;
}

void MasPaintStyle::setGlobalPainter(GlobalPainter aGlobalPainter)
{
    globalPainter = aGlobalPainter;
}

const FeaturePainter* MasPaintStyle::getPainter(int i) const
{
    return &(Painters[i]);
}

QList<FeaturePainter> MasPaintStyle::getPainters() const
{
    return Painters;
}

void MasPaintStyle::setPainters(QList<FeaturePainter> aPainters)
{
    Painters = aPainters;
}


