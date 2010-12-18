#include "MasPaintStyle.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>


#include <math.h>
#include <utility>

//#define GLOBALZOOM		0.002

#define ALWAYS 10e6

/* Zoom boundaries : expressed in Pixel per Meter

   eg 0.01->ALWAYS means show a feature from a zoom level of 0.01 Pixel Per M,
   or 100 Meter per Pixel. For a screen of 1000px wide this is when viewing
   100km or less across.

   eg 0.2->ALWAYS means show a feature from a zoom level 0.2 Px/M or 5M/Px which
   is viewing 5km or less across a screen of 1000Px. */

/* EDITPAINTSTYLE */

MasPaintStyle::MasPaintStyle()
    : m_isDirty(false)
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
    m_isDirty = false;
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
            Painter FP = Painter::fromXML(e, filename);
            Painters.push_back(FP);
        }
        n = n.nextSibling();
    }
    m_isDirty = false;
    m_filename = filename;
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

const Painter* MasPaintStyle::getPainter(int i) const
{
    return &(Painters[i]);
}

QList<Painter> MasPaintStyle::getPainters() const
{
    return Painters;
}

void MasPaintStyle::setPainters(QList<Painter> aPainters)
{
    Painters = aPainters;
    m_isDirty = true;
}

bool MasPaintStyle::isDirty()
{
    return m_isDirty;
}

QString MasPaintStyle::getFilename()
{
    return m_filename;
}


