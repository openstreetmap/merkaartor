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

#include "Coord.h"
#include "IRenderer.h"

#include <ui_NativeRenderDialog.h>

class Document;
class MapView;
class CoordBox;
class QPrinter;
class QPrintPreviewDialog;
class QPrintPreviewWidget;

class NativeRenderDialog: public QObject
{
    Q_OBJECT

public:
    NativeRenderDialog(Document *aDoc, const CoordBox& aCoordBox, QWidget *parent = 0);
    void render(QPainter& P, QRect theR, RendererOptions opt);

    RendererOptions options();
    void setOptions(RendererOptions aOpt);
    CoordBox boundingBox();
    void setBoundingBox(CoordBox aBBox);

public slots:
    void exportPDF();
    void exportSVG();
    void exportRaster();

public slots:
    int exec();

private slots:
    void renderPreview(QPrinter* prt);

private:
    void setPrinterOptions();

    Ui::NativeRenderWidget ui;
    Document* theDoc;
    MapView* mapview;
    CoordBox theOrigBox;
    QSettings*	Sets;
    double		ratio;
    QPrinter* thePrinter;
    QPrintPreviewDialog*  preview;
    QPrintPreviewWidget* prtW;
    QIntValidator* dpiValidator;
    int dpiMinimumValue;

};

#endif
