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

#include "ogrsf_frmts.h"

#include <QDir>

bool parseContainer(QDomElement& e, MapLayer* aLayer);

ImportExportSHP::ImportExportSHP(MapDocument* doc)
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
bool ImportExportSHP::export_(const QList<MapFeature *>& featList)
{
	Q_UNUSED(featList);

	return false;
}

// IMPORT

void ImportExportSHP::parseGeometry(MapLayer* aLayer, OGRGeometry *poGeometry)
{
	TrackPoint* N;
	double x, y;
	if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
	{
		OGRPoint *poPoint = (OGRPoint *) poGeometry;
		x = poPoint->getX(); y = poPoint->getY();
		if (!theProjection)
			N = new TrackPoint(Coord(angToInt(y), angToInt(x)));
		else {
			theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
			N = new TrackPoint(Coord(radToInt(y), radToInt(x)));
		}

		aLayer->add(N);
	} else
	if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
	{
		OGRPolygon  *poPoly = (OGRPolygon *) poGeometry;
		OGRLinearRing *poRing = poPoly->getExteriorRing();
		OGRPoint p;
		TrackPoint* firstPoint = NULL;

		if(int numNode = poRing->getNumPoints()) {
			Road* R = new Road();
			for(int i=0; i<numNode-1; i++) {
				poRing->getPoint(i, &p);
				x = p.getX(); y = p.getY();
				if (!theProjection)
					N = new TrackPoint(Coord(angToInt(y), angToInt(x)));
				else {
					theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
					N = new TrackPoint(Coord(radToInt(y), radToInt(x)));
				}
				aLayer->add(N);
				R->add(N);

				if (!firstPoint)
					firstPoint = N;
			}
			R->add(firstPoint);
			aLayer->add(R);
		}
	} else
	if ( wkbFlatten(poGeometry->getGeometryType()) == wkbLineString )
	{
		OGRLineString  *poLS = (OGRLineString *) poGeometry;
		OGRPoint p;

		if(int numNode = poLS->getNumPoints()) {
			Road* R = new Road();
			for(int i=0; i<numNode; i++) {
				poLS->getPoint(i, &p);
				x = p.getX(); y = p.getY();
				if (!theProjection)
					N = new TrackPoint(Coord(angToInt(y), angToInt(x)));
				else {
					theProjection->projTransformToWGS84(1, 0, &x, &y, NULL);
					N = new TrackPoint(Coord(radToInt(y), radToInt(x)));
				}
				aLayer->add(N);

				R->add(N);
			}
			aLayer->add(R);
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
			for(int i=0; i<numCol; i++) {
				parseGeometry(aLayer, poCol->getGeometryRef(i));
			}
		}
	}
}

// import the  input
bool ImportExportSHP::import(MapLayer* aLayer)
{
	OGRRegisterAll();

	OGRDataSource       *poDS;

	QFileInfo fi(FileName);
	poDS = OGRSFDriverRegistrar::Open( fi.path().toUtf8().constData(), FALSE );
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
		theSrs->morphFromESRI();
		char* theProj4;
		if (theSrs->exportToProj4(&theProj4) == OGRERR_NONE) {
			qDebug() << "SHP: to proj4 : " << theProj4;
		} else {
			qDebug() << "SHP: to proj4 error: " << CPLGetLastErrorMsg();
			return false;
		}
		theProjection = new Projection();
		theProjection->setProjectionType(QString(theProj4));
	}

	OGRFeature *poFeature;

	poLayer->ResetReading();
	while( (poFeature = poLayer->GetNextFeature()) != NULL )
	{
		OGRGeometry *poGeometry;

		poGeometry = poFeature->GetGeometryRef();
		if( poGeometry != NULL) {
			// qDebug( "GeometryType : %d,", poGeometry->getGeometryType() );

			parseGeometry(aLayer, poGeometry);
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

