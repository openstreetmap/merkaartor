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

#include "AddressBar.h"

#include <QtCore>
#include <QtGui>

class LineEdit: public QLineEdit
{
public:
    LineEdit(QWidget *parent = 0): QLineEdit(parent) {}

    void paintEvent(QPaintEvent *event) {
        QLineEdit::paintEvent(event);
        if (text().isEmpty()) {
            QPainter p(this);
            int flags = Qt::AlignLeft | Qt::AlignVCenter;
            p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
            p.drawText(rect().adjusted(10, 0, 0, 0), flags, "Enter address or search terms");
            p.end();
        }
    }
};

AddressBar::AddressBar(QWidget *parent)
    : QWidget(parent)
{
    QSize btSize(32, 32);

    m_lineEdit = new LineEdit(this);
    QFont fnt = m_lineEdit->font();
    fnt.setPixelSize(20);
    m_lineEdit->setFont(fnt);

    connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(processAddress()));
    m_goButton = new QToolButton(this);
//    m_toolButton->setText("Go");
    m_goButton->setIcon(QPixmap(":/icons/forward"));
    m_goButton->setIconSize(btSize);
    connect(m_goButton, SIGNAL(clicked()), SLOT(processAddress()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(2);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_goButton);

//    setFocusProxy(m_lineEdit);
}

void AddressBar::processAddress()
{
    if (m_lineEdit->text().isEmpty())
        return;

    QString url = m_lineEdit->text();
    if (!url.contains('.') || url.contains(' '))
        url = QString("http://www.google.com/search?q=%1").arg(url);
    QUrl u(url);
    emit addressEntered(QString(u.toEncoded()));
}

void AddressBar::setUrl(const QString& theUrl)
{
    m_lineEdit->setText(theUrl);
}

