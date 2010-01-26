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

IImportExport::IImportExport(Document* doc)
		: theDoc(doc), Device(0), ownDevice(false)
{
}

IImportExport::~IImportExport()
{
	if (ownDevice) {
		if (Device && Device->isOpen())
			Device->close();
		delete Device;
	}
}

// Specify the input as a QIODevice
bool IImportExport::setDevice(QIODevice* aDevice)
{
	Device = aDevice;
	return true;
}

// Specify the input as a QFile
bool IImportExport::loadFile(QString filename)
{
	FileName = filename;
	Device = new QFile(filename);
	ownDevice = true;
	return Device->open(QIODevice::ReadOnly);
}

bool IImportExport::saveFile(QString filename)
{
	FileName = filename;
	Device = new QFile(filename);
	ownDevice = true;
	return Device->open(QIODevice::WriteOnly | QIODevice::Truncate);
}

bool IImportExport::export_(const QList<Feature *>& featList)
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
