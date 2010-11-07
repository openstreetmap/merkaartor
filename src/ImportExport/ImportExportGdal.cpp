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

#include <QtGui>

#include "../ImportExport/ImportExportGdal.h"
#include "Maps/Projection.h"
#include "Features.h"
#include "Utils/ProjectionChooser.h"


#include "ogrsf_frmts.h"

#include <QDir>

bool parseContainer(QDomElement& e, Layer* aLayer);

ImportExportGdal::ImportExportGdal(Document* doc)
 : IImportExport(doc), theProjection(0)
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


Node *ImportExportGdal::nodeFor(const OGRPoint p)
{
    if (pointHash.contains(p)) {
        return pointHash[p];
    }

    if (toWGS84)
        return pointHash[p] = new Node(Coord(angToCoord(p.getY()), angToCoord(p.getX())));
    else {
        Coord c = theProjection->inverse2Coord(QPointF(p.getX(), p.getY()));
        return pointHash[p] = new Node(c);
    }
}

// IMPORT

Way *ImportExportGdal::readWay(Layer* aLayer, OGRLineString *poRing) {
    int numNode = poRing->getNumPoints();

    if (!numNode) return NULL;

    OGRPoint p;

    Way* w = new Way();
    for(int i = 0;  i < numNode;  i++) {
        poRing->getPoint(i, &p);
        Node *n = nodeFor(p);
        if (!aLayer->exists(n))
            aLayer->add(n);
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
                if (!aLayer->exists(outer))
                    aLayer->add(outer);
                Relation* rel = new Relation;
                aLayer->add(rel);
                rel->setTag("type", "multipolygon");
                rel->add("outer", outer);
                for (int i=0;  i<numHoles;  i++) {
                    poRing = poPoly->getInteriorRing(i);
                    Way *inner = readWay(aLayer, poRing);
                    if (inner) {
                        if (!aLayer->exists(inner))
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
                if (F ) {
                    if (!aLayer->exists(F))
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
bool ImportExportGdal::import(Layer* aLayer)
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
    toWGS84 = NULL;
    theProjection = NULL;

    if (theSrs) {
        // Workaround for OSGB - otherwise its datum is ignored (TODO: why?)
        QString gcs = theSrs->GetAttrValue("GEOGCS");
        if (gcs == "GCS_OSGB_1936" || gcs == "OSGB 1936") {
            qDebug() << "SHP: substituting GCS_OSGB_1936 with EPSG:27700";
            OGRSpatialReference * the27700Srs = new OGRSpatialReference();
            if ((ogrError = the27700Srs->importFromEPSG(27700)) != OGRERR_NONE) {
                qDebug("SHP: couldn't initialise EPSG:27700: %d: %s", ogrError, CPLGetLastErrorMsg());
                delete the27700Srs;
            } else {
                theSrs = the27700Srs;
            }
        }
        toWGS84 = OGRCreateCoordinateTransformation(theSrs, &wgs84srs);
    }

    theProjection = new Projection();
    if (theSrs) {
        if (!toWGS84) {
            theSrs->morphFromESRI();
            {
                char* cTheProj4;
                if (theSrs->exportToProj4(&cTheProj4) != OGRERR_NONE) {
                    qDebug() << "SHP: to proj4 error: " << CPLGetLastErrorMsg();
                    return false;
                }
                QString theProj4(cTheProj4);
                // Hack because GDAL (as of 1.6.1) do not recognize "DATUM["D_OSGB_1936"" from the WKT
                QString datum = theSrs->GetAttrValue("DATUM");
                if (datum == "OSGB_1936" && !theProj4.contains("+datum"))
                    theProj4 += " +datum=OSGB36";
                qDebug() << "SHP: to proj4 : " << theProj4;
                theProjection->setProjectionType(QString(theProj4));
            }
        }
    } else {
        QString sPrj;
        sPrj = ProjectionChooser::getProjection(QCoreApplication::translate("ImportExportGdal", "Unable to set projection; please specify one"));
        if (sPrj.isEmpty()) {
            delete theProjection;
            return false;
        }

        if (!theProjection->setProjectionType(sPrj)) {
//            QMessageBox::critical(0,QCoreApplication::translate("ImportExportGdal","Cannot load file"),QCoreApplication::translate("ImportExportGdal","Unable to set projection."));
            delete theProjection;
            return false;
        }
    }

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
                if (!aLayer->exists(F))
                    aLayer->add(F);
                for (int i=0; i<poFeature->GetFieldCount(); ++i) {
                    OGRFieldDefn  *fd = poFeature->GetFieldDefnRef(i);
                    QString k(fd->GetNameRef());
                    if (!g_Merk_NoGuardedTagsImport) {
                        k.prepend("_");
                        k.append("_");
                    }
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

