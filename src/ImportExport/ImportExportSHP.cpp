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

bool ImportExportSHP::saveFile(QString filename)
{
    return false;
}


// export
bool ImportExportSHP::export_(const QList<Feature *>& featList)
{
    Q_UNUSED(featList);

    return false;
}

// IMPORT

Feature* ImportExportSHP::parseGeometry(Layer* aLayer, OGRGeometry *poGeometry)
{
    Feature* F = NULL;
    Node* N;
    double x, y;
    if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
    {
        OGRPoint *poPoint = (OGRPoint *) poGeometry;
        x = poPoint->getX(); y = poPoint->getY();
        if (!theProjection || theProjection->projIsLatLong())
            N = new Node(Coord(angToInt(y), angToInt(x)));
        else {
            theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
            N = new Node(Coord(radToInt(y), radToInt(x)));
        }

        aLayer->add(N);
        F = N;
    } else
    if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
    {
        OGRPolygon  *poPoly = (OGRPolygon *) poGeometry;
        OGRLinearRing *poRing = poPoly->getExteriorRing();
        OGRPoint p;
        Node* firstPoint = NULL;

        if(int numNode = poRing->getNumPoints()) {
            Way* R = new Way();
            for(int i=0; i<numNode-1; i++) {
                poRing->getPoint(i, &p);
                x = p.getX(); y = p.getY();
                if (!theProjection || theProjection->projIsLatLong())
                    N = new Node(Coord(angToInt(y), angToInt(x)));
                else {
                    theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
                    N = new Node(Coord(radToInt(y), radToInt(x)));
                }
                aLayer->add(N);
                R->add(N);

                if (!firstPoint)
                    firstPoint = N;
            }
            R->add(firstPoint);
            aLayer->add(R);
            F = R;
        }
    } else
    if ( wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
    {
        OGRLineString  *poLS = (OGRLineString *) poGeometry;
        OGRPoint p;

        if(int numNode = poLS->getNumPoints()) {
            Way* R = new Way();
            for(int i=0; i<numNode; i++) {
                poLS->getPoint(i, &p);
                x = p.getX(); y = p.getY();
                if (!theProjection || theProjection->projIsLatLong())
                    N = new Node(Coord(angToInt(y), angToInt(x)));
                else {
                    theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
                    N = new Node(Coord(radToInt(y), radToInt(x)));
                }
                aLayer->add(N);

                R->add(N);
            }
            aLayer->add(R);
            F = R;
        }
    } else
    if (
        ( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon  ) ||
        ( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString  ) ||
        ( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint  )
        )
    {
        OGRGeometryCollection  *poCol = (OGRGeometryCollection *) poGeometry;

        if(int numCol = poCol->getNumGeometries()) {
            Relation* R = new Relation;
            for(int i=0; i<numCol; i++) {
                Feature* F = parseGeometry(aLayer, poCol->getGeometryRef(i));
                if (F)
                    R->add("", F);
            }
            aLayer->add(R);
            F = R;
        }
    }
    return F;
}

// import the  input
bool ImportExportSHP::import(Layer* aLayer)
{
    OGRRegisterAll();

    OGRDataSource       *poDS;

    QFileInfo fi(FileName);
//    poDS = OGRSFDriverRegistrar::Open( fi.path().toUtf8().constData(), FALSE );
    poDS = OGRSFDriverRegistrar::Open( FileName.toUtf8().constData(), FALSE );
    if( poDS == NULL )
    {
        qDebug( "SHP Open failed.\n" );
        return false;
    }

    OGRLayer  *poLayer;

    //poLayer = poDS->GetLayerByName( "point" );
    poLayer = poDS->GetLayer( 0 );
    OGRSpatialReference * theSrs = poLayer->GetSpatialRef();
    if (theSrs) {
        theProjection = new Projection();
        theSrs->morphFromESRI();
//        if (theSrs->AutoIdentifyEPSG() == OGRERR_NONE)
//        {
//            qDebug() << "SHP: EPSG:" << theSrs->GetAuthorityCode(NULL);
//            theProjection->setProjectionType(QString("EPSG:%1").arg(theSrs->GetAuthorityCode(NULL)));
//        } else
        {
            char* cTheProj4;
            if (theSrs->exportToProj4(&cTheProj4) == OGRERR_NONE) {
                qDebug() << "SHP: to proj4 : " << cTheProj4;
            } else {
                qDebug() << "SHP: to proj4 error: " << CPLGetLastErrorMsg();
                return false;
            }
            QString theProj4(cTheProj4);

            // Hack because GDAL (as of 1.6.1) do not recognize "DATUM["D_OSGB_1936"" from the WKT
            QString datum = theSrs->GetAttrValue("DATUM");
            if (datum == "OSGB_1936" && !theProj4.contains("+datum"))
                theProj4 += " +datum=OSGB36";

            theProjection->setProjectionType(QString(theProj4));
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

            Feature* F = parseGeometry(aLayer, poGeometry);
            if (F) {
                for (int i=0; i<poFeature->GetFieldCount(); ++i) {
                    OGRFieldDefn  *fd = poFeature->GetFieldDefnRef(i);
                    QString k(fd->GetNameRef());
                    k = "_" + k + "_";
                    F->setTag(k, poFeature->GetFieldAsString(i));
                }
            }
        }
        else
        {
            qDebug( "no geometry\n" );
        }

        OGRFeature::DestroyFeature( poFeature );
    }

    OGRDataSource::DestroyDataSource( poDS );

    return true;
}

