#include "OsbLayer.h"

#include "ImportExport/ImportExportOsmBin.h"
#include "LayerWidget.h"

// OsbLayer

class OsbLayerPrivate
{
public:
    OsbLayerPrivate()
        : theImp(0), IsWorld(false)
    {
    }
    OsbLayer* theLayer;
    ImportExportOsmBin* theImp;
    QList<qint32> loadedTiles;
    QList<qint32> loadedRegions;

    int rl;
    QMap< qint32, quint64 >::const_iterator ri;
    void loadRegion(Document* d, int rt);
    void clearRegion(Document* d, int rt);
    void handleTile(Document* d, int rt);

    bool IsWorld;
    CoordBox theVP;
};

class OsbFeatureIterator
{
public:
    OsbLayer* theLayer;

    QMap< qint32, OsbRegion* >::const_iterator itR;
    QHash< qint32, OsbTile* >::const_iterator itT;
    QList<Feature_ptr>::const_iterator itF;

    bool isAtEnd;

    Feature* get()
    {
        if (!isAtEnd)
            if (*itF)
                return (*itF).data();
            else
                return NULL;
        else
            return NULL;
    }

    OsbFeatureIterator(OsbLayer* aLayer)
            : theLayer(aLayer), isAtEnd(false)
    {
        itR = theLayer->pp->theImp->theRegionList.constBegin()+1;
        while (true) {
            if (itR != theLayer->pp->theImp->theRegionList.constEnd()) {
                itT = itR.value()->getTileIndex().constBegin();
                while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                    ++itT;
                }
                if (itT == itR.value()->getTileIndex().constEnd()) {
                    ++itR;
                    continue;
                }
                itF = itT.value()->theIndex.constBegin();
                break;
            } else {
                isAtEnd = true;
                break;
            }
        }
    }

    OsbFeatureIterator& operator++()
    {
        if (isAtEnd)
            return *this;

        while (true) {
            ++itF;
            while (itF == itT.value()->theIndex.constEnd())
            {
                ++itT;
                while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                    ++itT;
                }
                if (itT != itR.value()->getTileIndex().constEnd())
                    itF = itT.value()->theIndex.constBegin();
                else {
                    while (true) {
                        ++itR;
                        if (itR != theLayer->pp->theImp->theRegionList.constEnd()) {
                            itT = itR.value()->getTileIndex().constBegin();
                            while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                                ++itT;
                            }
                            if (itT == itR.value()->getTileIndex().constEnd())
                                continue;
                            itF = itT.value()->theIndex.constBegin();
                            break;
                        } else {
                            isAtEnd = true;
                            return *this;
                        }
                    }
                }
            }

            if ((*itF)->isDeleted() || (*itF)->isHidden() || !(*itF)->hasPainter())
                continue;

            break;
        }
        return *this;
    }

    bool isEnd() const
    {
        return isAtEnd;
    }

};

OsbLayer::OsbLayer(const QString & aName)
    : Layer(aName)
{
    Layer::setVisible(true);
    pp = new OsbLayerPrivate();
    pp->theLayer = this;
    pp->theImp = new ImportExportOsmBin(NULL);
}

OsbLayer::OsbLayer(const QString & aName, const QString & filename, bool isWorld)
    : Layer(aName)
{
    Layer::setVisible(true);
    pp = new OsbLayerPrivate();
    pp->theLayer = this;
    pp->IsWorld = isWorld;
    pp->theImp = new ImportExportOsmBin(NULL);
    if (pp->theImp->loadFile(filename))
        pp->theImp->import(this);
}

OsbLayer::~ OsbLayer()
{
    delete pp->theImp;
    delete pp;
}

bool OsbLayer::arePointsDrawable()
{
    return true;
}

void OsbLayer::setFilename(const QString& filename)
{
    delete pp->theImp;

    pp->theImp = new ImportExportOsmBin(NULL);
    if (pp->theImp->loadFile(filename))
        pp->theImp->import(this);
}

LayerWidget* OsbLayer::newWidget(void)
{
    theWidget = new OsbLayerWidget(this);
    return theWidget;
}

void OsbLayerPrivate::loadRegion(Document* d, int rt)
{
    while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
        if (loadedRegions.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
            ++rl;
        }
    }
    while(ri != theImp->theRegionToc.constEnd() && ri.key() < rt) {
        ++ri;
    }
    if (ri != theImp->theRegionToc.constEnd() && ri.key() == rt) {
        if (theImp->loadRegion(rt, d, theLayer)) {
            loadedRegions.insert(rl, rt);
            ++rl;
            ++ri;
        }
    }
}

void OsbLayerPrivate::handleTile(Document* d, int rt)
{
    while (rl < loadedTiles.size() && loadedTiles.at(rl) <= rt) {
        if (loadedTiles.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedTiles.size() && loadedTiles.at(rl) < rt ) {
            if (theImp->clearTile(loadedTiles.at(rl), d, theLayer))
                loadedTiles.removeAt(rl);
            else
                ++rl;
        }
    }
    if (theImp->loadTile(rt, d, theLayer)) {
        loadedTiles.insert(rl, rt);
        ++rl;
    }
}

void OsbLayerPrivate::clearRegion(Document* d, int rt)
{
    while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
        if (loadedRegions.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
            if (theImp->clearRegion(loadedRegions.at(rl), d, theLayer))
                loadedRegions.removeAt(rl);
            else
                ++rl;
        }
    }
}


void OsbLayer::preload()
{
    pp->rl = 0;
    pp->ri = pp->theImp->theRegionToc.constBegin();

    pp->loadRegion(NULL, 0);
}

void OsbLayer::get(const CoordBox& hz, QList<Feature*>& theFeatures)
{
    if (coordToAng(pp->theVP.lonDiff()) > M_PREFS->getRegionTo0Threshold()) {
//		Layer::get(hz, theFeatures);
        return;
    } else {
        OsbFeatureIterator oit(this);
        while (!oit.isEnd()) {
            Feature* F = oit.get();

            if (!theFeatures.contains(F)) {
                if (hz.intersects(F->boundingBox())) {
                    if (Way * R = CAST_WAY(F)) {
                        theFeatures.push_back(R);
                    } else
                    if (CAST_RELATION(F)) {
//						theFeatures.push_back(RR);
                    } else
                    if (Node * pt = CAST_NODE(F)) {
                        if (arePointsDrawable())
                            theFeatures.push_back(pt);
                    } else
                        theFeatures.push_back(F);
                    break;
                }
            }

            ++oit;
        }
    }
}

void OsbLayer::getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, Document* theDocument,
                   QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform)
{
    if (!isVisible())
        return;

    pp->theVP = CoordBox();
    for (int i=0; i < invalidRects.size(); ++i) {
        if (pp->theVP.isNull())
            pp->theVP = invalidRects[i];
        else
            pp->theVP.merge(invalidRects[i]);
    }

    QRectF r(pp->theVP.toRectF());

    int xr1 = int((r.topLeft().x() + COORD_MAX) / REGION_WIDTH);
    int yr1 = int((r.topLeft().y() + COORD_MAX) / REGION_WIDTH);
    int xr2 = int((r.bottomRight().x() + COORD_MAX) / REGION_WIDTH);
    int yr2 = int((r.bottomRight().y() + COORD_MAX) / REGION_WIDTH);

    int xt1 = int((r.topLeft().x() + COORD_MAX) / TILE_WIDTH);
    int yt1 = int((r.topLeft().y() + COORD_MAX) / TILE_WIDTH);
    int xt2 = int((r.bottomRight().x() + COORD_MAX) / TILE_WIDTH);
    int yt2 = int((r.bottomRight().y() + COORD_MAX) / TILE_WIDTH);

    pp->rl = 0;
    pp->ri = pp->theImp->theRegionToc.constBegin();


    pp->loadRegion(theDocument, 0);

    if (coordToAng(pp->theVP.lonDiff()) <= M_PREFS->getRegionTo0Threshold()) {
        for (int j=yr1; j <= yr2; ++j)
            for (int i=xr1; i <= xr2; ++i) {
                pp->loadRegion(theDocument, j*NUM_REGIONS+i);
            }
    }

    pp->rl = 0;
    if (coordToAng(pp->theVP.lonDiff()) <= M_PREFS->getTileToRegionThreshold()) {
        for (int j=yt1; j <= yt2; ++j)
            for (int i=xt1; i <= xt2; ++i)
                pp->handleTile(theDocument, j*NUM_TILES+i);
    }
    while (pp->rl < pp->loadedTiles.size() ) {
        if (pp->theImp->clearTile(pp->loadedTiles.at(pp->rl), theDocument, this))
            pp->loadedTiles.removeAt(pp->rl);
        else
            ++pp->rl;
    }

    pp->rl = 1;
    if (coordToAng(pp->theVP.lonDiff()) <= M_PREFS->getRegionTo0Threshold()) {
        for (int j=yr1; j <= yr2; ++j)
            for (int i=xr1; i <= xr2; ++i) {
                pp->clearRegion(theDocument, j*NUM_REGIONS+i);
            }
    }

    while (pp->rl < pp->loadedRegions.size()) {
        if (pp->theImp->clearRegion(pp->loadedRegions.at(pp->rl), theDocument, this))
            pp->loadedRegions.removeAt(pp->rl);
        else
            ++pp->rl;
    }

    if (coordToAng(pp->theVP.lonDiff()) > M_PREFS->getRegionTo0Threshold()) {
        Layer::getFeatureSet(theFeatures, theDocument,
                   invalidRects, clipRect, theProjection, theTransform);
    } else {
        OsbFeatureIterator oit(this);
        while (!oit.isEnd()) {
            Feature* F = oit.get();
            if (theFeatures[F->renderPriority()].contains(F)) {
                ++oit;
                continue;
            }
            for (int i=0; i < invalidRects.size(); ++i) {
                if (invalidRects[i].intersects(F->boundingBox())) {
                    if (Way * R = CAST_WAY(F)) {
                        R->buildPath(theProjection, theTransform, clipRect);
                        theFeatures[F->renderPriority()].insert(F);
                    } else
                    if (Relation * RR = CAST_RELATION(F)) {
                        RR->buildPath(theProjection, theTransform, clipRect);
                        theFeatures[F->renderPriority()].insert(F);
                    } else
                    if (CAST_NODE(F)) {
                        if (arePointsDrawable())
                            theFeatures[F->renderPriority()].insert(F);
                    } else
                        theFeatures[F->renderPriority()].insert(F);
                    break;
                }
            }
            ++oit;
        }
    }
}

//Feature*  OsbLayer::getFeatureByRef(Document* d, quint64 ref)
//{
//	return pp->theImp->getFeature(d, this, ref);
//}

bool OsbLayer::toXML(QDomElement& xParent, bool asTemplate, QProgressDialog * progress)
{
    Q_UNUSED(progress);

    bool OK = true;

    if (pp->IsWorld)
        return OK;

    QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
    xParent.appendChild(e);
    Layer::toXML(e, asTemplate, progress);
    e.setAttribute("filename", pp->theImp->getFilename());

    return OK;

//	bool OK = true;
//
//	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
//	xParent.appendChild(e);
//
//	e.setAttribute("xml:id", id());
//	e.setAttribute("name", p->Name);
//	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
//	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
//	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
//	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
//	e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));
//
//	QDomElement o = xParent.ownerDocument().createElement("osm");
//	e.appendChild(o);
//	o.setAttribute("version", "0.5");
//	o.setAttribute("generator", "Merkaartor");
//
//	if (p->Features.size()) {
//		QDomElement bb = xParent.ownerDocument().createElement("bound");
//		o.appendChild(bb);
//		CoordBox layBB = boundingBox();
//		QString S = QString().number(coordToAng(layBB.bottomLeft().lat()),'f',6) + ",";
//		S += QString().number(coordToAng(layBB.bottomLeft().lon()),'f',6) + ",";
//		S += QString().number(coordToAng(layBB.topRight().lat()),'f',6) + ",";
//		S += QString().number(coordToAng(layBB.topRight().lon()),'f',6);
//		bb.setAttribute("box", S);
//		bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
//	}
//
//	QList<MapFeaturePtr>::iterator it;
//	for(it = p->Features.begin(); it != p->Features.end(); it++)
//		(*it)->toXML(o, progress);
//
//	return OK;

}

OsbLayer * OsbLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog * progress)
{
    Q_UNUSED(progress);

    OsbLayer* l = new OsbLayer(e.attribute("name"));

    l->setId(e.attribute("xml:id"));
    l->setAlpha(e.attribute("alpha").toDouble());
    l->setVisible((e.attribute("visible") == "true" ? true : false));
    l->setSelected((e.attribute("selected") == "true" ? true : false));
    l->setEnabled((e.attribute("enabled") == "false" ? false : true));
    l->setReadonly((e.attribute("readonly") == "true" ? true : false));

    if (l->pp->theImp->loadFile(e.attribute("filename")))
        l->pp->theImp->import(l);
    else {
        delete l;
        return NULL;
    }

    d->add(l);
    return l;
}

QString OsbLayer::toHtml()
{
    QString S;

    S += "<i>"+QApplication::translate("OsbLayer", "# of loaded Regions")+": </i>" + QApplication::translate("OsbLayer", "%1").arg(QLocale().toString(pp->loadedRegions.size()))+"<br/>";
    S += "<i>"+QApplication::translate("OsbLayer", "# of loaded Tiles")+": </i>" + QApplication::translate("OsbLayer", "%1").arg(QLocale().toString(pp->loadedTiles.size()))+"<br/>";

    return toMainHtml().arg(S);
}

