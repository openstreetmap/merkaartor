#include "PrimitivePainter.h"
#include "Features/IFeature.h"

#include "Utils/SvgCache.h"

#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QMatrix>
#include <math.h>

#define CAPSTYLE Qt::RoundCap
#define JOINSTYLE Qt::RoundJoin

PrimitivePainter::PrimitivePainter()
: Painter(), theTagSelector(0){
}

PrimitivePainter::PrimitivePainter(const PrimitivePainter& f)
: Painter(f), theTagSelector(0)
{
    if (f.theTagSelector)
        theTagSelector = f.theTagSelector->copy();

}

PrimitivePainter& PrimitivePainter::operator=(const PrimitivePainter& f)
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

PrimitivePainter::PrimitivePainter(const Painter& f)
: Painter(f), theTagSelector(0)
{
    if (!f.theSelector.isEmpty())
        theTagSelector = TagSelector::parse(f.theSelector);
}

PrimitivePainter& PrimitivePainter::operator=(const Painter& f)
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

PrimitivePainter::~PrimitivePainter()
{
    delete theTagSelector;
}

void PrimitivePainter::setSelector(const QString& anExpression)
{
    delete theTagSelector;
    theTagSelector = TagSelector::parse(anExpression);
    theSelector = anExpression;
}

void PrimitivePainter::setSelector(TagSelector* aSel)
{
    delete theTagSelector;
    theTagSelector = aSel;
    theSelector = aSel->asExpression(false);
}

TagSelectorMatchResult PrimitivePainter::matchesTag(const IFeature* F, double PixelPerM) const
{
    TagSelectorMatchResult res;

    if (!theTagSelector) return TagSelect_NoMatch;

    if ((res = theTagSelector->matches(F,PixelPerM)))
        return res;

    return TagSelect_NoMatch;
}

void PrimitivePainter::drawBackground(QPainterPath* R, QPainter* thePainter, double PixelPerM) const
{
    if (!DrawBackground && !ForegroundFill) return;

    thePainter->setPen(Qt::NoPen);
    if (DrawBackground)
    {
        double WW = PixelPerM*BackgroundScale+BackgroundOffset;
        if (WW >= 0)
        {
            QPen thePen(BackgroundColor,WW);
            thePen.setCapStyle(CAPSTYLE);
            thePen.setJoinStyle(JOINSTYLE);
            ////thePainter->strokePath(R->getPath(),thePen);
            thePainter->setPen(thePen);
        }
    }

    if (ForegroundFill && (R->elementCount() > 2))
    {
        thePainter->setBrush(ForegroundFillFillColor);
    }
    else
        thePainter->setBrush(Qt::NoBrush);

    thePainter->drawPath(*R);
}

void PrimitivePainter::drawForeground(QPainterPath* R, QPainter* thePainter, double PixelPerM) const
{
    if (!DrawForeground) return;

    double WW = 0.0;
    if (DrawForeground)
    {
        WW = PixelPerM*ForegroundScale+ForegroundOffset;
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

    thePainter->drawPath(*R);
}

void PrimitivePainter::drawTouchup(QPointF* Pt, QPainter* thePainter, double PixelPerM) const
{
    bool IconOK = false;
    if (DrawIcon)
    {
        if (!IconName.isEmpty()) {
            double WW = PixelPerM*IconScale+IconOffset;

            QPixmap* pm = getPixmapFromFile(IconName,int(WW));
            if (!pm->isNull()) {
                IconOK = true;
                thePainter->drawPixmap( int(Pt->x()-pm->width()/2), int(Pt->y()-pm->height()/2) , *pm);
            }
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

        QRectF R(*Pt-QPointF(2,2),QSize(4,4));
        thePainter->fillRect(R,theColor);
    }
}

void PrimitivePainter::drawTouchup(QPainterPath* R, QPainter* thePainter, double PixelPerM) const
{
    if (DrawTouchup)
    {
        double WW = PixelPerM*TouchupScale+TouchupOffset;
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
            thePainter->strokePath(*R, thePen);
        }
    }
//    if (DrawTrafficDirectionMarks)
//    {
//        Feature::TrafficDirectionType TT = trafficDirection(R);
//        if ( (TT != Feature::UnknownDirection) || (theView->renderOptions().arrowOptions == RendererOptions::ArrowsAlways) )
//        {
//            double theWidth = theView->pixelPerM()*R->widthOf()-4;
//            if (theWidth > 8)
//                theWidth = 8;
//            double DistFromCenter = 2*(theWidth+4);
//            if (theWidth > 0)
//            {
//                if ( theView->renderOptions().arrowOptions == RendererOptions::ArrowsAlways )
//                    thePainter->setPen(QPen(QColor(255,0,0), 2));
//                else
//                    thePainter->setPen(QPen(QColor(0,0,255), 2));


//                for (int i=1; i<R->size(); ++i)
//                {
//                    QPointF FromF(theView->transform().map(theView->projection().project(R->getNode(i-1))));
//                    QPointF ToF(theView->transform().map(theView->projection().project(R->getNode(i))));
//                    if (distance(FromF,ToF) > (DistFromCenter*2+4))
//                    {
//                        QPoint H(FromF.toPoint()+ToF.toPoint());
//                        H *= 0.5;
//                        if (!theView->rect().contains(H))
//                            continue;
//                        double A = angle(FromF-ToF);
//                        QPoint T(qRound(DistFromCenter*cos(A)),qRound(DistFromCenter*sin(A)));
//                        QPoint V1(qRound(theWidth*cos(A+M_PI/6)),qRound(theWidth*sin(A+M_PI/6)));
//                        QPoint V2(qRound(theWidth*cos(A-M_PI/6)),qRound(theWidth*sin(A-M_PI/6)));
//                        if ( (TT == Feature::OtherWay) || (TT == Feature::BothWays) )
//                        {
//                            thePainter->drawLine(H+T,H+T-V1);
//                            thePainter->drawLine(H+T,H+T-V2);
//                        }
//                        if ( (TT == Feature::OneWay) || (TT == Feature::BothWays) )
//                        {
//                            thePainter->drawLine(H-T,H-T+V1);
//                            thePainter->drawLine(H-T,H-T+V2);
//                        }
//                        else
//                        {
//                            if ( theView->renderOptions().arrowOptions == RendererOptions::ArrowsAlways )
//                            {
//                                thePainter->drawLine(H-T,H-T+V1);
//                                thePainter->drawLine(H-T,H-T+V2);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
}

#define LABEL_PATH_DISTANCE 3
#define LABEL_STRAIGHT_DISTANCE 50
#define BG_SPACING 6
#define BG_PEN_SZ 2

void PrimitivePainter::drawPointLabel(QPointF C, QString str, QString strBg, QPainter* thePainter, double PixelPerM) const
{
    LineParameters lp = labelBoundary();
    double WW = PixelPerM*lp.Proportional+lp.Fixed;
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
            QPixmap pm(IconName);
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
            QPixmap pm(IconName);
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
        thePainter->setClipRegion(rg);
    }
}


void PrimitivePainter::drawLabel(QPointF* Pt, QPainter* thePainter, double PixelPerM, QString str, QString strBg) const
{
    if (!DrawLabel)
        return;

    if (str.isEmpty() && strBg.isEmpty())
        return;

    thePainter->save();
    drawPointLabel(*Pt, str, strBg, thePainter, PixelPerM);
    thePainter->restore();
}

void PrimitivePainter::drawLabel(QPainterPath* R, QPainter* thePainter, double PixelPerM, QString str, QString strBg) const
{
    if (!DrawLabel)
        return;

    if (str.isEmpty() && strBg.isEmpty())
        return;

    thePainter->save();
    if (getLabelArea()) {
        QPointF C(R->boundingRect().center());
        drawPointLabel(C, str, strBg, thePainter, PixelPerM);
        thePainter->restore();
        return;
    }

    LineParameters lp = labelBoundary();
    double WW = PixelPerM*lp.Proportional+lp.Fixed;
    if (WW < 10) return;
    //double WWR = qMax(PixelPerM*R->widthOf()*BackgroundScale+BackgroundOffset, PixelPerM*R->widthOf()*ForegroundScale+ForegroundOffset);

    QPainterPath textPath;
    QPainterPath tranformedRoadPath = *R;
    QFont font = getLabelFont();

    if (!str.isEmpty()) {
        QRegion rg = thePainter->clipRegion();
        font.setPixelSize(int(WW));
        QFontMetrics metrics(font);

        if (font.pixelSize() >= 5 && tranformedRoadPath.length() > metrics.width(str)) {
            thePainter->setFont(font);

            int repeat = int((tranformedRoadPath.length() / ((metrics.width(str) * LABEL_PATH_DISTANCE))) - 0.5);
            int numSegment = repeat+1;
            qreal lenSegment = tranformedRoadPath.length() / numSegment;
            qreal startSegment = 0;
            QPainterPath textPath;
            do {
                QRegion rg = thePainter->clipRegion();

                qreal curLen = startSegment + ((lenSegment - metrics.width(str)) / 2);
                int modIncrement = 1;
                qreal modAngle = 0;
                int modY = 0;
                if (cos(angToRad(tranformedRoadPath.angleAtPercent((startSegment+(lenSegment/2))/tranformedRoadPath.length()))) < 0) {
                    modIncrement = -1;
                    modAngle = 180.0;
                    curLen += metrics.width(str);
                }
                for (int i = 0; i < str.length(); ++i) {
                    qreal t = tranformedRoadPath.percentAtLength(curLen);
                    QPointF pt = tranformedRoadPath.pointAtPercent(t);
                    qreal angle = tranformedRoadPath.angleAtPercent(t);
                    modY = (metrics.ascent()/2)-3;

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
            thePainter->setClipRegion(rg);
        }
    }
    if (DrawLabelBackground && !strBg.isEmpty()) {
        QRegion rg = thePainter->clipRegion();
        font.setPixelSize(int(WW));
        QFontMetrics metrics(font);

        int repeat = int((tranformedRoadPath.length() / (metrics.width(strBg) * LABEL_STRAIGHT_DISTANCE)) - 0.5);
        int numSegment = repeat+1;
        qreal lenSegment = tranformedRoadPath.length() / numSegment;
        qreal startSegment = 0;
        do {

            int modX = 0;
            int modY = 0;

            qreal curLen = startSegment + (lenSegment / 2);
            qreal t = tranformedRoadPath.percentAtLength(curLen);
            QPointF pt = tranformedRoadPath.pointAtPercent(t);

            modX = - (metrics.width(strBg)/2);
            //modX = WW;
            modY = (metrics.ascent()/2);

            QPainterPath textPath, bgPath;
            textPath.addText(modX, modY, font, strBg);
            bgPath.addRect(textPath.boundingRect().adjusted(-BG_SPACING, -BG_SPACING, BG_SPACING, BG_SPACING));

            bool rgContains = false;
            for (int i=0; i<rg.rects().size(); i++) {
                if (rg.rects()[i].contains(bgPath.boundingRect().toRect().translated(pt.toPoint()))) {
                    rgContains = true;
                    break;
                }
            }
            if (rgContains) {
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
    thePainter->restore();
}
