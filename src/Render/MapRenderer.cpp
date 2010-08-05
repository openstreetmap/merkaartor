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

#define TEST_RFLAGS(x) theOptions.options.testFlag(x)

void BackgroundStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getEditPainter(r->theView->pixelPerM());
    if (paintsel) {
        paintsel->drawBackground(R,r->thePainter,r->theView);
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
        if ( r->theOptions.arrowOptions != RendererOptions::ArrowsNever )
        {
            Feature::TrafficDirectionType TT = trafficDirection(R);
            if ( (TT != Feature::UnknownDirection) || (r->theOptions.arrowOptions == RendererOptions::ArrowsAlways) )
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
                                if ( r->theOptions.arrowOptions == RendererOptions::ArrowsAlways )
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
        if (!Pt->isReadonly() && Pt->isSelectable(r->theView))
        {
            QPoint P = r->theView->transform().map(r->theView->projection().project(Pt)).toPoint();
            double theWidth = r->theView->nodeWidth();
            if (theWidth >= 1) {
                if (Pt->isWaypoint()) {
                    QRect R2(P-QPoint(theWidth*4/3/2,theWidth*4/3/2),QSize(theWidth*4/3,theWidth*4/3));
                    r->thePainter->fillRect(R2,QColor(255,0,0,128));
                }

                QRect R(P-QPoint(theWidth/2,theWidth/2),QSize(theWidth,theWidth));
                r->thePainter->fillRect(R,QColor(0,0,0,128));
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
    bglayer = BackgroundStyleLayer(this);
    fglayer = ForegroundStyleLayer(this);
    tchuplayer = TouchupStyleLayer(this);
    lbllayer = LabelStyleLayer(this);
}

#if 0
void MapRenderer::render(
        QPainter* P,
        QMap<RenderPriority, QSet <Feature*> > theFeatures,
        const RendererOptions& options,
        MapView* aView
)
{
    #ifndef NDEBUG
        QTime Start(QTime::currentTime());
    #endif

    theView = aView;
    theOptions = options;

    QMap<RenderPriority, QSet<Feature*> >::const_iterator itm;
    QSet<Feature*>::const_iterator it;

    bool bgLayerVisible = TEST_RFLAGS(RendererOptions::BackgroundVisible);
    bool fgLayerVisible = TEST_RFLAGS(RendererOptions::ForegroundVisible);
    bool tchpLayerVisible = TEST_RFLAGS(RendererOptions::TouchupVisible);
    bool lblLayerVisible = TEST_RFLAGS(RendererOptions::NamesVisible);

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
                double alpha = (*it)->layer()->getAlpha();
                if ((*it)->isReadonly())
                    alpha /= 2;
                thePainter->setOpacity(alpha);

                R = NULL;
                Pt = NULL;
                RR = NULL;

                if (!(R = CAST_WAY(*it)))
                    if (!(Pt = CAST_NODE(*it)))
                        RR = CAST_RELATION(*it);

                if (R) {
                    // If there is painter at the relation level, don't paint at the way level
                    bool draw = true;
                    for (int i=0; i<R->sizeParents(); ++i) {
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->getEditPainter(theView->pixelPerM()))
                            draw = false;
                    }
                    if (!draw)
                        continue;
                }

                if (!Pt) {
                    if (bgLayerVisible)
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
                    if (fgLayerVisible)
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
                if (tchpLayerVisible)
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
                if (lblLayerVisible) {
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
#ifndef NDEBUG
    QTime Stop(QTime::currentTime());
    qDebug() << Start.msecsTo(Stop) << "ms";
#endif
    }
}
#else
void MapRenderer::render(
        QPainter* P,
        QMap<RenderPriority, QSet <Feature*> > theFeatures,
        const RendererOptions& options,
        MapView* aView
)
{
//    #ifndef NDEBUG
//        QTime Start(QTime::currentTime());
//    #endif

    theView = aView;
    theOptions = options;

    bool bgLayerVisible = TEST_RFLAGS(RendererOptions::BackgroundVisible);
    bool fgLayerVisible = TEST_RFLAGS(RendererOptions::ForegroundVisible);
    bool tchpLayerVisible = TEST_RFLAGS(RendererOptions::TouchupVisible);
    bool lblLayerVisible = TEST_RFLAGS(RendererOptions::NamesVisible);

    QMap<RenderPriority, QSet<Feature*> >::const_iterator itm;
    QMap<RenderPriority, QSet<Feature*> >::const_iterator itmCur;
    QSet<Feature*>::const_iterator it;

    P->setRenderHint(QPainter::Antialiasing);
    thePainter = P;

    itm = theFeatures.constBegin();
    while (itm != theFeatures.constEnd())
    {
        int curLayer = (itm.key()).layer();
        itmCur = itm;
        while (itm != theFeatures.constEnd() && (itm.key()).layer() == curLayer)
        {
            if (bgLayerVisible)
            {
                for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                    P->save();
                    P->setOpacity((*it)->layer()->getAlpha());
                    if (Way * R = CAST_WAY(*it)) {
                        for (int i=0; i<R->sizeParents(); ++i)
                            if (!R->getParent(i)->isDeleted() && R->getParent(i)->getEditPainter(theView->pixelPerM()))
                                continue;
                        bglayer.draw(R);
                    } else if (Node * Pt = CAST_NODE(*it))
                        bglayer.draw(Pt);
                    else if (Relation * RR = CAST_RELATION(*it))
                        bglayer.draw(RR);
                    P->restore();
                }
            }
            ++itm;
        }
        itm = itmCur;
        while (itm != theFeatures.constEnd() && (itm.key()).layer() == curLayer)
        {
            if (fgLayerVisible)
            {
                ForegroundStyleLayer layer(this);

                for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                    P->save();
                    P->setOpacity((*it)->layer()->getAlpha());
                    if (Way * R = CAST_WAY(*it)) {
                        for (int i=0; i<R->sizeParents(); ++i)
                            if (!R->getParent(i)->isDeleted() && R->getParent(i)->getEditPainter(theView->pixelPerM()))
                                continue;
                        fglayer.draw(R);
                    } else if (Node * Pt = CAST_NODE(*it))
                        fglayer.draw(Pt);
                    else if (Relation * RR = CAST_RELATION(*it))
                        fglayer.draw(RR);
                    P->restore();
                }
            }
            ++itm;
        }
    }
    if (tchpLayerVisible)
    {
        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm) {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->save();
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = CAST_WAY(*it)) {
                    for (int i=0; i<R->sizeParents(); ++i)
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->getEditPainter(theView->pixelPerM()))
                            continue;
                    tchuplayer.draw(R);
                } else if (Node * Pt = CAST_NODE(*it))
                    tchuplayer.draw(Pt);
                else if (Relation * RR = CAST_RELATION(*it))
                    tchuplayer.draw(RR);
                P->restore();
            }
        }
    }
    if (lblLayerVisible)
    {
        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm) {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->save();
                P->setOpacity((*it)->layer()->getAlpha());
                if (Way * R = CAST_WAY(*it)) {
                    for (int i=0; i<R->sizeParents(); ++i)
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->getEditPainter(theView->pixelPerM()))
                            continue;
                    lbllayer.draw(R);
                } else if (Node * Pt = CAST_NODE(*it))
                    lbllayer.draw(Pt);
                else if (Relation * RR = CAST_RELATION(*it))
                    lbllayer.draw(RR);
                P->restore();
            }
        }
    }

    for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
    {
        for (it = itm.value().constBegin() ;it != itm.value().constEnd(); ++it)
        {
            P->setOpacity((*it)->layer()->getAlpha());
            (*it)->draw(*P, aView);
        }
    }
//    #ifndef NDEBUG
//        QTime Stop(QTime::currentTime());
//        qDebug() << Start.msecsTo(Stop) << "ms";
//    #endif
}
#endif

