//***************************************************************
// CLass: SpatialiteBackend
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING QString file that comes with this distribution
//
//******************************************************************

#ifndef SPATIALITEBACKEND_H
#define SPATIALITEBACKEND_H

#include <QtCore>

#include "SpatialiteBase.h"
#include "IBackend.h"
#include "Features.h"

class SpatialBackendPrivate;
class SpatialiteBackend : public SpatialiteBase, public IBackend
{
public:
    SpatialiteBackend();
    SpatialiteBackend(const QString& filename);
    virtual ~SpatialiteBackend();

private:
    SpatialBackendPrivate* p;

public:
    virtual Node* allocNode(const Node& other);
    virtual Node* allocNode(const QPointF& aCoord);

    virtual Way* allocWay();
    virtual Way* allocWay(const Way& other);

    virtual Relation* allocRelation();
    virtual Relation* allocRelation(const Relation& other);

    virtual TrackSegment* allocSegment();

    virtual void deallocFeature(Feature* f);

    virtual void sync(Feature* f);

    virtual const QList<Feature*>& indexFind(const QRectF& vp);
    virtual void get(const QRectF& bb, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                       QList<QRectF>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform);

protected:
    void InitializeNew();

    SpatialStatement fSelectFeature;
    SpatialStatement fSelectFeatureBbox;

    SpatialStatement fSelectTag;
    SpatialStatement fInsertTag;
    SpatialStatement fCreateFeature;
    SpatialStatement fUpdateFeature;
    SpatialStatement fInsertFeatureTags;
    SpatialStatement fInsertWayTags;
    SpatialStatement fInsertWayNodes;
    SpatialStatement fInsertRelationTags;
    SpatialStatement fInsertRelationMembers;

public slots:
    void updateFeature(Feature* F);
    void deleteFeature(Feature* F);

protected:
    bool isTemp;
    QString theFilename;
};

#endif // SPATIALITEBACKEND_H
