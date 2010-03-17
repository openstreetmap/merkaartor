//
// C++ Implementation: MapRenderer
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "MapRenderer.h"

#include "Document.h"
#include "Features.h"
#include "MapView.h"
#include "PaintStyle/MasPaintStyle.h"
#include "ImageMapLayer.h"
#include "Utils/LineF.h"

void BackgroundStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel) {
        paintsel->drawBackground(R,r->thePainter,r->theView);
        return;
    }
    for (int i=0; i<R->sizeParents(); ++i) {
        if ((paintsel = R->getParent(i)->getEditPainter(r->theView->pixelPerM())))
            return;
    }
    if (/*!globalZoom(r->theProjection) && */!R->hasEditPainter()) //FIXME Untagged roads level of zoom?
    {
        QPen thePen(QColor(0,0,0),1);

        r->thePainter->setBrush(Qt::NoBrush);
        if (dynamic_cast<ImageMapLayer*>(R->layer()) && M_PREFS->getUseShapefileForBackground()) {
            thePen = QPen(QColor(0xc0,0xc0,0xc0),1);
            if (!R->isCoastline()) {
                if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
                    r->thePainter->setBrush(M_PREFS->getBgColor());
                else
                    r->thePainter->setBrush(QBrush(M_STYLE->getGlobalPainter().getBackgroundColor()));
            }
        } else {
            if (r->theView->pixelPerM() < M_PREFS->getRegionalZoom())
                thePen = QPen(QColor(0x77,0x77,0x77),1);
        }

        r->thePainter->setPen(thePen);
        r->thePainter->drawPath(r->theView->transform().map(R->getPath()));
    }
}

void BackgroundStyleLayer::draw(Relation* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawBackground(R,r->thePainter,r->theView);
}


void BackgroundStyleLayer::draw(Node*)
{
}

void ForegroundStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawForeground(R,r->thePainter,r->theView);
}

void ForegroundStyleLayer::draw(Relation* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawForeground(R,r->thePainter,r->theView);
}

void ForegroundStyleLayer::draw(Node*)
{
}

void TouchupStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawTouchup(R,r->thePainter,r->theView);
    else {
        if ( M_PREFS->getDirectionalArrowsVisible() != DirectionalArrows_Never )
        {
            Feature::TrafficDirectionType TT = trafficDirection(R);
            if ( (TT != Feature::UnknownDirection) || (M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Always) )
            {
                double theWidth = r->theView->pixelPerM()*R->widthOf()-4;
                if (theWidth > 8)
                    theWidth = 8;
                double DistFromCenter = 2*(theWidth+4);
                if (theWidth > 0)
                {
                    for (int i=1; i<R->size(); ++i)
                    {
                        QPointF FromF(r->theView->transform().map(r->theView->projection().project(R->getNode(i-1))));
                        QPointF ToF(r->theView->transform().map(r->theView->projection().project(R->getNode(i))));
                        if (distance(FromF,ToF) > (DistFromCenter*2+4))
                        {
                            QPointF H(FromF+ToF);
                            H *= 0.5;
                            double A = angle(FromF-ToF);
                            QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
                            QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
                            QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));
                            if ( (TT == Feature::OtherWay) || (TT == Feature::BothWays) )
                            {
                                r->thePainter->setPen(QPen(QColor(0,0,255), 2));
                                r->thePainter->drawLine(H+T,H+T-V1);
                                r->thePainter->drawLine(H+T,H+T-V2);
                            }
                            if ( (TT == Feature::OneWay) || (TT == Feature::BothWays) )
                            {
                                r->thePainter->setPen(QPen(QColor(0,0,255), 2));
                                r->thePainter->drawLine(H-T,H-T+V1);
                                r->thePainter->drawLine(H-T,H-T+V2);
                            }
                            else
                            {
                                if ( M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Always )
                                {
                                    r->thePainter->setPen(QPen(QColor(255,0,0), 2));
                                    r->thePainter->drawLine(H-T,H-T+V1);
                                    r->thePainter->drawLine(H-T,H-T+V2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void TouchupStyleLayer::draw(Relation* /* R */)
{
}

void TouchupStyleLayer::draw(Node* Pt)
{
    const FeaturePainter* paintsel = Pt->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawTouchup(Pt,r->thePainter,r->theView);
    else if (!Pt->hasEditPainter()) {
        if (M_PREFS->getTrackPointsVisible() || (Pt->lastUpdated() == Feature::Log && !M_PREFS->getTrackSegmentsVisible())) {
            bool Draw = r->theView->pixelPerM() > M_PREFS->getLocalZoom();
            // Do not draw GPX nodes when simple GPX track appearance is enabled
            if (M_PREFS->getSimpleGpxTrack() && Pt->layer()->isTrack())
                Draw = false;
            if (!Draw) {
                if (!Pt->sizeParents())
                    Draw = true;
                else if (Pt->lastUpdated() == Feature::Log && !M_PREFS->getTrackSegmentsVisible())
                    Draw = true;
            }
            if (Draw)
            {
                QPoint P = r->theView->transform().map(r->theView->projection().project(Pt)).toPoint();

                if (Pt->isVirtual()) {
                    if (M_PREFS->getVirtualNodesVisible()) {
                        r->thePainter->save();
                        r->thePainter->setPen(QColor(0,0,0));
                        r->thePainter->drawLine(P+QPoint(-3, -3), P+QPoint(3, 3));
                        r->thePainter->drawLine(P+QPoint(3, -3), P+QPoint(-3, 3));
                        r->thePainter->restore();
                    }
                } else {
                    if (Pt->isWaypoint()) {
                        QRect R2(P-QPoint(4,4),QSize(8,8));
                        r->thePainter->fillRect(R2,QColor(255,0,0,128));
                    }

                    QRect R(P-QPoint(3,3),QSize(6,6));
                    r->thePainter->fillRect(R,QColor(0,0,0,128));
                }
            }
        }
    }
}

void LabelStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawLabel(R,r->thePainter,r->theView);
}

void LabelStyleLayer::draw(Relation* /* R */)
{
}

void LabelStyleLayer::draw(Node* Pt)
{
    const FeaturePainter* paintsel = Pt->getEditPainter(r->theView->pixelPerM());
    if (paintsel)
        paintsel->drawLabel(Pt,r->thePainter,r->theView);
}

/*** MapRenderer ***/

MapRenderer::MapRenderer()
{
}

void MapRenderer::render(
        QPainter* P,
        QMap<RenderPriority, QSet <Feature*> > theFeatures,
        MapView* aView
)
{
    theView = aView;

    QMap<RenderPriority, QSet<Feature*> >::const_iterator itm;
    QSet<Feature*>::const_iterator it;

#if 0
    P->setRenderHint(QPainter::Antialiasing);
    thePainter = P;

    if (M_PREFS->getBackgroundVisible())
    {
        BackgroundStyleLayer layer(this);
        P->save();

        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = dynamic_cast < Way * >(*it))
                    layer.draw(R);
                else if (Node * Pt = dynamic_cast < Node * >(*it))
                    layer.draw(Pt);
                else if (Relation * RR = dynamic_cast < Relation * >(*it))
                    layer.draw(RR);
            }
        P->restore();
    }
    if (M_PREFS->getForegroundVisible())
    {
        ForegroundStyleLayer layer(this);
        P->save();

        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = dynamic_cast < Way * >(*it))
                    layer.draw(R);
                else if (Node * Pt = dynamic_cast < Node * >(*it))
                    layer.draw(Pt);
                else if (Relation * RR = dynamic_cast < Relation * >(*it))
                    layer.draw(RR);
            }
        P->restore();
    }
    if (M_PREFS->getTouchupVisible())
    {
        TouchupStyleLayer layer(this);
        P->save();

        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = dynamic_cast < Way * >(*it))
                    layer.draw(R);
                else if (Node * Pt = dynamic_cast < Node * >(*it))
                    layer.draw(Pt);
                else if (Relation * RR = dynamic_cast < Relation * >(*it))
                    layer.draw(RR);
            }
        P->restore();
    }
    if (M_PREFS->getNamesVisible()) {
        LabelStyleLayer layer(this);
        P->save();

        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = dynamic_cast < Way * >(*it))
                    layer.draw(R);
                else if (Node * Pt = dynamic_cast < Node * >(*it))
                    layer.draw(Pt);
                else if (Relation * RR = dynamic_cast < Relation * >(*it))
                    layer.draw(RR);
            }
        P->restore();
    }

    for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
    {
        for (it = itm.value().constBegin() ;it != itm.value().constEnd(); ++it)
        {
            P->setOpacity((*it)->layer()->getAlpha());
            (*it)->draw(*P, aView);
        }
    }

#else
    BackgroundStyleLayer bglayer(this);
    ForegroundStyleLayer fglayer(this);
    TouchupStyleLayer tchuplayer(this);
    LabelStyleLayer lbllayer(this);

    Way * R = NULL;
    Node * Pt = NULL;
    Relation * RR = NULL;

    QPixmap pix(theView->size());
    thePainter = new QPainter();

    itm = theFeatures.constBegin();
    while (itm != theFeatures.constEnd())
    {
        pix.fill(Qt::transparent);

        thePainter->begin(&pix);
        thePainter->setRenderHint(QPainter::Antialiasing);
        int curLayer = (itm.key()).layer();
        while (itm != theFeatures.constEnd() && (itm.key()).layer() == curLayer)
        {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it)
            {
                thePainter->setOpacity((*it)->layer()->getAlpha());

                R = NULL;
                Pt = NULL;
                RR = NULL;

                if (!(R = CAST_WAY(*it)))
                    if (!(Pt = CAST_NODE(*it)))
                        RR = CAST_RELATION(*it);

                if (!Pt) {
                    if (M_PREFS->getBackgroundVisible())
                    {
                        thePainter->save();
                        if (R && R->area() == 0)
                            thePainter->setCompositionMode(QPainter::CompositionMode_DestinationOver);

                        if (R)
                            bglayer.draw(R);
                        else if (Pt)
                            bglayer.draw(Pt);
                        else if (RR)
                            bglayer.draw(RR);

                        thePainter->restore();
                    }
                    if (M_PREFS->getForegroundVisible())
                    {
                        thePainter->save();

                        if (R)
                            fglayer.draw(R);
                        else if (Pt)
                            fglayer.draw(Pt);
                        else if (RR)
                            fglayer.draw(RR);

                        thePainter->restore();
                    }
                }
                if (M_PREFS->getTouchupVisible())
                {
                    thePainter->save();

                    if (R)
                        tchuplayer.draw(R);
                    else if (Pt)
                        tchuplayer.draw(Pt);
                    else if (RR)
                        tchuplayer.draw(RR);

                    thePainter->restore();
                }
                if (M_PREFS->getNamesVisible()) {
                    thePainter->save();

                    if (R)
                        lbllayer.draw(R);
                    else if (Pt)
                        lbllayer.draw(Pt);
                    else if (RR)
                        lbllayer.draw(RR);

                    thePainter->restore();
                }

                (*it)->draw(*thePainter, aView);
            }
            ++itm;
        }
        thePainter->end();
        P->drawPixmap(0, 0, pix);
    }
#endif
}


