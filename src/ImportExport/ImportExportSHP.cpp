//
// C++ Implementation: ImportExportSHP
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtGui>

#include "../ImportExport/ImportExportSHP.h"
#include "Maps/Projection.h"
#include "Features.h"

#include "ogrsf_frmts.h"

#include <QDir>

bool parseContainer(QDomElement& e, Layer* aLayer);

ImportExportSHP::ImportExportSHP(Document* doc)
 : IImportExport(doc), theProjection(0)
{
}


ImportExportSHP::~ImportExportSHP()
{
}

// Specify the input as a QFile
bool ImportExportSHP::loadFile(QString filename)
{
    FileName = filename;

    return true;
}

bool ImportExportSHP::saveFile(QString)
{
    return false;
}


// export
bool ImportExportSHP::export_(const QList<Feature *>& featList)
{
    Q_UNUSED(featList);

    return false;
}

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


Node *ImportExportSHP::nodeFor(const OGRPoint p)
{
    if (pointHash.contains(p)) {
        return pointHash[p];
    }

    return pointHash[p] = new Node(Coord(angToInt(p.getY()), angToInt(p.getX())));
}

// IMPORT

Way *ImportExportSHP::readWay(Layer* aLayer, OGRLineString *poRing) {
    int numNode = poRing->getNumPoints();

    if (!numNode) return NULL;

    OGRPoint p;

    Way* w = new Way();
    for(int i = 0;  i < numNode;  i++) {
        poRing->getPoint(i, &p);
        Node *n = nodeFor(p);
        aLayer->add(n);
        w->add(n);
    }
    return w;
}

Feature* ImportExportSHP::parseGeometry(Layer* aLayer, OGRGeometry *poGeometry)
{
    OGRwkbGeometryType type = wkbFlatten(poGeometry->getGeometryType());

    switch(type) {
    case wkbPoint: {
        OGRPoint *p = (OGRPoint*)(poGeometry);
        if (p->getDimension() > 2)
            return nodeFor(OGRPoint(p->getX(), p->getY(), p->getZ()));
        else
            return nodeFor(OGRPoint(p->getX(), p->getY()));
    }

    case wkbPolygon: {
        OGRPolygon *poPoly = (OGRPolygon*)poGeometry;
        OGRLinearRing *poRing = poPoly->getExteriorRing();
        Way *outer = readWay(aLayer, poRing);
        if (outer) {
            if (int numHoles = poPoly->getNumInteriorRings()) {
                aLayer->add(outer);
                Relation* rel = new Relation;
                rel->setTag("type", "multipolygon");
                rel->add("outer", outer);
                for (int i=0;  i<numHoles;  i++) {
                    poRing = poPoly->getInteriorRing(i);
                    Way *inner = readWay(aLayer, poRing);
                    if (inner) {
                        aLayer->add(inner);
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
            Relation* R = new Relation;
            for(int i=0; i<numCol; i++) {
                Feature* F = parseGeometry(aLayer, poCol->getGeometryRef(i));
                if (F) {
                    aLayer->add(F);
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
bool ImportExportSHP::import(Layer* aLayer)
{
    OGRRegisterAll();

    OGRDataSource       *poDS;
    int ogrError;

    poDS = OGRSFDriverRegistrar::Open( FileName.toUtf8().constData(), FALSE );
    if( poDS == NULL )
    {
        qDebug( "SHP Open failed.\n" );
        return false;
    }

    OGRSpatialReference wgs84srs;
    if (wgs84srs.SetWellKnownGeogCS("WGS84") != OGRERR_NONE) {
        qDebug("SHP: couldn't initialise WGS84: %s", CPLGetLastErrorMsg());
        return false;
    }

    OGRLayer  *poLayer;
    // TODO: iterate over all layers?
    poLayer = poDS->GetLayer( 0 );

    OGRSpatialReference * theSrs = poLayer->GetSpatialRef();
    OGRCoordinateTransformation *toWGS84 = NULL;

    // { char *wkt; theSrs->exportToPrettyWkt(&wkt); qDebug() << "SHP: input SRS:" << endl << wkt; }

    // Workaround for OSGB - otherwise its datum is ignored (TODO: why?)
    QString gcs = theSrs->GetAttrValue("GEOGCS");
    if (gcs == "GCS_OSGB_1936" || gcs == "OSGB 1936") {
        qDebug() << "SHP: substituting GCS_OSGB_1936 with EPSG:27700";
        if ((ogrError = theSrs->importFromEPSG(27700)) != OGRERR_NONE) {
            qDebug("SHP: couldn't initialise EPSG:27700: %d: %s", ogrError, CPLGetLastErrorMsg());
            return false;
        }
    }

    if (theSrs)
        toWGS84 = OGRCreateCoordinateTransformation(theSrs, &wgs84srs);

    OGRFeature *poFeature;

    poLayer->ResetReading();
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
        OGRGeometry *poGeometry;

        poGeometry = poFeature->GetGeometryRef();
        if( poGeometry != NULL) {
            // qDebug( "GeometryType : %d,", poGeometry->getGeometryType() );

            if (toWGS84)
                poGeometry->transform(toWGS84);

            Feature* F = parseGeometry(aLayer, poGeometry);
            if (F) {
                aLayer->add(F);
                for (int i=0; i<poFeature->GetFieldCount(); ++i) {
                    OGRFieldDefn  *fd = poFeature->GetFieldDefnRef(i);
                    QString k(fd->GetNameRef());
                    k.prepend("_");
                    k.append("_");
                    F->setTag(k, poFeature->GetFieldAsString(i));
                }
            }
        }
        else
        {
            qDebug( "no geometry\n" );
        }
    }

    pointHash.clear();

    if (toWGS84)
        delete toWGS84;

    OGRDataSource::DestroyDataSource( poDS );

    return true;
}

