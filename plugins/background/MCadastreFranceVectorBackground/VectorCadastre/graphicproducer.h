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


#ifndef GRAPHICPRODUCER_H
#define GRAPHICPRODUCER_H

#include <QObject>
#include <QPainterPath>
#include <QPen>
#include <QBrush>

struct GraphicContext {
    QBrush brush;
    QPen pen;
    QPainterPath clipPath;
};

class GraphicProducer : public QObject
{
    Q_OBJECT
public:
    explicit GraphicProducer(QObject *parent = 0);

public slots:
    bool parseStream(const char *stream);
    bool parsePDF(const QString &fileName);

signals:
    void strikePath(const QPainterPath &path, const GraphicContext &context);
    void fillPath(const QPainterPath &path, const GraphicContext &context, Qt::FillRule fillRule);
    void parsingDone(bool result);

};

#endif // GRAPHICPRODUCER_H
