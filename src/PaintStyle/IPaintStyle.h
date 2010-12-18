#ifndef IPAINTSTYLE_H
#define IPAINTSTYLE_H

#include "Painter.h"

class MapView;

#include <QList>

class IPaintStyle
{
public:
    virtual int painterSize() = 0;
    virtual const GlobalPainter& getGlobalPainter() const = 0;
    virtual void setGlobalPainter(GlobalPainter aGlobalPainter) = 0;
    virtual const Painter* getPainter(int i) const = 0;
    virtual QList<Painter> getPainters() const = 0;
    virtual void setPainters(QList<Painter> aPainters) = 0;
    virtual bool isDirty() = 0;

    virtual QString getFilename() = 0;
    virtual void savePainters(const QString& filename) = 0;
    virtual void loadPainters(const QString& filename) = 0;
};

#endif // IPAINTSTYLE_H
