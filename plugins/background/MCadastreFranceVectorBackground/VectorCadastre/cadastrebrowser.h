/* This file is part of Qadastre
 * Copyright (C) 2010 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CADASTREBROWSER_H
#define CADASTREBROWSER_H

#include <QWidget>
#include <QDir>
#include <QMenu>
#include <QSignalMapper>
#include <QGraphicsScene>
#include "graphicproducer.h"

namespace Ui {
    class CadastreBrowser;
}

class CadastreBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit CadastreBrowser(QWidget *parent = 0);
    ~CadastreBrowser();

private:
    Ui::CadastreBrowser *ui;
    QDir m_cacheFolder;
    QMenu *m_openMenu;
    QSignalMapper *m_menuMapper;
    QGraphicsScene m_scene;

private slots:
    void on_downloadButton_clicked();
    void fillMenu();
    void openCity(const QString &cityCode);
    void documentParsed(bool success);
    void strikePath(const QPainterPath &path, const GraphicContext &context);
    void fillPath(const QPainterPath &path, const GraphicContext &context, Qt::FillRule fillRule);
};

#endif // CADASTREBROWSER_H
