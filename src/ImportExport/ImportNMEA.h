//
// C++ Interface: ImportNMEA
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IMPORTNMEA_H
#define IMPORTNMEA_H

#include "IImportExport.h"

/**
    @author cbro <cbro@semperpax.com>
*/
class ImportNMEA : public IImportExport
{
public:
    ImportNMEA(Document* doc);

    ~ImportNMEA();

    // import the  input
    virtual bool import(Layer* aLayer);
    // export
    virtual bool export_(const QList<Feature *>& featList);

private:
    TrackLayer* theLayer;

    bool importGSA (QString line);
    bool importGSV (QString line);
    bool importGGA (QString line);
    bool importGLL (QString line);
    TrackNode* importRMC (QString line);

    qreal curAltitude;

};

#endif
