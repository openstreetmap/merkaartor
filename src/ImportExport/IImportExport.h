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

#include "Features.h"
#include "Layer.h"
#include "Command.h"
#include "DocumentCommands.h"

/**
Interface for Import/Export

    @author cbro <cbro@semperpax.com>
*/
class IImportExport{
public:
    IImportExport(Document* doc);
    virtual ~IImportExport();

public:
    // Specify the input as a QIODevice
    virtual bool setDevice(QIODevice* aDevice);
    // Specify the input as a QFile
    virtual bool loadFile(QString filename);
    // Specify the output as a QFile
    virtual bool saveFile(QString filename);
    // import the  input
    virtual bool import(Layer* /* aLayer */) { return false; }
    // export
    virtual bool export_(const QList<Feature *>& featList = QList<Feature *>());

    // Return the filename
    const QString& getFilename() const;

    virtual CommandList* getCommandList();

protected:
    Document* theDoc;
    QIODevice* Device;
    CommandList* theList;
    QList<Feature*> theFeatures;
    QString FileName;
    bool ownDevice;
};

#endif
