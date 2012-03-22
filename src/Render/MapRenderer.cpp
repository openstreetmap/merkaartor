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
#include "MasPaintStyle.h"
#include "ImageMapLayer.h"
#include "LineF.h"

#define TEST_RFLAGS(x) theOptions.options.testFlag(x)
#define TEST_RENDERER_RFLAGS(x) r->theOptions.options.testFlag(x)

void BackgroundStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel) {
        paintsel->drawBackground(R, r->thePainter, r);
    } else if (!R->hasPainter() && !TEST_RENDERER_RFLAGS(RendererOptions::UnstyledHidden))
//    if (/*!globalZoom(r->theProjection) && */!R->hasPainter()) //FIXME Untagged roads level of zoom?
    {
        QPen thePen(QColor(0,0,0),1);

        r->thePainter->setBrush(Qt::NoBrush);
        if (dynamic_cast<ImageMapLayer*>(R->layer()) && M_PREFS->getUseShapefileForBackground()) {
            thePen = QPen(QColor(0xc0,0xc0,0xc0),1);
            if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
                r->thePainter->setBrush(M_PREFS->getBgColor());
            else
                r->thePainter->setBrush(QBrush(M_STYLE->getGlobalPainter().getBackgroundColor()));
        } else {
            if (r->thePixelPerM < M_PREFS->getRegionalZoom())
                thePen = QPen(QColor(0x77,0x77,0x77),1);
        }

        r->thePainter->setPen(thePen);
        r->thePainter->drawPath(r->theTransform.map(R->getPath()));
    }
}

void BackgroundStyleLayer::draw(Relation* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawBackground(R,r->thePainter,r);
}


void BackgroundStyleLayer::draw(Node* N)
{
    if ((N->isReadonly() || !N->isSelectable(r->thePixelPerM, r->theOptions)) && (!N->isPOI() && !N->isWaypoint()))
        return;

    const FeaturePainter* paintsel = N->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawBackground(N,r->thePainter,r);
}

void ForegroundStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawForeground(R,r->thePainter,r);
}

void ForegroundStyleLayer::draw(Relation* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawForeground(R,r->thePainter,r);
}

void ForegroundStyleLayer::draw(Node* N)
{
    if ((N->isReadonly() || !N->isSelectable(r->thePixelPerM, r->theOptions)) && (!N->isPOI() && !N->isWaypoint()))
        return;

    const FeaturePainter* paintsel = N->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawForeground(N,r->thePainter,r);
}

void TouchupStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawTouchup(R,r->thePainter,r);
    else if (!R->hasPainter() && !TEST_RENDERER_RFLAGS(RendererOptions::UnstyledHidden)) {
        if ( r->theOptions.arrowOptions != RendererOptions::ArrowsNever )
        {
            Feature::TrafficDirectionType TT = trafficDirection(R);
            if ( (TT != Feature::UnknownDirection) || (r->theOptions.arrowOptions == RendererOptions::ArrowsAlways) )
            {
                qreal theWidth = r->thePixelPerM*R->widthOf()-4;
                if (theWidth > 8)
                    theWidth = 8;
                qreal DistFromCenter = 2*(theWidth+4);
                if (theWidth > 0)
                {
                    for (int i=1; i<R->size(); ++i)
                    {
                        QPointF FromF(r->toView(R->getNode(i-1)));
                        QPointF ToF(r->toView(R->getNode(i)));
                        if (distance(FromF,ToF) > (DistFromCenter*2+4))
                        {
                            QPointF H(FromF+ToF);
                            H *= 0.5;
                            qreal A = angle(FromF-ToF);
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
    const FeaturePainter* paintsel = Pt->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawTouchup(Pt,r->thePainter,r);
    else if (!Pt->hasPainter() && !TEST_RENDERER_RFLAGS(RendererOptions::UnstyledHidden)) {
        if (! ((Pt->isReadonly() || !Pt->isSelectable(r->thePixelPerM, r->theOptions)) && (!Pt->isPOI() && !Pt->isWaypoint())))
//        if (!Pt->isReadonly() && Pt->isSelectable(r))
        {
            QColor theColor = QColor(0,0,0,128);
            qreal WW = r->NodeWidth;
            if (r->theGlobalPainter.DrawNodes) {
                theColor = r->theGlobalPainter.NodesColor;
            }
            QPointF P(r->toView(Pt));
            if (WW >= 1) {
                if (Pt->layer()->classGroups() & Layer::Special) {
                    QRect R2(P.x()-(WW+4)/2, P.y()-(WW+4)/2, WW+4, WW+4);
                    r->thePainter->fillRect(R2,QColor(255,0,255,192));
                } else if (Pt->isWaypoint()) {
                    QRect R2(P.x()-(WW+4)/2, P.y()-(WW+4)/2, WW+4, WW+4);
                    r->thePainter->fillRect(R2,QColor(255,0,0,192));
                }

                QRect R(P.x()-WW/2, P.y()-WW/2, WW, WW);
                r->thePainter->fillRect(R,theColor);
            }
        }
    }
}

void LabelStyleLayer::draw(Way* R)
{
    const FeaturePainter* paintsel = R->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawLabel(R,r->thePainter,r);
}

void LabelStyleLayer::draw(Relation* /* R */)
{
}

void LabelStyleLayer::draw(Node* Pt)
{
    const FeaturePainter* paintsel = Pt->getPainter(r->thePixelPerM);
    if (paintsel)
        paintsel->drawLabel(Pt,r->thePainter,r);
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
        if (M_PREFS->getUseAntiAlias())
            thePainter->setRenderHint(QPainter::Antialiasing);
        int curLayer = (itm.key()).layer();
        while (itm != theFeatures.constEnd() && (itm.key()).layer() == curLayer)
        {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it)
            {
                qreal alpha = (*it)->getAlpha();
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
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->hasPainter(PixelPerM))
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

QPoint MapRenderer::toView(Node* aPt) const
{
    return theTransform.map(aPt->projected()).toPoint();
}


void MapRenderer::render(
        QPainter* P,
        const QMap<RenderPriority, QSet <Feature*> >& theFeatures,
        const QRectF& pViewport,
        const QRect& screen,
        const qreal pixelPerM,
        const RendererOptions& options
)
{
//    #ifndef NDEBUG
//        QTime Start(QTime::currentTime());
//    #endif

    theViewport = pViewport;
    theScreen = screen;
    thePixelPerM = pixelPerM;

    qreal Aspect = (double)screen.width() / screen.height();
    qreal pAspect = fabs(pViewport.width() / pViewport.height());

    qreal wv, hv;
    if (pAspect > Aspect) {
        wv = fabs(pViewport.width());
        hv = fabs(pViewport.height() * pAspect / Aspect);
    } else {
        wv = fabs(pViewport.width() * Aspect / pAspect);
        hv = fabs(pViewport.height());
    }

    qreal ScaleLon = screen.width() / wv;
    qreal ScaleLat = screen.height() / hv;
    theTransform.reset();
    theTransform.scale(ScaleLon, -ScaleLat);
    theTransform.translate(-pViewport.topLeft().x(), -pViewport.topLeft().y());
//    qDebug() << "render transform: " << theTransform;

    theOptions = options;
    theGlobalPainter = M_STYLE->getGlobalPainter();
    if (theGlobalPainter.DrawNodes) {
        NodeWidth = thePixelPerM*theGlobalPainter.NodesProportional+theGlobalPainter.NodesFixed;
    } else {
        NodeWidth = thePixelPerM * M_PREFS->getNodeSize();
        if (NodeWidth > M_PREFS->getNodeSize())
            NodeWidth = M_PREFS->getNodeSize();
    }

    bool bgLayerVisible = TEST_RFLAGS(RendererOptions::BackgroundVisible);
    bool fgLayerVisible = TEST_RFLAGS(RendererOptions::ForegroundVisible);
    bool tchpLayerVisible = TEST_RFLAGS(RendererOptions::TouchupVisible);
    bool lblLayerVisible = TEST_RFLAGS(RendererOptions::NamesVisible);

    QMap<RenderPriority, QSet<Feature*> >::const_iterator itm;
    QMap<RenderPriority, QSet<Feature*> >::const_iterator itmCur;
    QSet<Feature*>::const_iterator it;

    thePainter = P;
    thePainter->save();
    thePainter->translate(screen.left(), screen.top());

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
                    qreal alpha = (*it)->getAlpha();
                    if ((*it)->isReadonly() && !TEST_RFLAGS(RendererOptions::ForPrinting))
                        alpha /= 2.0;
                    if (alpha != 1.) {
                        P->save();
                        P->setOpacity(alpha);
                    }

                    if (CHECK_WAY(*it)) {
                        Way * R = STATIC_CAST_WAY(*it);
                        for (int i=0; i<R->sizeParents(); ++i)
                            if (!R->getParent(i)->isDeleted() && R->getParent(i)->hasPainter(thePixelPerM))
                                continue;
                        bglayer.draw(R);
                    } else if (CHECK_NODE(*it))
                        bglayer.draw(STATIC_CAST_NODE(*it));
                    else if (CHECK_RELATION(*it))
                        bglayer.draw(STATIC_CAST_RELATION(*it));
                    if (alpha != 1.) {
                        P->restore();
                    }
                }
            }
            ++itm;
        }
        itm = itmCur;
        while (itm != theFeatures.constEnd() && (itm.key()).layer() == curLayer)
        {
            if (fgLayerVisible)
            {
                for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                    qreal alpha = (*it)->getAlpha();
                    if ((*it)->isReadonly() && !TEST_RFLAGS(RendererOptions::ForPrinting))
                        alpha /= 2.0;
                    if (alpha != 1.) {
                        P->save();
                        P->setOpacity(alpha);
                    }

                    if (CHECK_WAY(*it)) {
                        Way * R = STATIC_CAST_WAY(*it);
                        for (int i=0; i<R->sizeParents(); ++i)
                            if (!R->getParent(i)->isDeleted() && R->getParent(i)->hasPainter(thePixelPerM))
                                continue;
                        fglayer.draw(R);
                    } else if (CHECK_NODE(*it))
                        fglayer.draw(STATIC_CAST_NODE(*it));
                    else if (CHECK_RELATION(*it))
                        fglayer.draw(STATIC_CAST_RELATION(*it));
                    if (alpha != 1.) {
                        P->restore();
                    }
                }
            }
            ++itm;
        }
    }
    if (tchpLayerVisible)
    {
        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm) {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                qreal alpha = (*it)->getAlpha();
                if ((*it)->isReadonly() && !TEST_RFLAGS(RendererOptions::ForPrinting))
                    alpha /= 2.0;
                if (alpha != 1.) {
                    P->save();
                    P->setOpacity(alpha);
                }

                if (CHECK_WAY(*it)) {
                    Way * R = STATIC_CAST_WAY(*it);
                    for (int i=0; i<R->sizeParents(); ++i)
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->hasPainter(thePixelPerM))
                            continue;
                    tchuplayer.draw(R);
                } else if (CHECK_NODE(*it))
                    tchuplayer.draw(STATIC_CAST_NODE(*it));
                else if (CHECK_RELATION(*it))
                    tchuplayer.draw(STATIC_CAST_RELATION(*it));
                if (alpha != 1.) {
                    P->restore();
                }
            }
        }
    }

    for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm)
    {
        for (it = itm.value().constBegin() ;it != itm.value().constEnd(); ++it)
        {
            qreal alpha = (*it)->getAlpha();
            if ((*it)->isReadonly() && !TEST_RFLAGS(RendererOptions::ForPrinting))
                alpha /= 2.0;
            if (alpha != 1.)
                P->setOpacity(alpha);

            (*it)->draw(*P, this);
        }
    }
    if (lblLayerVisible)
    {
        for (itm = theFeatures.constBegin() ;itm != theFeatures.constEnd(); ++itm) {
            for (it = itm.value().constBegin(); it != itm.value().constEnd(); ++it) {
                P->save();
                qreal alpha = (*it)->getAlpha();
                if ((*it)->isReadonly() && !TEST_RFLAGS(RendererOptions::ForPrinting))
                    alpha /= 2.0;
                P->setOpacity(alpha);

                if (CHECK_WAY(*it)) {
                    Way * R = STATIC_CAST_WAY(*it);
                    for (int i=0; i<R->sizeParents(); ++i)
                        if (!R->getParent(i)->isDeleted() && R->getParent(i)->hasPainter(thePixelPerM))
                            continue;
                    lbllayer.draw(R);
                } else if (CHECK_NODE(*it))
                    lbllayer.draw(STATIC_CAST_NODE(*it));
                else if (CHECK_RELATION(*it))
                    lbllayer.draw(STATIC_CAST_RELATION(*it));
                P->restore();
            }
        }
    }
    thePainter->restore();

//    #ifndef NDEBUG
//        QTime Stop(QTime::currentTime());
//        qDebug() << Start.msecsTo(Stop) << "ms";
//    #endif
}
#endif

