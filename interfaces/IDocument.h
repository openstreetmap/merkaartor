#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include "Coord.h"

class Layer;

class IDocument
{
public:
    virtual int layerSize() const = 0;
    virtual Layer* getLayer(int i) = 0;
    virtual const Layer* getLayer(int i) const = 0;

    virtual const QList<CoordBox> getDownloadBoxes() const = 0;
    virtual const QList<CoordBox> getDownloadBoxes(Layer* l) const = 0;

};

#endif // IDOCUMENT_H
