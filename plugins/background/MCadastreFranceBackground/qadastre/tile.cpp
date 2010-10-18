/*
   This file is part of Qadastre.
   Copyright (C)  2010 Pierre Ducroquet <pinaraf@pinaraf.info>

   Qadastre is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Qadastre is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Qadastre. If not, see <http://www.gnu.org/licenses/>.
*/

#include "tile.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QFile>
#include <QProcess>

Tile::Tile(const QString &fileName, QGraphicsItem * parent) :
    QGraphicsItem(parent), m_filename(fileName), m_blank(false)
{
    qDebug() << QFile(fileName).size();
//    if (QFile(fileName).size() == 2365)
    if (QFile(fileName).size() == 2808)
        m_blank = true;
}

void appendNeighbour(QPoint point, QImage *image, QList<QPoint> *visited) {
    if (!visited->contains(point)) {
        visited->append(point);
        for (int x = qMax(0, point.x() - 1) ; x <= qMin(599, point.x() + 1) ; x++) {
            for (int y = qMax(0, point.y() - 1) ; y <= qMin(599, point.y() + 1) ; y++) {
                QPoint pt(x, y);
                if (visited->contains(pt))
                    continue;
                if (image->pixel(pt) == image->pixel(point)) {
                    appendNeighbour(pt, image, visited);
                }
            }
        }
    }
}

QPixmap Tile::pixmap() {
    if (m_blank)
        return QPixmap();
    if (m_pixCache.isNull())
        m_pixCache = QPixmap(m_filename);
    return m_pixCache;
}

QPoint getNeightbour(const QPoint &point, const QPoint &previous, const QList<QPoint> points) {
    QList<QPoint> neightbours;
    foreach (QPoint pt, points) {
        if ((abs(pt.x() - point.x()) < 2) && (abs(pt.y() - point.y()) < 2))
            neightbours.append(pt);
    }
    int maxDistance = 0;
    QPoint bestPoint(-1, -1);
    foreach (QPoint pt, neightbours) {
        if ((previous - pt).manhattanLength() > maxDistance) {
            maxDistance = (previous - pt).manhattanLength();
            bestPoint = pt;
        }
    }
    return bestPoint;
}

void Tile::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QList<QPoint> points;
    QImage image = this->pixmap().toImage();

    QPoint position = event->pos().toPoint();

    // 248, 198, 50 ==> normal buildings
    // 255, 229, 153 ==> uncovered part of buildings
    qDebug() << image.pixel(position);
    appendNeighbour(position, &image, &points);

    qDebug() << points.count();

    QImage bitmap(image.size(), QImage::Format_Mono);
    bitmap.fill(1);
    foreach(QPoint pt, points) {
        bitmap.setPixel(pt, 0);
        image.setPixel(pt, QColor(Qt::blue).rgb());
    }
    bitmap.save("/tmp/test.ppm");
    QProcess::execute("potrace", QStringList() << "-n" << "-u" << "1" << "-W" << "600" << "-H" << "600" << "-s" << "/tmp/test.ppm");

    m_pixCache = QPixmap::fromImage(image);
    this->update();
}

void Tile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (m_blank) {
        painter->fillRect(0, 0, 600, 600, Qt::white);
    } else {
        painter->drawPixmap(0, 0, pixmap());
    }
}

QRectF Tile::boundingRect() const {
    return QRectF(0, 0, 600, 600);
}
