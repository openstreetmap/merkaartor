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

#include "ZoomStrip.h"
#include "ControlButton.h"

#include <QtCore>
#include <QtGui>

ZoomStrip::ZoomStrip(QWidget *parent)
    : QWidget(parent)
{
    QSize IconSize(48, 48);

    zoomIn = new QAction(QPixmap(":/icons/add"), "Zoom In", this);
    zoomOut = new QAction(QPixmap(":/icons/remove"), "Zoom Out", this);
    zoomBest = new QAction(QPixmap(":/icons/zoomBest"), "Zoom Best", this);
    zoom100 = new QAction(QPixmap(":/icons/zoom100"), "Zoom 100%", this);

    connect(zoomIn, SIGNAL(triggered()), SIGNAL(zoomInClicked()));
    connect(zoomOut, SIGNAL(triggered()), SIGNAL(zoomOutClicked()));
    connect(zoomBest, SIGNAL(triggered()), SIGNAL(zoomBestClicked()));
    connect(zoom100, SIGNAL(triggered()), SIGNAL(zoom100Clicked()));

    ControlButton *zoomInBt = new ControlButton(zoomIn, IconSize, this);
    ControlButton *zoomOutBt = new ControlButton(zoomOut, IconSize, this);
    ControlButton *zoomBestBt = new ControlButton(zoomBest, IconSize, this);
    ControlButton *zoom100Bt = new ControlButton(zoom100, IconSize, this);

    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(zoomInBt);
    lay->addWidget(zoomBestBt);
    lay->addWidget(zoom100Bt);
    lay->addWidget(zoomOutBt);
}

void ZoomStrip::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
