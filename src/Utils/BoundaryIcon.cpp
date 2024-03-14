#ifndef BOUNDARYICON_H
#define BOUNDARYICON_H

#include "BoundaryIcon.h"

#include <QPainter>

void makeBoundaryIcon(QToolButton* bt, QColor C)
{
    QPixmap pm(36, 18);
    pm.fill(QColor(255, 255, 255));
    QPainter p(&pm);
    p.setPen(C);
    p.setBrush(C);
    p.drawRect(0, 6, 36, 6);
    bt->setIcon(pm);
}

#endif // BOUNDARYICON_H
