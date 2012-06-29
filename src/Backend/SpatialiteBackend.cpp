//*************************************************************** // CLass: SpatialiteBackend
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "Global.h"
#include "SpatialiteBackend.h"

#include <QFile>

#include "Features.h"
#include "MerkaartorPreferences.h"

class SpatialBackendPrivate
{
public:
    QHash<Feature*, qint64> AllocFeatures;
    QList<Feature*> findResult;
};

SpatialiteBackend::SpatialiteBackend()
    : SpatialiteBase(), p(new SpatialBackendPrivate)
{
    /*
    VERY IMPORTANT:
    you must initialize the SpatiaLite extension [and related]
    BEFORE attempting to perform any other SQLite call
    */
    spatialite_init (0);

    /* showing the SQLite version */
    qDebug ("SQLite version: %s", sqlite3_libversion ());
    /* showing the SpatiaLite version */
    qDebug ("SpatiaLite version: %s", spatialite_version ());

    isTemp = true;
    theFilename = HOMEDIR + "/" + QDateTime::currentDateTime().toString("yyyyMMdd-hhmmsszzz") + ".spatialite";
    open(HOMEDIR + "/temDb.spatialite", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    InitializeNew();
}

SpatialiteBackend::SpatialiteBackend(const QString& filename)
    : SpatialiteBase(), p(new SpatialBackendPrivate)
{
    /*
    VERY IMPORTANT:
    you must initialize the SpatiaLite extension [and related]
    BEFORE attempting to perform any other SQLite call
    */
    spatialite_init (0);

    /* showing the SQLite version */
    qDebug ("SQLite version: %s", sqlite3_libversion ());
    /* showing the SpatiaLite version */
    qDebug ("SpatiaLite version: %s", spatialite_version ());

    isTemp = false;
    theFilename = filename;
    open(theFilename, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    InitializeNew();
}

SpatialiteBackend::~SpatialiteBackend()
{
    if (isTemp)
        QFile::remove(theFilename);
}

void SpatialiteBackend::InitializeNew()
{
    execFile(":/Utils/Spatialite/init_spatialite-2.3.sql");

    //
    // Tables pour gérer les features.
    //
    exec(
         "CREATE TABLE IF NOT EXISTS feature ("
         "   type UNSIGNED CHAR,"
         "   id INTEGER,"
         "   version INTEGER,"
         "   timestamp TIMESTAMP,"
         "   user TEXT DEFAULT NULL,"

         "   PRIMARY KEY (type, id)"
         "       );"

         "SELECT AddGeometryColumn('feature', 'GEOMETRY', 4326, 'GEOMETRY', 2);"
         "SELECT CreateSpatialIndex('feature','GEOMETRY');"

         "CREATE TABLE IF NOT EXISTS feature_tags ("
         "   id_feature INTEGER NOT NULL REFERENCES feature,"
         "   id_tag INTEGER NOT NULL REFERENCES tag,"
         "   PRIMARY KEY (id_feature, id_tag))"
                );

    //
    // Table pour stocker les tags (pour tous les éléments).
    //
    exec("CREATE TABLE IF NOT EXISTS tag ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "key TEXT,"
         "value TEXT,"
         "UNIQUE (key, value))");

    //
    // Tables pour gérer les way.
    //
    exec(
         "CREATE TABLE IF NOT EXISTS way_nodes ("
         "   id_way INTEGER REFERENCES feature,"
         "   rang INTEGER(5),"
         "   id_node INTEGER REFERENCES feature,"
         "   PRIMARY KEY (id_way, rang));"
                );

    //
    // Tables pour gérer les relations.
    //
    exec(
         "CREATE TABLE IF NOT EXISTS relation_members ("
         "   id_relation INTEGER NOT NULL REFERENCES feature,"
         "   rang INTEGER(5),"
         "   type INTEGER(1) NOT NULL,"
         "   id_member INTEGER NOT NULL,"
         "   role TEXT NULL,"
         "   PRIMARY KEY (id_relation, rang, type, id_member))");

    fSelectFeature = SpatialStatement(this, "SELECT ROWID from feature where (type = ? AND id = ?)");
    fSelectFeatureBbox = SpatialStatement(this, "SELECT * from feature where ROWID IN "
                                       "(Select rowid from idx_feature_Geometry WHERE xmax > ? and ymax > ? and xmin < ? and ymin < ?);");

    fSelectTag = SpatialStatement(this, "SELECT id FROM tag WHERE (key=? AND value=?)");
    fInsertTag = SpatialStatement(this, "INSERT INTO tag (key, value) VALUES (?,?)");
    fCreateFeature = SpatialStatement(this, "INSERT INTO feature (type) VALUES (?)");
    fUpdateFeature = SpatialStatement(this, "UPDATE feature set version=?,uid=?,actor=?,virtual=?,deleted=?,special=?,uploaded=?,dirtylevel=?,timestamp=?,bbox=?) WHERE ROWID=?");
    fInsertFeatureTags = SpatialStatement(this, "INSERT INTO feature_tags (id_feature, id_tag) VALUES (?,?)");
    fInsertWayNodes = SpatialStatement(this, "INSERT INTO way_nodes (id_way, id_node, rang) VALUES (?,?,?)");
    fInsertRelationMembers = SpatialStatement(this, "INSERT INTO relation_members (id_relation, type, id_member, role, rang) VALUES (?,?,?,?,?)");

    exec("PRAGMA cache_size = 10000");
    exec("PRAGMA synchronous = OFF");
//    exec("PRAGMA journal_mode = OFF");
    exec("PRAGMA temp_store =  MEMORY");
    exec("PRAGMA locking_mode = EXCLUSIVE");
}

void SpatialiteBackend::updateFeature(Feature *F)
{
    qint64 rowid;
    fSelectFeature.bind_int(1, F->id().type);
    fSelectFeature.bind_int64(2, F->id().numId);
    if (fSelectFeature.step()) {
        rowid = fSelectFeature.col_int64(1);
        fUpdateFeature.bind_int(1, F->versionNumber());
    }
}

void SpatialiteBackend::deleteFeature(Feature *F)
{
}

/*******/

Node * SpatialiteBackend::allocNode(const Node& other)
{
    fCreateFeature.bind_int(1, 'N');
    if (fCreateFeature.step()) {
        Node* f = new Node(other);
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

Node * SpatialiteBackend::allocNode(const QPointF& aCoord)
{
    fCreateFeature.bind_int(1, 'N');
    if (fCreateFeature.step()) {
        Node* f = new Node(aCoord);
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

Way * SpatialiteBackend::allocWay()
{
    fCreateFeature.bind_int(1, 'W');
    if (fCreateFeature.step()) {
        Way* f = new Way();
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

Way * SpatialiteBackend::allocWay(const Way& other)
{
    fCreateFeature.bind_int(1, 'W');
    if (fCreateFeature.step()) {
        Way* f = new Way(other);
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

Relation * SpatialiteBackend::allocRelation()
{
    fCreateFeature.bind_int(1, 'R');
    if (fCreateFeature.step()) {
        Relation* f = new Relation();
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

Relation * SpatialiteBackend::allocRelation(const Relation& other)
{
    fCreateFeature.bind_int(1, 'R');
    if (fCreateFeature.step()) {
        Relation* f = new Relation(other);
        f->internal_id = lastRowId();
        p->AllocFeatures[f] = f->internal_id;
        return f;
    }
    return NULL;
}

TrackSegment * SpatialiteBackend::allocSegment()
{
    TrackSegment* f = new TrackSegment();
    p->AllocFeatures[f] = -1;
    return f;
}

void SpatialiteBackend::deallocFeature(Feature *f)
{
}

void SpatialiteBackend::sync(Feature *f)
{
//    CoordBox bb = f->boundingBox();
//    if (!bb.isNull()) {
//        qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
//        qreal max[] = {bb.topRight().x(), bb.topRight().y()};
//        p->theRTree.Insert(min, max, f);
    //    }
}

void SpatialiteBackend::getFeatureSet(QMap<RenderPriority, QSet<Feature *> > &theFeatures, QList<QRectF> &invalidRects, QRectF &clipRect, Projection &theProjection, QTransform &theTransform)
{
}

const QList<Feature *> & SpatialiteBackend::indexFind(const QRectF &vp)
{
    return findResult;
}

void SpatialiteBackend::get(const QRectF &bb, QList<Feature *> &theFeatures)
{
}

