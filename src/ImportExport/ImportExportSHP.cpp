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

#include "ogrsf_frmts.h"

#include <QDir>

bool parseContainer(QDomElement& e, MapLayer* aLayer);

ImportExportSHP::ImportExportSHP(MapDocument* doc)
 : IImportExport(doc)
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
bool ImportExportSHP::export_(const QVector<MapFeature *>& featList)
{
	Q_UNUSED(featList);

	return false;
}

// IMPORT

void parseGeometry(MapLayer* aLayer, OGRGeometry *poGeometry)
{
	if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
	{
		OGRPoint *poPoint = (OGRPoint *) poGeometry;

		TrackPoint* N = new TrackPoint(Coord(angToInt(poPoint->getY()), angToInt(poPoint->getX())));

		aLayer->add(N);
	} else
	if ( wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
	{
		OGRPolygon  *poPoly = (OGRPolygon *) poGeometry;
		OGRLinearRing *poRing = poPoly->getExteriorRing();
		OGRPoint p;

		if(int numNode = poRing->getNumPoints()) {
			Road* R = new Road();
			for(int i=0; i<numNode; i++) {
				poRing->getPoint(i, &p);
				TrackPoint* N = new TrackPoint(Coord(angToInt(p.getY()), angToInt(p.getX())));
				aLayer->add(N);

				R->add(N);
			}
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
				TrackPoint* N = new TrackPoint(Coord(angToInt(p.getY()), angToInt(p.getX())));
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

	poDS = OGRSFDriverRegistrar::Open( FileName.toUtf8().constData(), FALSE );
	if( poDS == NULL )
	{
		qDebug( "SHP Open failed.\n" );
		return false;
	}

	OGRLayer  *poLayer;

	//poLayer = poDS->GetLayerByName( "point" );
	poLayer = poDS->GetLayer( 0 );

	OGRFeature *poFeature;

	poLayer->ResetReading();
	while( (poFeature = poLayer->GetNextFeature()) != NULL )
	{
		//OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		//int iField;

		//for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
		//{
		//	OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );

		//	if( poFieldDefn->GetType() == OFTInteger )
		//		qDebug( "%d,", poFeature->GetFieldAsInteger( iField ) );
		//	else if( poFieldDefn->GetType() == OFTReal )
		//		qDebug( "%.3f,", poFeature->GetFieldAsDouble(iField) );
		//	else if( poFieldDefn->GetType() == OFTString )
		//		qDebug( "%s,", poFeature->GetFieldAsString(iField) );
		//	else
		//		qDebug( "%s,", poFeature->GetFieldAsString(iField) );
		//}

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

