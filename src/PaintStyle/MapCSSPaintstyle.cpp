#include "MapView.h"
#include "PaintStyle/MapCSSPaintstyle.h"
#include "Maps/Painting.h"
#include "Maps/Projection.h"
#include "Features.h"
#include "Layer.h"
#include "Utils/TagSelector.h"
#include "Utils/LineF.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>


#include <math.h>
#include <utility>

//#define GLOBALZOOM		0.002

MapCSSPaintstyle* MapCSSPaintstyle::m_MapCSSInstance = 0;

#define ALWAYS 10e6

/* Zoom boundaries : expressed in Pixel per Meter

   eg 0.01->ALWAYS means show a feature from a zoom level of 0.01 Pixel Per M,
   or 100 Meter per Pixel. For a screen of 1000px wide this is when viewing
   100km or less across.

   eg 0.2->ALWAYS means show a feature from a zoom level 0.2 Px/M or 5M/Px which
   is viewing 5km or less across a screen of 1000Px. */

/* EDITPAINTSTYLE */

MapCSSPaintstyle::MapCSSPaintstyle()
{
}

MapCSSPaintstyle::~MapCSSPaintstyle(void)
{
}

void MapCSSPaintstyle::savePainters(const QString& /*filename*/)
{
}

QString parseSelector(QString in)
{
    int idx = 0;
    QString out;

    QList<TagSelector*> terms;
    while (idx < in.length())
        terms.append(TagSelector::parse(in, idx));

    if (terms.length()) {
        out += terms[terms.length()-1]->asExpression(true);
        for (int i=terms.length()-2; i>=0; --i) {
            out += " and parent(";
            out += terms[i]->asExpression(true);
            out += ") ";
        }
    }

    return out;
}

void MapCSSPaintstyle::loadPainters(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QByteArray css = file.readAll();
    QString cssS(css);
    file.close();

    /* Remove comments */
    QRegExp cssComments("/\\*.*\\*/");
    cssComments.setMinimal(true);
    cssS.replace(cssComments, "");

    /* Styles */
    QRegExp cssStyle("\\s*(.*)\\s*\\{(.*)\\}");
    cssStyle.setMinimal(true);

    QRegExp blanks("\\s*");
    QRegExp attSep("\\s*;\\s*");
    QHash <QString, QStringList> styles;
    int pos=0;
    while (cssStyle.indexIn(cssS, pos) != -1) {
        QString selector = parseSelector(cssStyle.capturedTexts().at(1).trimmed());
        QString attributes = cssStyle.capturedTexts().at(2).trimmed();
        styles[selector] = attributes.split(attSep);

        pos += cssStyle.matchedLength();
    }
    qDebug() << styles;


}

int MapCSSPaintstyle::painterSize()
{
    return Painters.size();
}

const GlobalPainter& MapCSSPaintstyle::getGlobalPainter() const
{
    return globalPainter;
}

void MapCSSPaintstyle::setGlobalPainter(GlobalPainter aGlobalPainter)
{
    globalPainter = aGlobalPainter;
}

const FeaturePainter* MapCSSPaintstyle::getPainter(int i) const
{
    return &(Painters[i]);
}

QList<FeaturePainter> MapCSSPaintstyle::getPainters() const
{
    return Painters;
}

void MapCSSPaintstyle::setPainters(QList<FeaturePainter> aPainters)
{
    Painters = aPainters;
}


