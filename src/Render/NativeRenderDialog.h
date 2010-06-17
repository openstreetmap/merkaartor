//
// C++ Interface: NativeRenderDialog
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NATIVERENDERDIALOG_H
#define NATIVERENDERDIALOG_H

#include <QWidget>
#include <QSettings>

#include "Maps/Coord.h"

#include <ui_NativeRenderDialog.h>

class Document;
class MapView;
class CoordBox;
class QPrinter;
class QMyPrintPreviewDialog;

class NativeRenderDialog: public QObject
{
    Q_OBJECT

public:
    NativeRenderDialog(Document *aDoc, const CoordBox& aCoordBox, QWidget *parent = 0);
    void render(QPainter& P, QRect theR);

public slots:
    void exportPDF();
    void exportSVG();
    void exportRaster();

public slots:
    int exec();

private slots:
    void print(QPrinter* prt);

private:
    Document* theDoc;
    MapView* mapview;
    CoordBox theOrigBox;
    QSettings*	Sets;
    double		ratio;
    QPrinter* thePrinter;
    QMyPrintPreviewDialog*  preview;

};

#endif
