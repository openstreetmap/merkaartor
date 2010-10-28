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


#include "pdfgraphicview.h"
#include <QWheelEvent>

PDFGraphicView::PDFGraphicView(QWidget *parent) :
    QGraphicsView(parent)
{
}

void PDFGraphicView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier) {
        QAbstractScrollArea::wheelEvent(new QWheelEvent(event->pos(), event->delta()/2, event->buttons(), event->modifiers(), Qt::Vertical));
        //scrollContentsBy(event->delta(), 0);
    } else if (event->modifiers() & Qt::ControlModifier) {
        QAbstractScrollArea::wheelEvent(new QWheelEvent(event->pos(), event->delta()/2, event->buttons(), event->modifiers(), Qt::Horizontal));
        //scrollContentsBy(0, event->delta());
    } else {
        if (event->delta() > 0)
            scale(1.1, 1.1);
        else
            scale(0.9, 0.9);
    }
}
