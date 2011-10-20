//
// C++ Interface: Global
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MERKAARTOR_GLOBAL_H
#define MERKAARTOR_GLOBAL_H

#include <QList>
#include <QPair>

#include "MemoryBackend.h"

class MainWindow;
class IBackend;

extern bool g_Merk_Portable;
extern bool g_Merk_Frisius;
extern bool g_Merk_NoGuardedTagsImport;
extern bool g_Merk_Segment_Mode;
extern bool g_Merk_Ignore_Preferences;
extern bool g_Merk_Reset_Preferences;
extern bool g_Merk_IgnoreStartupTemplate;
extern bool g_Merk_SelfClip;

extern MainWindow* g_Merk_MainWindow;

extern QPair<quint32, quint32> g_addToTagList(QString k, QString v);
extern void g_removeFromTagList(quint32 k, quint32 v);
extern QList<QString> g_getTagKeys();
extern QList<QString> g_getTagValues();
extern const QString& g_getTagKey(int idx);
extern quint32 g_getTagKeyIndex(const QString& s);
extern QStringList g_getTagKeyList();
extern QString g_getTagValue(int idx);
extern quint32 g_getTagValueIndex(const QString& s);
extern QStringList g_getTagValueList(QString k) ;

extern quint32 g_setUser(const QString& u);
extern const QString& g_getUser(quint32 idx);

extern MemoryBackend g_backend;

#endif

