//
// C++ Interface: IImportExport
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IIMPORTEXPORT_H
#define IIMPORTEXPORT_H

class QString;
class QIODevice;

#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/MapLayer.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"

/**
Interface for Import/Export

	@author cbro <cbro@semperpax.com>
*/
class IImportExport{
public:
	IImportExport(MapDocument* doc);
	virtual ~IImportExport();

public:
	// Specify the input as a QFile
	virtual bool loadFile(QString filename);
	// Specify the output as a QFile
	virtual bool saveFile(QString filename);
	// import the  input
	virtual bool import(MapLayer* /* aLayer */) { return false; };
	// export
	virtual bool export_(const QVector<MapFeature *>& featList);

	// Return the filename
	const QString& getFilename() const;

	virtual CommandList* getCommandList();

protected:
	MapDocument* theDoc;
	QIODevice* Device;
	CommandList* theList;
	QVector<MapFeature*> theFeatures;
	QString FileName;
};

#endif
