//
// C++ Interface: ImportExportSHP
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportSHP_H
#define ImportExportSHP_H

#include <ImportExport/IImportExport.h>

class Projection;
class Layer;
class OGRGeometry;
class OGRLineString;
class OGRPoint;


/**
    @author cbro <cbro@semperpax.com>
*/
class ImportExportSHP : public IImportExport
{
public:
    ImportExportSHP(Document* doc);

    ~ImportExportSHP();

    // Specify the input as a QFile
    virtual bool loadFile(QString filename);
    // Specify the output as a QFile
    virtual bool saveFile(QString filename);
    // import the  input
    virtual bool import(Layer* aLayer);

    //export
    virtual bool export_(const QList<Feature *>& featList);

protected:
    Projection* theProjection;

    Feature* parseGeometry(Layer* aLayer, OGRGeometry *poGeometry);

    Node *nodeFor(OGRPoint point);
    Way *readWay(Layer* aLayer, OGRLineString *poRing);

private:
    QHash<OGRPoint, Node*> pointHash;
};

#endif
