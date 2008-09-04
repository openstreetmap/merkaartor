//
// C++ Implementation: IImportExport
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

#include "../ImportExport/IImportExport.h"


IImportExport::~IImportExport()
{
	if (Device->isOpen())
		Device->close();
	delete Device;
}

// Specify the input as a QFile
bool IImportExport::loadFile(QString filename)
{
	FileName = filename;
	Device = new QFile(filename);
	return Device->open(QIODevice::ReadOnly);
}

bool IImportExport::saveFile(QString filename)
{
	FileName = filename;
	Device = new QFile(filename);
	return Device->open(QIODevice::WriteOnly | QIODevice::Truncate);
}

bool IImportExport::export_(const QVector<MapFeature *>& featList)
{
	theFeatures = featList;

	return true;
}

CommandList* IImportExport::getCommandList()
{
	return theList;
}

const QString& IImportExport::getFilename() const
{
	return FileName;
}
