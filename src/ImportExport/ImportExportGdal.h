//
// C++ Interface: ImportExportGdal
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportGDAL_H
#define ImportExportGDAL_H

#include "IImportExport.h"

#include <ogrsf_frmts.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_version.h>

#if GDAL_VERSION_MAJOR >= 2
#define GDAL2
#endif

class Projection;
class Layer;
class OGRGeometry;
class OGRLineString;
class OGRPoint;
class OGRCoordinateTransformation;


/**
    @author cbro <cbro@semperpax.com>
*/
class ImportExportGdal : public IImportExport
{
public:
    ImportExportGdal(Document* doc);

    ~ImportExportGdal();

    // Specify the input as a QFile
    virtual bool loadFile(QString filename);
    // Specify the output as a QFile
    virtual bool saveFile(QString filename);
    // import the  input
    virtual bool import(Layer* aLayer);
    bool import(Layer *aLayer, const QByteArray &ba, bool confirmProjection);

    //export
    virtual bool export_(const QList<Feature *>& featList);

protected:
    OGRCoordinateTransformation *toWGS84;

    Feature* parseGeometry(Layer* aLayer, OGRGeometry *poGeometry);

    Node *nodeFor(Layer* aLayer, OGRPoint point);
    Way *readWay(Layer* aLayer, OGRLineString *poRing);

#ifndef GDAL2
#define GDALDataset OGRDataSource
#endif
    bool importGDALDataset(GDALDataset *poDs, Layer *aLayer, bool confirmProjection);
#undef GDALDataset

private:
    QHash<OGRPoint, Node*> pointHash;
};

#endif
