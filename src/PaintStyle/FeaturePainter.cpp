#include "MapRenderer.h"
#include "FeaturePainter.h"
#include "Painting.h"
#include "Projection.h"
#include "Features.h"
#include "LineF.h"
#include "SvgCache.h"

#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QMatrix>
#include <QDomElement>
#include <math.h>

#define TEST_RFLAGS(x) theRenderer->theOptions.options.testFlag(x)
#define CAPSTYLE Qt::RoundCap
#define JOINSTYLE Qt::RoundJoin

FeaturePainter::FeaturePainter()
: Painter(), theTagSelector(0){
}

FeaturePainter::FeaturePainter(const FeaturePainter& f)
: Painter(f), theTagSelector(0)
{
    if (f.theTagSelector)
        theTagSelector = f.theTagSelector->copy();

}

FeaturePainter& FeaturePainter::operator=(const FeaturePainter& f)
{
    if (&f == this) return *this;
    delete theTagSelector;
    if (f.theTagSelector)
        theTagSelector = f.theTagSelector->copy();
    else
        theTagSelector = 0;
    ZoomLimitSet = f.ZoomLimitSet;
    ZoomUnder = f.ZoomUnder;
    ZoomUpper = f.ZoomUpper;
    DrawBackground = f.DrawBackground;
    BackgroundColor = f.BackgroundColor;
    BackgroundScale = f.BackgroundScale;
    BackgroundOffset = f.BackgroundOffset;
    DrawForeground = f.DrawForeground;
    ForegroundColor = f.ForegroundColor;
    ForegroundScale = f.ForegroundScale;
    ForegroundOffset = f.ForegroundOffset;
    ForegroundDashSet = f.ForegroundDashSet ;
    ForegroundDash = f.ForegroundDash;
    ForegroundWhite = f.ForegroundWhite;
    DrawTouchup = f.DrawTouchup;
    TouchupColor = f.TouchupColor;
    TouchupScale = f.TouchupScale;
    TouchupOffset = f.TouchupOffset;
    TouchupDashSet = f.TouchupDashSet;
    TouchupDash = f.TouchupDash;
    TouchupWhite = f.TouchupWhite;
    ForegroundFill = f.ForegroundFill;
    ForegroundFillFillColor = f.ForegroundFillFillColor;
    DrawTrafficDirectionMarks = f.DrawTrafficDirectionMarks;
    DrawIcon = f.DrawIcon;
    IconName = f.IconName;
    IconScale = f.IconScale;
    IconOffset = f.IconOffset;
    DrawLabel = f.DrawLabel;
    LabelColor = f.LabelColor;
    LabelScale = f.LabelScale;
    LabelOffset = f.LabelOffset;
    DrawLabelBackground = f.DrawLabelBackground;
    LabelBackgroundColor = f.LabelBackgroundColor;
    LabelFont = f.LabelFont;
    LabelTag = f.LabelTag;
    LabelBackgroundTag = f.LabelBackgroundTag;
    LabelHalo = f.LabelHalo;
    LabelArea = f.LabelArea;
    return *this;
}

FeaturePainter::FeaturePainter(const Painter& f)
: Painter(f), theTagSelector(0)
{
    if (!f.theSelector.isEmpty())
        theTagSelector = TagSelector::parse(f.theSelector);
}

FeaturePainter& FeaturePainter::operator=(const Painter& f)
{
    if (&f == this) return *this;
    delete theTagSelector;
    if (!f.theSelector.isEmpty())
        theTagSelector = TagSelector::parse(f.theSelector);
    else
        theTagSelector = 0;
    ZoomLimitSet = f.ZoomLimitSet;
    ZoomUnder = f.ZoomUnder;
    ZoomUpper = f.ZoomUpper;
    DrawBackground = f.DrawBackground;
    BackgroundColor = f.BackgroundColor;
    BackgroundScale = f.BackgroundScale;
    BackgroundOffset = f.BackgroundOffset;
    DrawForeground = f.DrawForeground;
    ForegroundColor = f.ForegroundColor;
    ForegroundScale = f.ForegroundScale;
    ForegroundOffset = f.ForegroundOffset;
    ForegroundDashSet = f.ForegroundDashSet ;
    ForegroundDash = f.ForegroundDash;
    ForegroundWhite = f.ForegroundWhite;
    DrawTouchup = f.DrawTouchup;
    TouchupColor = f.TouchupColor;
    TouchupScale = f.TouchupScale;
    TouchupOffset = f.TouchupOffset;
    TouchupDashSet = f.TouchupDashSet;
    TouchupDash = f.TouchupDash;
    TouchupWhite = f.TouchupWhite;
    ForegroundFill = f.ForegroundFill;
    ForegroundFillFillColor = f.ForegroundFillFillColor;
    DrawTrafficDirectionMarks = f.DrawTrafficDirectionMarks;
    DrawIcon = f.DrawIcon;
    IconName = f.IconName;
    IconScale = f.IconScale;
    IconOffset = f.IconOffset;
    DrawLabel = f.DrawLabel;
    LabelColor = f.LabelColor;
    LabelScale = f.LabelScale;
    LabelOffset = f.LabelOffset;
    DrawLabelBackground = f.DrawLabelBackground;
    LabelBackgroundColor = f.LabelBackgroundColor;
    LabelFont = f.LabelFont;
    LabelTag = f.LabelTag;
    LabelBackgroundTag = f.LabelBackgroundTag;
    LabelHalo = f.LabelHalo;
    LabelArea = f.LabelArea;
    return *this;
}

FeaturePainter::~FeaturePainter()
{
    delete theTagSelector;
}

void FeaturePainter::setSelector(const QString& anExpression)
{
    delete theTagSelector;
    theTagSelector = TagSelector::parse(anExpression);
    theSelector = anExpression;
}

void FeaturePainter::setSelector(TagSelector* aSel)
{
    delete theTagSelector;
    theTagSelector = aSel;
    theSelector = aSel->asExpression(false);
}

TagSelectorMatchResult FeaturePainter::matchesTag(const Feature* F, MapRenderer* theRender) const
{
    TagSelectorMatchResult res;

    if (!theTagSelector) return TagSelect_NoMatch;
    // Special casing for multipolygon roads
    //if (const Road* R = dynamic_cast<const Road*>(F))
    //{
    //	// TODO create a isPartOfMultiPolygon(R) function for this
    //	for (int i=0; i<R->sizeParents(); ++i)
    //	{
    //		if (const Relation* Parent = dynamic_cast<const Relation*>(R->getParent(i)))
    //			if (Parent->tagValue("type","") == "multipolygon")
    //				return TagSelect_NoMatch;
    //	}
    //}
    if (theRender)
        res = theTagSelector->matches(F,theRender->thePixelPerM);
    else
        res = theTagSelector->matches(F,0);
    if (res)
        return res;
    // Special casing for multipolygon relations
    //if (const Relation* R = dynamic_cast<const Relation*>(F))
    //{
    //	for (int i=0; i<R->size(); ++i)
    //		if ((res = theSelector->matches(R->get(i))))
    //			return res;
    //}
    return TagSelect_NoMatch;
}

/*
void buildPathFromRoad(Road *R, const Projection &theProjection, QPainterPath &Path)
{
    Path.moveTo(theProjection.project(R->get(0)));
    for (int i=1; i<R->size(); ++i)
        Path.lineTo(theProjection.project(R->get(i)));
}

void buildPathFromRelation(Relation *R, const Projection &theProjection, QPainterPath &Path)
{
    for (int i=0; i<R->size(); ++i)
        if (Road* M = dynamic_cast<Road*>(R->get(i)))
            buildPathFromRoad(M, theProjection, Path);
}

*/

void FeaturePainter::drawBackground(Node* N, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawBackground)
        return;

    qreal PixelPerM = theRenderer->thePixelPerM;
    qreal WW = PixelPerM*BackgroundScale+BackgroundOffset;
    if (WW >= 0)
    {
        QPointF P(theRenderer->toView(N));
        QRect R(P.x()-WW/2, P.y()-WW/2, WW, WW);
        thePainter->fillRect(R,BackgroundColor);
    }
}

void FeaturePainter::drawBackground(Way* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawBackground && !ForegroundFill && !ForegroundFillUseIcon) return;

    thePainter->setPen(Qt::NoPen);
    if (M_PREFS->getAreaOpacity() != 100 && ForegroundFill) {
        thePainter->setOpacity(qreal(M_PREFS->getAreaOpacity()) / 100);
    }
    if (DrawBackground)
    {
        qreal WW = theRenderer->thePixelPerM*R->widthOf()*BackgroundScale+BackgroundOffset;
        if (WW >= 0)
        {
            if (BackgroundExterior || BackgroundInterior) {
                QPen thePen(BackgroundColor,WW);
                thePen.setCapStyle(CAPSTYLE);
                thePen.setJoinStyle(Qt::BevelJoin);
                thePainter->setPen(thePen);

                QPainterPath thePath = theRenderer->theTransform.map(R->getPath());
                QPainterPath aPath;

                for (int j=1; j < thePath.elementCount(); j++) {
                    QLineF l(QPointF(thePath.elementAt(j)), QPointF(thePath.elementAt(j-1)));
                    QLineF l1 = l.normalVector();
                    if (BackgroundInterior)
                        l1.setAngle(l1.angle() + 180.);
                    l1.setLength(WW / 2.0);
                    if (j == 1) {
                        QLineF l3(l1);
                        l3.translate(l.p2() - l.p1());
                        aPath.moveTo(l3.p2());
                    }
                    if (j < thePath.elementCount() - 1) {
                        QLineF l4(QPointF(thePath.elementAt(j)), QPointF(thePath.elementAt(j+1)));
                        qreal theAngle = (l4.angle() - l.angle()) / 2.0;
                        if (BackgroundInterior) {
                            if (theAngle > 0.0) theAngle -= 180.0;
                            l1.setLength(-1/sin(angToRad(theAngle))*l1.length());
                        } else {
                            if (theAngle < 0.0) theAngle += 180.0;
                            l1.setLength(1/sin(angToRad(theAngle))*l1.length());
                        }
                        l1.setAngle(l.angle() + theAngle);
                    }
                    aPath.lineTo(l1.p2());
                }
                thePainter->drawPath(aPath);
                thePainter->setPen(Qt::NoPen);
            } else {
                QPen thePen(BackgroundColor,WW);
                thePen.setCapStyle(CAPSTYLE);
                thePen.setJoinStyle(JOINSTYLE);
                ////thePainter->strokePath(R->getPath(),thePen);
                thePainter->setPen(thePen);
            }
        }
    }

    thePainter->setBrush(Qt::NoBrush);
    if (R->size() > 2) {
        if (ForegroundFillUseIcon) {
            if (!IconName.isEmpty()) {
                qreal PixelPerM = theRenderer->thePixelPerM;
                qreal WW = PixelPerM*IconScale+IconOffset;

                QImage* pm = getSVGImageFromFile(IconName,int(WW));
                if (pm && !pm->isNull()) {
                    thePainter->setBrush(*pm);
                }
            }
        } else if (ForegroundFill) {
            thePainter->setBrush(ForegroundFillFillColor);
        }
    }

    thePainter->drawPath(theRenderer->theTransform.map(R->getPath()));
}

void FeaturePainter::drawBackground(Relation* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawBackground && !ForegroundFill && !ForegroundFillUseIcon) return;

    thePainter->setPen(Qt::NoPen);
    if (M_PREFS->getAreaOpacity() != 100 && ForegroundFill) {
        thePainter->setOpacity(qreal(M_PREFS->getAreaOpacity()) / 100);
    }
    if (DrawBackground)
    {
        qreal PixelPerM = theRenderer->thePixelPerM;
        qreal WW = PixelPerM*R->widthOf()*BackgroundScale+BackgroundOffset;
        if (WW >= 0)
        {
            if (BackgroundExterior || BackgroundInterior) {
                QPen thePen(BackgroundColor,WW);
                thePen.setCapStyle(CAPSTYLE);
                thePen.setJoinStyle(Qt::BevelJoin);
                thePainter->setPen(thePen);

                QPainterPath thePath = theRenderer->theTransform.map(R->getPath());
                QPainterPath aPath;

                for (int j=1; j < thePath.elementCount(); j++) {
                    QLineF l(QPointF(thePath.elementAt(j)), QPointF(thePath.elementAt(j-1)));
                    QLineF l1 = l.normalVector();
                    if (BackgroundInterior)
                        l1.setAngle(l1.angle() + 180.);
                    l1.setLength(WW / 2.0);
                    if (j == 1) {
                        QLineF l3(l1);
                        l3.translate(l.p2() - l.p1());
                        aPath.moveTo(l3.p2());
                    }
                    if (j < thePath.elementCount() - 1) {
                        QLineF l4(QPointF(thePath.elementAt(j)), QPointF(thePath.elementAt(j+1)));
                        qreal theAngle = (l4.angle() - l.angle()) / 2.0;
                        if (BackgroundInterior) {
                            if (theAngle > 0.0) theAngle -= 180.0;
                            l1.setLength(-1/sin(angToRad(theAngle))*l1.length());
                        } else {
                            if (theAngle < 0.0) theAngle += 180.0;
                            l1.setLength(1/sin(angToRad(theAngle))*l1.length());
                        }
                        l1.setAngle(l.angle() + theAngle);
                    }
                    aPath.lineTo(l1.p2());
                }
                thePainter->drawPath(aPath);
                thePainter->setPen(Qt::NoPen);
            } else {
                QPen thePen(BackgroundColor,WW);
                thePen.setCapStyle(CAPSTYLE);
                thePen.setJoinStyle(JOINSTYLE);
                ////thePainter->strokePath(R->getPath(),thePen);
                thePainter->setPen(thePen);
            }
        }
    }

    thePainter->setBrush(Qt::NoBrush);
    if (ForegroundFillUseIcon) {
        if (!IconName.isEmpty()) {
            qreal PixelPerM = theRenderer->thePixelPerM;
            qreal WW = PixelPerM*IconScale+IconOffset;

            QImage* pm = getSVGImageFromFile(IconName,int(WW));
            if (pm && !pm->isNull()) {
                thePainter->setBrush(*pm);
            }
        }
    } else if (ForegroundFill) {
        thePainter->setBrush(ForegroundFillFillColor);
    }

    thePainter->drawPath(theRenderer->theTransform.map(R->getPath()));
}

void FeaturePainter::drawForeground(Node* N, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawForeground)
        return;

    qreal PixelPerM = theRenderer->thePixelPerM;
    qreal WW = PixelPerM*ForegroundScale+ForegroundOffset;
    if (WW >= 0)
    {
        QPointF P(theRenderer->toView(N));
        QRect R(P.x()-WW/2, P.y()-WW/2, WW, WW);
        thePainter->fillRect(R,ForegroundColor);
    }
}

void FeaturePainter::drawForeground(Way* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawForeground) return;

    qreal WW = 0.0;
    if (DrawForeground)
    {
        qreal PixelPerM = theRenderer->thePixelPerM;
        WW = PixelPerM*R->widthOf()*ForegroundScale+ForegroundOffset;
        if (WW < 0) return;
        QPen thePen(ForegroundColor,WW);
        thePen.setCapStyle(CAPSTYLE);
        thePen.setJoinStyle(JOINSTYLE);
        if (ForegroundDashSet)
        {
            QVector<qreal> Pattern;
            Pattern << ForegroundDash << ForegroundWhite;
            thePen.setDashPattern(Pattern);
        }
        thePainter->setPen(thePen);
    }
    else
        thePainter->setPen(Qt::NoPen);

    thePainter->setBrush(Qt::NoBrush);

    thePainter->drawPath(theRenderer->theTransform.map(R->getPath()));
}

void FeaturePainter::drawForeground(Relation* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawForeground) return;

    qreal WW = 0.0;
    if (DrawForeground)
    {
        qreal PixelPerM = theRenderer->thePixelPerM;
        WW = PixelPerM*R->widthOf()*ForegroundScale+ForegroundOffset;
        if (WW < 0) return;
        QPen thePen(ForegroundColor,WW);
        thePen.setCapStyle(CAPSTYLE);
        thePen.setJoinStyle(JOINSTYLE);
        if (ForegroundDashSet)
        {
            QVector<qreal> Pattern;
            Pattern << ForegroundDash << ForegroundWhite;
            thePen.setDashPattern(Pattern);
        }
        thePainter->setPen(thePen);
    }
    else
        thePainter->setPen(Qt::NoPen);

    thePainter->setBrush(Qt::NoBrush);

    thePainter->drawPath(theRenderer->theTransform.map(R->getPath()));
}


void FeaturePainter::drawTouchup(Node* Pt, QPainter* thePainter, MapRenderer* theRenderer) const
{
    bool IconOK = false;
    if (DrawIcon)
    {
        if (!IconName.isEmpty()) {
            qreal PixelPerM = theRenderer->thePixelPerM;
            qreal WW = PixelPerM*IconScale+IconOffset;

            QImage* pm = getSVGImageFromFile(IconName,int(WW));
            if (pm && !pm->isNull()) {
                IconOK = true;
                QPointF C(theRenderer->theTransform.map(Pt->projected()));
                // cbro-20090109: Don't draw the dot if there is an icon
                // thePainter->fillRect(QRect(C-QPoint(2,2),QSize(4,4)),QColor(0,0,0,128));
                thePainter->drawImage( int(C.x()-pm->width()/2), int(C.y()-pm->height()/2) , *pm);
            }
        }
        if (!IconOK)
        {
            QColor theColor = QColor(0,0,0,128);
            if (DrawForeground)
                theColor = ForegroundColor;
            else
                if (DrawBackground)
                    theColor = BackgroundColor;

            QPointF P(theRenderer->toView(Pt));
            qreal WW = theRenderer->NodeWidth;
            if (WW >= 1) {
                if (Pt->layer()->classGroups() & Layer::Special) {
                    QRect R2(P.x()-WW*4/3/2, P.y()-WW*4/3/2, WW*4/3, WW*4/3);
                    thePainter->fillRect(R2,QColor(255,0,255,192));
                } else if (Pt->isWaypoint()) {
                    QRect R2(P.x()-WW*4/3/2, P.y()-WW*4/3/2, WW*4/3, WW*4/3);
                    thePainter->fillRect(R2,QColor(255,0,0,192));
                }

                QRect R(P.x()-WW/2, P.y()-WW/2, WW, WW);
                thePainter->fillRect(R,theColor);
            }
        }
    }
}

void FeaturePainter::drawTouchup(Way* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (DrawTouchup)
    {
        qreal PixelPerM = theRenderer->thePixelPerM;
        qreal WW = PixelPerM*R->widthOf()*TouchupScale+TouchupOffset;
        if (WW > 0)
        {
            QPen thePen(TouchupColor,WW);
            thePen.setCapStyle(CAPSTYLE);
            thePen.setJoinStyle(JOINSTYLE);
            if (TouchupDashSet)
            {
                QVector<qreal> Pattern;
                Pattern << TouchupDash << TouchupWhite;
                thePen.setDashPattern(Pattern);
            }
            thePainter->strokePath(theRenderer->theTransform.map(R->getPath()),thePen);
        }
    }
    if (DrawIcon && !ForegroundFillUseIcon)
    {
        if (!IconName.isEmpty()) {
            qreal PixelPerM = theRenderer->thePixelPerM;
            qreal WW = PixelPerM*IconScale+IconOffset;

            QImage* pm = getSVGImageFromFile(IconName,int(WW));
            if (pm && !pm->isNull()) {
                QPointF C(theRenderer->theTransform.map(R->getPath().boundingRect().center()));
                thePainter->drawImage( int(C.x()-pm->width()/2), int(C.y()-pm->height()/2) , *pm);
            }
        }
    }
    if ( ((DrawTrafficDirectionMarks) && (theRenderer->theOptions.arrowOptions == RendererOptions::ArrowsOneway)) ||  theRenderer->theOptions.arrowOptions == RendererOptions::ArrowsAlways)
    {
        Feature::TrafficDirectionType TT = trafficDirection(R);
        if ( (TT != Feature::UnknownDirection) || (theRenderer->theOptions.arrowOptions == RendererOptions::ArrowsAlways) )
        {
            qreal theWidth = theRenderer->thePixelPerM*R->widthOf()-4;
            if (theWidth > 8)
                theWidth = 8;
            qreal DistFromCenter = 2*(theWidth+4);
            if (theWidth > 0)
            {
                if ( theRenderer->theOptions.arrowOptions == RendererOptions::ArrowsAlways )
                    thePainter->setPen(QPen(QColor(255,0,0), 2));
                else
                    thePainter->setPen(QPen(TrafficDirectionMarksColor, 2));


                for (int i=1; i<R->size(); ++i)
                {
                    QPointF FromF(theRenderer->theTransform.map(R->getNode(i-1)->projected()));
                    QPointF ToF(theRenderer->theTransform.map(R->getNode(i)->projected()));
                    if (distance(FromF,ToF) > (DistFromCenter*2+4))
                    {
                        QPoint H(FromF.toPoint()+ToF.toPoint());
                        H *= 0.5;
                        if (!theRenderer->theScreen.contains(H))
                            continue;
                        qreal A = angle(FromF-ToF);
                        QPoint T(qRound(DistFromCenter*cos(A)),qRound(DistFromCenter*sin(A)));
                        QPoint V1(qRound(theWidth*cos(A+M_PI/6)),qRound(theWidth*sin(A+M_PI/6)));
                        QPoint V2(qRound(theWidth*cos(A-M_PI/6)),qRound(theWidth*sin(A-M_PI/6)));
                        if ( (TT == Feature::OtherWay) || (TT == Feature::BothWays) )
                        {
                            thePainter->drawLine(H+T,H+T-V1);
                            thePainter->drawLine(H+T,H+T-V2);
                        }
                        if ( (TT == Feature::OneWay) || (TT == Feature::BothWays) )
                        {
                            thePainter->drawLine(H-T,H-T+V1);
                            thePainter->drawLine(H-T,H-T+V2);
                        }
                        else
                        {
                            if ( theRenderer->theOptions.arrowOptions == RendererOptions::ArrowsAlways )
                            {
                                thePainter->drawLine(H-T,H-T+V1);
                                thePainter->drawLine(H-T,H-T+V2);
                            }
                        }
                    }
                }
            }
        }
    }
}

#define LABEL_PATH_DISTANCE 3
#define LABEL_STRAIGHT_DISTANCE 50
#define BG_SPACING 6
#define BG_PEN_SZ 2

void FeaturePainter::drawPointLabel(QPointF C, QString str, QString strBg, QPainter* thePainter, MapRenderer* theRenderer) const
{
    LineParameters lp = labelBoundary();
    qreal PixelPerM = theRenderer->thePixelPerM;
    qreal WW = PixelPerM*lp.Proportional+lp.Fixed;
    if (WW < 10) return;

    QFont font = getLabelFont();
    font.setPixelSize(int(WW));
    QFontMetrics metrics(font);

    int modX = 0;
    int modY = 0;
    QPainterPath textPath;
    QPainterPath bgPath;

    if (!str.isEmpty()) {
        modX = - (metrics.width(str)/2);
        if (DrawIcon && (IconName != "") )
        {
            QImage pm(IconName);
            modY = - pm.height();
            if (DrawLabelBackground)
                modY -= BG_SPACING;
        }
        textPath.addText(modX, modY, font, str);
        thePainter->translate(C);
    }
    if (DrawLabelBackground && !strBg.isEmpty()) {
        modX = - (metrics.width(strBg)/2);
        if (DrawIcon && (IconName != "") )
        {
            QImage pm(IconName);
            modY = - pm.height();
            if (DrawLabelBackground)
                modY -= BG_SPACING;
        }

        textPath.addText(modX, modY, font, strBg);
        thePainter->translate(C);

        bgPath.addRect(textPath.boundingRect().adjusted(-BG_SPACING, -BG_SPACING, BG_SPACING, BG_SPACING));
        thePainter->setPen(QPen(LabelColor, BG_PEN_SZ));
        thePainter->setBrush(LabelBackgroundColor);
        thePainter->drawPath(bgPath);
    }
    if (getLabelHalo()) {
        thePainter->setPen(QPen(Qt::white, font.pixelSize()/5));
        thePainter->drawPath(textPath);
    }
    thePainter->setPen(Qt::NoPen);
    thePainter->setBrush(LabelColor);
    thePainter->drawPath(textPath);

    if (DrawLabelBackground && !strBg.isEmpty()) {
        QRegion rg = thePainter->clipRegion();
        rg -= textPath.boundingRect().toRect().translated(C.toPoint());
        thePainter->setClipRegion(theRenderer->theTransform.map(rg));
    }
}


void FeaturePainter::drawLabel(Node* Pt, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawLabel)
        return;

    QString str = Pt->tagValue(getLabelTag(), "");
    QString strBg = Pt->tagValue(getLabelBackgroundTag(), "");

    if (str.isEmpty() && strBg.isEmpty())
        return;

    QPointF C(theRenderer->theTransform.map(Pt->projected()));
    drawPointLabel(C, str, strBg, thePainter, theRenderer);
}

void FeaturePainter::drawLabel(Way* R, QPainter* thePainter, MapRenderer* theRenderer) const
{
    if (!DrawLabel)
        return;

    QString str = R->tagValue(getLabelTag(), "");
    QString strBg = R->tagValue(getLabelBackgroundTag(), "");
    if (str.isEmpty() && strBg.isEmpty())
        return;

    if (getLabelArea()) {
        QPointF C(theRenderer->theTransform.map(R->getPath().boundingRect().center()));
//        if (rg.contains(C.toPoint())) {
            drawPointLabel(C, str, strBg, thePainter, theRenderer);
//        }
        return;
    }

    LineParameters lp = labelBoundary();
    qreal PixelPerM = theRenderer->thePixelPerM;
    qreal WW = PixelPerM*R->widthOf()*lp.Proportional+lp.Fixed;
    if (WW < 10 && !TEST_RFLAGS(RendererOptions::PrintAllLabels)) return;
    //qreal WWR = qMax(PixelPerM*R->widthOf()*BackgroundScale+BackgroundOffset, PixelPerM*R->widthOf()*ForegroundScale+ForegroundOffset);

    QPainterPath tranformedRoadPath = theRenderer->theTransform.map(R->getPath());
    QFont font = getLabelFont();
//#if QT_VERSION >= 0x040700 || defined(FORCE_46)
//    qreal pathSurface = tranformedRoadPath.controlPointRect().width() * tranformedRoadPath.controlPointRect().height();
//    if (pathSurface > theRenderer->theScreen.width() * theRenderer->theScreen.height() * 3) {
//        QPainterPath clipPath;
//        clipPath.addRect(theRenderer->theScreen.adjusted(-500, -500, 500, 500));

//        tranformedRoadPath = clipPath.intersected(tranformedRoadPath);
//    }
//#endif

    if (!str.isEmpty()) {
        font.setPixelSize(int(WW));
        QFontMetricsF metrics(font);
        qreal strWidth = metrics.width(str);

        if ((font.pixelSize() >= 5 || TEST_RFLAGS(RendererOptions::PrintAllLabels)) && tranformedRoadPath.length() > strWidth) {
            thePainter->setFont(font);

            int repeat = int((tranformedRoadPath.length() / ((strWidth * LABEL_PATH_DISTANCE))) - 0.5);
            int numSegment = repeat+1;
            qreal lenSegment = tranformedRoadPath.length() / numSegment;
            qreal startSegment = 0;
            QPainterPath textPath;
            do {
                qreal curLen = startSegment + ((lenSegment - strWidth) / 2);
                int modIncrement = 1;
                qreal modAngle = 0;
                qreal modY = 0;
                if (cos(angToRad(tranformedRoadPath.angleAtPercent((startSegment+(lenSegment/2))/tranformedRoadPath.length()))) < 0) {
                    modIncrement = -1;
                    modAngle = 180.0;
                    curLen += strWidth;
                }
                for (int i = 0; i < str.length(); ++i) {
                    qreal t = tranformedRoadPath.percentAtLength(curLen);
                    QPointF pt = tranformedRoadPath.pointAtPercent(t);

//                    if (!theRenderer->theScreen.contains(pt.toPoint()))
//                        continue;

                    qreal angle = tranformedRoadPath.angleAtPercent(t);
//                    modY = (metrics.ascent()/2)-3;
                    modY = (metrics.height()/2)-metrics.descent();

                    QMatrix m;
                    m.translate(pt.x(), pt.y());
                    m.rotate(-angle+modAngle);

                    QPainterPath charPath;
                    charPath.addText(0, modY, font, str.mid(i, 1));
                    charPath = charPath * m;

                    textPath.addPath(charPath);

                    qreal incremenet = metrics.width(str[i]);
                    curLen += (incremenet * modIncrement);
                }
                startSegment += lenSegment;
            } while (--repeat >= 0);

            if (getLabelHalo()) {
                thePainter->setPen(QPen(Qt::white, font.pixelSize()/6));
                thePainter->drawPath(textPath);
            }
            thePainter->setPen(Qt::NoPen);
            thePainter->setBrush(LabelColor);
            thePainter->drawPath(textPath);
        }
    }
    if (DrawLabelBackground && !strBg.isEmpty()) {
        QRegion rg = thePainter->clipRegion();
        font.setPixelSize(int(WW));
        QFontMetrics metrics(font);
        qreal strWidth = metrics.width(strBg);

        int repeat = int((tranformedRoadPath.length() / (strWidth * LABEL_STRAIGHT_DISTANCE)) - 0.5);
        int numSegment = repeat+1;
        qreal lenSegment = tranformedRoadPath.length() / numSegment;
        qreal startSegment = 0;
        do {
            int modX = 0;
            int modY = 0;

            qreal curLen = startSegment + (lenSegment / 2);
            qreal t = tranformedRoadPath.percentAtLength(curLen);
            QPointF pt = tranformedRoadPath.pointAtPercent(t);

            modX = - (strWidth/2);
            //modX = WW;
            modY = (metrics.ascent()/2);

            QPainterPath textPath, bgPath;
            textPath.addText(modX, modY, font, strBg);
            bgPath.addRect(textPath.boundingRect().adjusted(-BG_SPACING, -BG_SPACING, BG_SPACING, BG_SPACING));

            if (rg.contains(bgPath.boundingRect().toRect().translated(pt.toPoint()))) {
                thePainter->translate(pt);

                thePainter->setPen(QPen(LabelColor, BG_PEN_SZ));
                thePainter->setBrush(LabelBackgroundColor);
                thePainter->drawPath(bgPath);

                if (getLabelHalo()) {
                    thePainter->setPen(QPen(Qt::white, font.pixelSize()/5));
                    thePainter->drawPath(textPath);
                }
                thePainter->setPen(Qt::NoPen);
                thePainter->setBrush(LabelColor);
                thePainter->drawPath(textPath);

                rg -= bgPath.boundingRect().toRect().translated(pt.toPoint());
            }

            startSegment += lenSegment;
        } while (--repeat >= 0);

        thePainter->setClipRegion(rg);
    }
}
