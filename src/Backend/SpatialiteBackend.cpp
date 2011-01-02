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

SpatialiteBackend::SpatialiteBackend(QObject* parent)
    : QObject(parent)
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

SpatialiteBackend::SpatialiteBackend(const QString& filename, QObject* parent)
    : QObject(parent)
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
    // Table pour stocker les users (pour tous les éléments).
    //
    exec("CREATE TABLE IF NOT EXISTS user ("
         "id INTEGER PRIMARY KEY,"
         "name TEXT)");

    //
    // Table pour stocker les tags (pour tous les éléments).
    //
    exec("CREATE TABLE IF NOT EXISTS tag ("
         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "key TEXT,"
         "value TEXT,"
         "UNIQUE (key, value))");

    //
    // Tables pour gérer les changesets.
    //
//    exec("CREATE TABLE IF NOT EXISTS changeset ("
//         "   id INTEGER PRIMARY KEY,"
//         "   user TEXT DEFAULT NULL,"
//         "   uid INTEGER REFERENCES user,"
//         "   created_at TEXT NOT NULL,"
//         "   num_changes INTEGER NOT NULL,"
//         "   closed_at TEXT DEFAULT NULL,"
//         "   open INTEGER(1) NOT NULL);"

//         "SELECT AddGeometryColumn('changeset','cbbox',4326,'POLYGON',2,1);"

//         "CREATE TABLE IF NOT EXISTS changeset_tags ("
//         "    id_changeset INTEGER REFERENCES changeset,"
//         "    id_tag INTEGER NOT NULL REFERENCES tag,"
//         "    PRIMARY KEY (id_changeset, id_tag))");

    //
    // Tables pour gérer les features.
    //
    exec(
         "CREATE TABLE IF NOT EXISTS feature ("
         "   type UNSIGNED CHAR,"
         "   id INTEGER,"
         "   version INTEGER,"
//         "   changeset INTEGER REFERENCES changeset,"
         "   uid INTEGER REFERENCES user,"
         "   actor INTEGER,"
//         "   visible INTEGER(1) DEFAULT 1 NOT NULL,"
//         "   virtual INTEGER(1) DEFAULT 0 NOT NULL,"
//         "   deleted INTEGER(1) DEFAULT 0 NOT NULL,"
         "   special INTEGER(1) DEFAULT 0 NOT NULL,"
//         "   uploaded INTEGER(1) DEFAULT 0 NOT NULL,"
//         "   dirtylevel INTEGER DEFAULT 0 NOT NULL,"
         "   timestamp TEXT NOT NULL,"

         "   PRIMARY KEY (type, id)"
         "       );"

         "SELECT AddGeometryColumn('feature', 'bbox', 4326, 'POLYGON', 2);"
         "SELECT CreateSpatialIndex('feature','bbox');"

         "CREATE TABLE IF NOT EXISTS feature_tags ("
         "   id_feature INTEGER NOT NULL REFERENCES feature,"
         "   id_tag INTEGER NOT NULL REFERENCES tag,"
         "   PRIMARY KEY (id_feature, id_tag))"
                );

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

    fSelectUser = SpatialStatement(this, "SELECT id, name FROM user WHERE id=?");
    fInsertUser = SpatialStatement(this, "INSERT INTO user (id, name) VALUES (?,?)");
    fSelectTag = SpatialStatement(this, "SELECT id FROM tag WHERE (key=? AND value=?)");
    fInsertTag = SpatialStatement(this, "INSERT INTO tag (key, value) VALUES (?,?)");
//    fInsertChangeset = SpatialStatement(this, "INSERT INTO changeset (id, user, uid, created_at, num_changes, closed_at, open, mbr) VALUES (?,?,?,?,?,?,?,?)");
//    fInsertChangesetTags = SpatialStatement(this, "INSERT INTO changeset_tags (id_changeset, id_tag) VALUES (?,?)");
    fInsertFeature = SpatialStatement(this, "INSERT INTO feature (type,id,version,uid,actor,virtual,deleted,special,uploaded,dirtylevel,timestamp,bbox) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)");
    fUpdateFeature = SpatialStatement(this, "UPDATE feature set version=?,uid=?,actor=?,virtual=?,deleted=?,special=?,uploaded=?,dirtylevel=?,timestamp=?,bbox=?) WHERE ROWID=?");
    fInsertFeatureTags = SpatialStatement(this, "INSERT INTO feature_tags (id_feature, id_tag) VALUES (?,?)");
    fInsertWayNodes = SpatialStatement(this, "INSERT INTO way_nodes (id_way, id_node, rang) VALUES (?,?,?)");
    fInsertRelationMembers = SpatialStatement(this, "INSERT INTO relation_members (id_relation, type, id_member, role, rang) VALUES (?,?,?,?,?)");

    exec("PRAGMA cache_size = 10000");
//    exec("PRAGMA synchronous = OFF");
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
