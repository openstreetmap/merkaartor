/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demos of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BOOKMARKSVIEW_H
#define BOOKMARKSVIEW_H

#include <QtWidgets>
#include <QtXml>

#include "BookmarksModel.h"
#include "MouseMachine/MouseMachine.h"

class QListWidgetItem;
class QUrl;

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    BookmarksView(QWidget *parent = 0);
    ~BookmarksView();

    QTreeView* view() { return m_iconView; }
    BookmarksModel* model() { return &m_model; }

    void fromXml();
    void toXml();

signals:
    void bookmarkSelected(const QString& name, const QString &url);
    void urlSelected(const QUrl &url);

public slots:
    void bookmarkAdd(const QString& name, const QString &url, int seq=0);
    void bookmarkDel(const QString& name);

private slots:
    void select(const QModelIndex &index);
    void activate(const QModelIndex &index);
    void slotDoubleClicked(QPoint p);

private:
    BookmarksModel m_model;
    QTreeView *m_iconView;
    MouseMachine* charm;

    void fromXmlFolder(QDomElement &e, BookmarkItem *fit);
    void toXmlFolder(QDomElement &e, QModelIndex &pindex);
};

#endif // BOOKMARKSVIEW_H
