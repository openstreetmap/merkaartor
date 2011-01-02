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

class Feature;

class SpatialiteBackend : public QObject, public SpatialiteBase
{
    Q_OBJECT

public:
    SpatialiteBackend(QObject* parent=0);
    SpatialiteBackend(const QString& filename, QObject* parent=0);
    virtual ~SpatialiteBackend();

protected:
    void InitializeNew();

    SpatialStatement fSelectFeature;
    SpatialStatement fSelectFeatureBbox;

    SpatialStatement fSelectUser;
    SpatialStatement fInsertUser;
    SpatialStatement fSelectTag;
    SpatialStatement fInsertTag;
//    SpatialStatement fInsertChangeset;
//    SpatialStatement fInsertChangesetTags;
    SpatialStatement fInsertFeature;
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
