//
// C++ Implementation: NativeRenderDialog
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "NativeRenderDialog.h"

#include "MainWindow.h"
#include "Document.h"
#include "MapView.h"
#include "Maps/Projection.h"
#include "Layer.h"
#include "Features.h"
#include "Preferences/MerkaartorPreferences.h"
#include "PaintStyle/MasPaintStyle.h"
#include "Utils/PictureViewerDialog.h"

#include <QPrinter>
#include <QPrintDialog>

#include <QProgressDialog>
#include <QPainter>
#include <QSvgGenerator>
#include <QFileDialog>

#include "qmyprintpreviewdialog.h"

NativeRenderDialog::NativeRenderDialog(Document *aDoc, const CoordBox& aCoordBox, QWidget *parent)
    :QObject(parent), theDoc(aDoc), theOrigBox(aCoordBox)
{
    thePrinter = new QPrinter();

    mapview = new MapView(NULL);
    mapview->setDocument(theDoc);

    preview = new QMyPrintPreviewDialog( thePrinter, parent );
    connect( preview, SIGNAL(paintRequested(QPrinter*)), SLOT(print(QPrinter*)) );
    preview->setBoundingBox(aCoordBox);

    connect(preview, SIGNAL(exportPDF()), SLOT(exportPDF()));
    connect(preview, SIGNAL(exportRaster()), SLOT(exportRaster()));
    connect(preview, SIGNAL(exportSVG()), SLOT(exportSVG()));

    //    setupUi(this);


//    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

//    buttonBox->addButton(tr("Proceed..."), QDialogButtonBox::ActionRole);
//    Sets = M_PREFS->getQSettings();
//    Sets->beginGroup("NativeRenderDialog");

////    rbSVG->setChecked(Sets->value("rbSVG", true).toBool());
////    rbBitmap->setChecked(Sets->value("rbBitmap", false).toBool());

//    cbShowScale->setCheckState((Qt::CheckState)Sets->value("cbShowScale", "1").toInt());
//    cbShowGrid->setCheckState((Qt::CheckState)Sets->value("cbShowGrid", "1").toInt());
////    cbShowBorders->setCheckState((Qt::CheckState)Sets->value("cbShowBorders", "1").toInt());
////    cbShowLicense->setCheckState((Qt::CheckState)Sets->value("cbShowLicense", "1").toInt());

//    sbMinLat->setValue(coordToAng(aCoordBox.bottomLeft().lat()));
//    sbMaxLat->setValue(coordToAng(aCoordBox.topLeft().lat()));
//    sbMinLon->setValue(coordToAng(aCoordBox.topLeft().lon()));
//    sbMaxLon->setValue(coordToAng(aCoordBox.topRight().lon()));

//    sbPreviewHeight->blockSignals(true);
//    sbPreviewHeight->setValue(Sets->value("sbPreviewHeight", "600").toInt());
//    sbPreviewHeight->blockSignals(false);
//    sbPreviewWidth->setValue(Sets->value("sbPreviewWidth", "800").toInt());

//    Sets->endGroup();


//    calcRatio();
}

int NativeRenderDialog::exec()
{
    return preview->exec();
}

void NativeRenderDialog::render(QPainter& P, QRect theR)
{
    RendererOptions opt = preview->options();

    mapview->setGeometry(theR);
    mapview->setViewport(preview->boundingBox(), theR);
    mapview->setRenderOptions(opt);
    mapview->invalidate(true, false);
    mapview->buildFeatureSet();
    mapview->drawCoastlines(P);
    mapview->drawFeatures(P);
    if (opt.options & RendererOptions::ScaleVisible)
        mapview->drawScale(P);
    if (opt.options & RendererOptions::LatLonGridVisible)
        mapview->drawLatLonGrid(P);
}

void NativeRenderDialog::print(QPrinter* prt)
{
    QPainter P(prt);
    P.setRenderHint(QPainter::Antialiasing);
    QRect theR = prt->pageRect();
    render(P, theR);
}

void NativeRenderDialog::exportPDF()
{
    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("PDF files (*.pdf)"));
    if (s.isNull())
        return;

    QPrinter* prt = preview->printer();
    prt->setOutputFormat(QPrinter::PdfFormat);
    prt->setOutputFileName(s);
    print(prt);
}

void NativeRenderDialog::exportRaster()
{
    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("Image files (*.png *.jpg)"));
    if (s.isNull())
        return;

    QRect theR = preview->printer()->pageRect();

    QPixmap pix(theR.size());
    if (M_PREFS->getUseShapefileForBackground())
        pix.fill(M_PREFS->getWaterColor());
    else if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
        pix.fill(M_PREFS->getBgColor());
    else
        pix.fill(M_STYLE->getGlobalPainter().getBackgroundColor());

    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);
    render(P, theR);

    pix.save(s);
}

void NativeRenderDialog::exportSVG()
{
    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("SVG files (*.svg)"));
    if (s.isNull())
        return;

    QSvgGenerator svgg;
    QRect theR = preview->printer()->pageRect();
    svgg.setSize(theR.size());
    svgg.setFileName(s);
#if QT_VERSION >= 0x040500
        svgg.setViewBox(theR);
#endif

    QPainter P(&svgg);
    P.setRenderHint(QPainter::Antialiasing);
    render(P, theR);
}

