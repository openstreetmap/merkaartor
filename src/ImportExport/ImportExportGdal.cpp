//
// C++ Implementation: ImportExportGdal
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "ImportExportGdal.h"
#include "Projection.h"
#include "Features.h"
#include "ProjectionChooser.h"
#include "Global.h"

#include <QtGui>
#include <QApplication>
#include <gdal.h>
#include <cpl_vsi.h>

#include <QDebug>
#include <QLoggingCategory>

QLoggingCategory lc_importExportGdal("merk.ImportExport.GDAL");

bool parseContainer(QDomElement& e, Layer* aLayer);

ImportExportGdal::ImportExportGdal(Document* doc)
 : IImportExport(doc)
{
}


ImportExportGdal::~ImportExportGdal()
{
}

// Specify the input as a QFile
bool ImportExportGdal::loadFile(QString filename)
{
    FileName = filename;

    return true;
}

bool ImportExportGdal::saveFile(QString)
{
    return false;
}


// export
bool ImportExportGdal::export_(const QList<Feature *>& featList)
{
    const char *pszDriverName = "SQLite";
    QString fileName(HOMEDIR + "/test.sqlite");
#ifdef GDAL2
    GDALDriver *poDriver;
    GDALDriverManager *driverManager = GetGDALDriverManager();
    GDALAllRegister();
    driverManager->AutoLoadDrivers();

    poDriver = driverManager->GetDriverByName(pszDriverName);
    if( poDriver == NULL )
    {
        qDebug(lc_importExportGdal) << pszDriverName <<  "driver not available.";
        return false;
    }

    /* Create create options */
    char **createOptions = NULL;
    createOptions = CSLSetNameValue( createOptions, "SPATIALITE", "YES" );

    /* Open new dataset */
    GDALDataset *poDS;

    QFile::remove(fileName);
    poDS = poDriver->Create( fileName.toUtf8().constData(), 0, 0, 0, GDT_Unknown, createOptions );
    if( poDS == NULL )
    {
        qDebug(lc_importExportGdal) << "Creation of output file failed.";
        return false;
    }
    poDS->ExecuteSQL("PRAGMA synchronous = OFF", NULL, NULL);

    /* Create Spatial reference object */
    OGRSpatialReference *poSRS;
    poSRS = new OGRSpatialReference();
    poSRS->importFromEPSG(4326);

    /* Create layer options */
    char **layerOptions = NULL;
    layerOptions = CSLSetNameValue( layerOptions, "FORMAT", "SPATIALITE" );
    layerOptions = CSLSetNameValue( layerOptions, "SPATIAL_INDEX", "YES" );

    /* Create layer */
    OGRLayer *poLayer;
    poLayer = poDS->CreateLayer( "osm", poSRS, wkbUnknown, layerOptions);

    /* Free the options */
    CSLDestroy( createOptions );
    CSLDestroy( layerOptions );
#else
    /* This is legacy code, and is not tested at all */
    OGRSFDriver *poDriver;

    OGRRegisterAll();

    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);
    if( poDriver == NULL )
    {
        qDebug( "%s driver not available.", pszDriverName );
        return false;
    }

    OGRDataSource *poDS;

    QFile::remove(fileName);
    poDS = poDriver->CreateDataSource( fileName.toUtf8().constData(), NULL );
    if( poDS == NULL )
    {
        qDebug( "Creation of output file failed." );
        return false;
    }
    poDS->ExecuteSQL("PRAGMA synchronous = OFF", NULL, NULL);

    OGRSpatialReference *poSRS;
    poSRS = new OGRSpatialReference();
    poSRS->importFromEPSG(4326);

    char **papszOptions = NULL;
    papszOptions = CSLSetNameValue( papszOptions, "SPATIALITE", "YES" );
    papszOptions = CSLSetNameValue( papszOptions, "FORMAT", "SPATIALITE" );
    papszOptions = CSLSetNameValue( papszOptions, "SPATIAL_INDEX", "YES" );

    OGRLayer *poLayer;
    poLayer = poDS->CreateLayer( "osm", poSRS, wkbUnknown, papszOptions);
    CSLDestroy( papszOptions );
#endif


    if( poLayer == NULL )
    {
        qDebug( "Layer creation failed." );
        return false;
    }

    OGRFieldDefn oField("osm_id", OFTReal);
    if( poLayer->CreateField( &oField ) != OGRERR_NONE )
    {
        qDebug( "Creating field failed." );
        return false;
    }
    oField.Set("osm_version", OFTInteger );
    poLayer->CreateField( &oField );
    oField.Set("osm_timestamp", OFTInteger );
    poLayer->CreateField( &oField );

    OGRFeature *poFeature;
    foreach (Feature* F, featList) {
        poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
        poFeature->SetField( "osm_id", (qreal)(F->id().numId));
#ifndef FRISIUS_BUILD
        poFeature->SetField( "osm_version", F->versionNumber());
        poFeature->SetField( "osm_timestamp", (int)F->time().toSecsSinceEpoch());
#endif

        if (CHECK_NODE(F)) {
            Node* N = STATIC_CAST_NODE(F);

            OGRPoint pt;
            pt.setX(N->position().x());
            pt.setY(N->position().y());

            poFeature->SetGeometry( &pt );
        } else if (CHECK_WAY(F)) {
            Way* W = STATIC_CAST_WAY(F);

            OGRLineString ls;
            ls.setNumPoints(W->size());
            for (int i=0; i<W->size(); ++i) {
                ls.setPoint(i, W->getNode(i)->position().x(), W->getNode(i)->position().y(), 0);
            }
            poFeature->SetGeometry( &ls );
        }

        if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
        {
           qDebug( "Failed to create feature in output." );
           return false;
        }
        OGRFeature::DestroyFeature( poFeature );
    }
#ifdef GDAL2
    GDALClose( (GDALDatasetH) poDS );
#else
    OGRDataSource::DestroyDataSource( poDS );
#endif
    return true;
}

/***************/

// Make OGRPoint usable in a QHash<>
static bool operator==(const OGRPoint a, const OGRPoint b)
{
    return a.getX() == b.getX()
        && a.getY() == b.getY()
        && a.getDimension() == b.getDimension()
        && (a.getDimension() < 3 || a.getZ() == b.getZ());
}
static uint qHash(const OGRPoint o)
{
    // A good algorithm depends strongly on the data.  In particular,
    // on the projection and extent of the map.  This is written for
    // EPSG:27700 10kmx10km at 1m resolution (i.e. OS OpenData tiles).
    return (uint)(o.getX() * 100000 + o.getY() * 1000000000);
}


Node *ImportExportGdal::nodeFor(Layer* aLayer, const OGRPoint p)
{
    if (pointHash.contains(p)) {
        return pointHash[p];
    }

    pointHash[p] = g_backend.allocNode(aLayer, Coord(p.getY(), p.getX()));
    aLayer->add(pointHash[p]);
    return pointHash[p];
}

// IMPORT

Way *ImportExportGdal::readWay(Layer* aLayer, OGRLineString *poRing)
{
    int numNode = poRing->getNumPoints();

    if (!numNode) return NULL;

    OGRPoint p;

    Way* w = g_backend.allocWay(aLayer);
    aLayer->add(w);
    for(int i = 0;  i < numNode;  i++) {
        poRing->getPoint(i, &p);
        Node *n = nodeFor(aLayer, p);
        w->add(n);
    }
    return w;
}

Feature* ImportExportGdal::parseGeometry(Layer* aLayer, OGRGeometry *poGeometry)
{
    OGRwkbGeometryType type = wkbFlatten(poGeometry->getGeometryType());

    switch(type) {
    case wkbPoint: {
        OGRPoint *p = (OGRPoint*)(poGeometry);
        if (p->getDimension() > 2)
            return nodeFor(aLayer, OGRPoint(p->getX(), p->getY(), p->getZ()));
        else
            return nodeFor(aLayer, OGRPoint(p->getX(), p->getY()));
    }

    case wkbPolygon: {
        OGRPolygon *poPoly = (OGRPolygon*)poGeometry;
        OGRLinearRing *poRing = poPoly->getExteriorRing();
        Way *outer = readWay(aLayer, poRing);
        if (outer) {
            if (int numHoles = poPoly->getNumInteriorRings()) {
                Relation* rel = g_backend.allocRelation(aLayer);
                aLayer->add(rel);
                rel->setTag("type", "multipolygon");
                rel->add("outer", outer);
                for (int i=0;  i<numHoles;  i++) {
                    poRing = poPoly->getInteriorRing(i);
                    Way *inner = readWay(aLayer, poRing);
                    if (inner) {
                        rel->add("inner", inner);
                    }
                }
                return rel;
            }
        }
        return outer;
    }

    case wkbLineString: {
        return readWay(aLayer, (OGRLineString*)poGeometry);
    }

    case wkbMultiPolygon:
        // TODO - merge multipolygon relations if members have holes; for now, fallthrough
    case wkbMultiLineString:
    case wkbMultiPoint:
    {
        OGRGeometryCollection  *poCol = (OGRGeometryCollection*) poGeometry;
        if(int numCol = poCol->getNumGeometries()) {
            Relation* R = g_backend.allocRelation(aLayer);
            aLayer->add(R);
            for(int i=0; i<numCol; i++) {
                Feature* F = parseGeometry(aLayer, poCol->getGeometryRef(i));
                if (F ) {
                    R->add("", F);
                }
            }
            return R;
        } else {
            return NULL;
        }
    }
    default:
        qWarning("SHP: Unrecognised Geometry type %d, ignored", type);
        return NULL;
    }
}

// import the  input

#ifndef GDAL2
#define GDALDataset OGRDataSource
#endif
bool ImportExportGdal::importGDALDataset(GDALDataset* poDS, Layer* aLayer, bool confirmProjection)
#undef GDALDataset
{
    int ogrError;

    OGRSpatialReference wgs84srs;
    if (wgs84srs.SetWellKnownGeogCS("WGS84") != OGRERR_NONE) {
        qDebug("GDAL: couldn't initialise WGS84: %s", CPLGetLastErrorMsg());
        return false;
    }

    qDebug(lc_importExportGdal) << "Layers #" << poDS->GetLayerCount();
    OGRLayer  *poLayer = poDS->GetLayer(0);

    const OGRSpatialReference * theSrs = poLayer->GetSpatialRef(); // Note: Contrary to other OGR objects, the spatial ref must NOT be released by our code!
    toWGS84 = NULL;

    if (theSrs) {
        // Workaround for OSGB - otherwise its datum is ignored (TODO: why?)
        // TODO: Is it necessary with current GDAL versions? The hack is pretty ancient.
        QString gcs = theSrs->GetAttrValue("GEOGCS");
        if (gcs == "GCS_OSGB_1936" || gcs == "OSGB 1936") {
            qDebug(lc_importExportGdal) << "GDAL: substituting GCS_OSGB_1936 with EPSG:27700";
            OGRSpatialReference * the27700Srs = new OGRSpatialReference();
            if ((ogrError = the27700Srs->importFromEPSG(27700)) != OGRERR_NONE) {
                qDebug("GDAL: couldn't initialise EPSG:27700: %d: %s", ogrError, CPLGetLastErrorMsg());
                the27700Srs->Release();
            } else {
                theSrs = the27700Srs;
            }
        }
    }

    theSrs = poLayer->GetSpatialRef();
    toWGS84 = OGRCreateCoordinateTransformation(theSrs, &wgs84srs);
    if (!toWGS84)
        return false;

    QProgressDialog progress(QApplication::tr("Importing..."), QApplication::tr("Cancel"), 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.setRange(0, 0);
    progress.show();

    int totimported = 0;
    OGRFeature *poFeature;
    for (int l=0; l<poDS->GetLayerCount() && !progress.wasCanceled(); ++l) {
        poLayer = poDS->GetLayer(l);

        int sz = poLayer->GetFeatureCount(FALSE);
        if (sz != -1)
            progress.setMaximum(progress.maximum()+sz);

        int curImported = 0;
        while( (poFeature = poLayer->GetNextFeature()) != NULL && !progress.wasCanceled())
        {
            OGRGeometry *poGeometry;

            poGeometry = poFeature->GetGeometryRef();
            if( poGeometry != NULL) {
                // qDebug( "GeometryType : %d,", poGeometry->getGeometryType() );

                poGeometry->transform(toWGS84);

                Feature* F = parseGeometry(aLayer, poGeometry);
                if (F) {
                    for (int i=0; i<poFeature->GetFieldCount(); ++i) {
                        const OGRFieldDefn  *fd = poFeature->GetFieldDefnRef(i);
                        QString k = QString::fromUtf8(fd->GetNameRef());
                        if (k == "osm_id") {
                            F->setId(IFeature::FId(F->getType(), (qint64)poFeature->GetFieldAsDouble(i)));
#ifndef FRISIUS_BUILD
                        } else if (k == "osm_version") {
                            F->setVersionNumber(poFeature->GetFieldAsInteger(i));
                        } else if (k == "osm_timestamp") {
                            F->setTime(QDateTime::fromSecsSinceEpoch(poFeature->GetFieldAsInteger(i)));
#endif
                        } else {
                            if (!g_Merk_NoGuardedTagsImport) {
                                k.prepend("_");
                                k.append("_");
                            }
                            F->setTag(k, QString::fromUtf8(poFeature->GetFieldAsString(i)));
                        }
                    }
                }
                ++curImported;
                ++totimported;
                progress.setLabelText(QApplication::tr("Imported: %1").arg(totimported));
                if (progress.maximum() > 0)
                    progress.setValue(totimported);
                qApp->processEvents();
            }
            else
            {
                qDebug( "no geometry" );
            }
//            if (progress.maximum() != 0)
//                progress.setValue(progress.value()+1);
        }
        qDebug() << "Layer #" << l << "Features#:" << curImported;
    }

    pointHash.clear();

#ifndef _MOBILE
    QApplication::restoreOverrideCursor();
#endif

    delete toWGS84;

    if (progress.wasCanceled())
        return false;
    else
        return true;
}

bool ImportExportGdal::import(Layer* aLayer)
{
#ifdef GDAL2
    GDALAllRegister();
    GDALDataset *poDS;
    poDS = (GDALDataset *) GDALOpenEx( FileName.toUtf8().constData(), GDAL_OF_VECTOR | GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR, NULL, NULL, NULL );
#else
    OGRRegisterAll();
    OGRDataSource *poDS;
    poDS = OGRSFDriverRegistrar::Open( FileName.toUtf8().constData(), FALSE );
#endif

    if( poDS == NULL ) {
        //qDebug() << "GDAL Open failed from file" << FileName.toUtf8().constData();
        return false;
    }

    importGDALDataset(poDS, aLayer, M_PREFS->getGdalConfirmProjection());

    GDALClose( (GDALDatasetH) poDS );

    return true;
}

bool ImportExportGdal::import(Layer* aLayer, const QByteArray& ba, bool confirmProjection)
{
    GByte* content = (GByte*)(ba.constData());
    VSILFILE* f = VSIFileFromMemBuffer("/vsimem/temp", content, ba.size(), FALSE);

#ifdef GDAL2
    GDALAllRegister();
    GDALDataset *poDS;
    poDS = (GDALDataset *) GDALOpen( "/vsimem/temp", GA_ReadOnly );
#else
    OGRRegisterAll();
    OGRDataSource *poDS;
    poDS = OGRSFDriverRegistrar::Open( "/vsimem/temp", FALSE );
#endif

    if( poDS == NULL )
    {
        qDebug( "GDAL Open failed." );
        return false;
    }
    importGDALDataset(poDS, aLayer, confirmProjection);

    GDALClose( (GDALDatasetH) poDS );
    VSIFCloseL(f);

    return true;
}
