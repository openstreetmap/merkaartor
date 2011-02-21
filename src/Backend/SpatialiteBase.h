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

#include <QtCore>

/*
these headers are required in order to support
SQLite/SpatiaLite
*/
#include <spatialite/sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>

#ifndef SPATIALITEBASE_H
#define SPATIALITEBASE_H

class SpatialiteBase
{
    friend class SpatialStatement;
public:
    SpatialiteBase();

    sqlite3 * open(const QString &aNom, const int aFlags);
    bool exec(const QString &aSql);
    bool execFile(const QString &aPath);
    qint64 lastRowId();

protected:
    QString m_dbName;
    sqlite3 *m_handle;
};

class SpatialStatement
{
    friend class SpatialiteBase;

public:
    SpatialStatement()
        :theBackend(0), isPrepared(false)
    {
    }
    SpatialStatement(SpatialiteBase* backend, const QString& query)
        : theBackend(backend), theQuery(query), isPrepared(false)
    {
    }

    void bind_double(int idx, qreal val);
    void bind_int(int idx, int val);
    void bind_int64(int idx, qint64 val);
    void bind_string(int idx, const QString& val);
    qreal col_double(int idx);
    int col_int(int idx);
    qint64 col_int64(int idx);
    QString col_string(int idx);

    sqlite3_stmt* statement();
    bool step();
    void reset();

protected:
    SpatialiteBase* theBackend;
    QString theQuery;

    bool isPrepared;
    sqlite3_stmt *pStmt;
};

#endif // SPATIALITEBASE_H
