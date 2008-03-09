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


// Specify the input as a QFile
bool IImportExport::loadFile(QString filename)
{
	source = new QFile(filename);
	return source->open(QIODevice::ReadOnly);
}

CommandList* IImportExport::getCommandList()
{
	return theList;
}
