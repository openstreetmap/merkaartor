//***************************************************************
// CLass: SpatialiteBase
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "SpatialiteBase.h"

#include <QFile>

SpatialiteBase::SpatialiteBase()
{
}

sqlite3* SpatialiteBase::open(const QString& aNom,
                                 const int aFlags)
{
    const int err = sqlite3_open_v2 (aNom.toUtf8().data(), &m_handle, aFlags, NULL);
    if (err != SQLITE_OK) {
        qDebug() << QString(sqlite3_errmsg(m_handle)) + " while opening: " + aNom;
        sqlite3_close (m_handle);
        return NULL;
    }
    return m_handle;
}

bool SpatialiteBase::exec(const QString& aSql)
{
    const int err = sqlite3_exec(m_handle, aSql.toUtf8().data(), 0, 0, 0);
    if (err != SQLITE_OK) {
        qDebug() << QString(sqlite3_errmsg(m_handle)) + " in request: " + aSql;
        return false;
    }
    return true;
}

bool SpatialiteBase::execFile(const QString& aPath)
{
    QFile f(aPath);
    if (!f.open(QIODevice::Text | QIODevice::ReadOnly))
        return false;
    QString s = f.readAll();
    f.close();

    return exec(s);
}

qint64 SpatialiteBase::lastRowId()
{
    return sqlite3_last_insert_rowid(m_handle);
}

/************************************/
void SpatialStatement::bind_double(int idx, double val)
{
    sqlite3_bind_double(statement(), idx, val);
}

void SpatialStatement::bind_int(int idx, int val)
{
    sqlite3_bind_int(statement(), idx, val);
}

void SpatialStatement::bind_int64(int idx, qint64 val)
{
    sqlite3_bind_int64(statement(), idx, val);
}

void SpatialStatement::bind_string(int idx, const QString& val)
{
    sqlite3_bind_text(statement(), idx, val.toUtf8().data(), val.size(), SQLITE_STATIC);
}

double SpatialStatement::col_double(int idx)
{
    return sqlite3_column_double(statement(), idx);
}

int SpatialStatement::col_int(int idx)
{
    return sqlite3_column_int(statement(), idx);
}

qint64 SpatialStatement::col_int64(int idx)
{
    return sqlite3_column_int64(statement(), idx);
}

QString SpatialStatement::col_string(int idx)
{
    return QString ((const char*)sqlite3_column_text(statement(), idx));
}

bool SpatialStatement::step()
{
    return ((sqlite3_step(statement()) == SQLITE_ROW));
}

void SpatialStatement::reset()
{
    sqlite3_reset(statement());
    sqlite3_clear_bindings(statement());
}

sqlite3_stmt* SpatialStatement::statement()
{
    if (isPrepared)
        return pStmt;

    const int ret = sqlite3_prepare_v2(theBackend->m_handle, theQuery.toUtf8().data(), theQuery.size(), &pStmt, NULL);
    if (ret != SQLITE_OK) {
        qDebug() << QString(sqlite3_errmsg(theBackend->m_handle)) + " in prepare statement: " + theQuery;
        return NULL;
    }
    return pStmt;
}
