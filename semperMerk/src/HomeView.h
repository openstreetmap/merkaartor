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

#ifndef HOMEVIEW_H
#define HOMEVIEW_H

#include <QWidget>

class QUrl;

class AddressBar;
class BookmarkBar;
class BookmarksView;

class MouseMachine;

class HomeView : public QWidget
{
    Q_OBJECT

public:
    HomeView(QWidget *parent);
    void setCurrentAdress(const QString & name, const QString& adress);

    AddressBar* addressBar() { return m_addressBar; }

signals:
    void urlActivated(const QUrl &url);
    void addressEntered(const QString &address);
    void back();

private slots:
    void gotoAddress(const QString &address);
    void slotBookmarkSelected(const QString& name, const QString& url);
    void toggleEdit();

public slots:
    void switchToBrowse();

private:
    AddressBar *m_addressBar;
    BookmarkBar *m_bookmarkBar;
    BookmarksView *m_bookmarks;

    MouseMachine* charm;
};

#endif // HOMEVIEW_H
