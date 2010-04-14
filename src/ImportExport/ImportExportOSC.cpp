//
// C++ Implementation: ImportExportKML
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

#include "../ImportExport/ImportExportOSC.h"

#include "DirtyListExecutorOSC.h"

bool parseContainer(QDomElement& e, Layer* aLayer);

ImportExportOSC::ImportExportOSC(Document* doc)
 : IImportExport(doc)
{
}


ImportExportOSC::~ImportExportOSC()
{
}


// export
bool ImportExportOSC::export_(const QList<Feature *>&)
{
    DirtyListBuild Future;
    theDoc->history().buildDirtyList(Future);

    Future.resetUpdates();
    DirtyListExecutorOSC Exec(theDoc, Future);
    QString doc = Exec.getChanges();

    return (Device->write(doc.toUtf8()) != -1);
}

// IMPORT


// import the  input
bool ImportExportOSC::import(Layer* aLayer)
{
    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(Device)) {
        //QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        Device->close();
        delete theXmlDoc;
        theXmlDoc = NULL;
        return false;
    }
    Device->close();

    QDomElement docElem = theXmlDoc->documentElement();
}

