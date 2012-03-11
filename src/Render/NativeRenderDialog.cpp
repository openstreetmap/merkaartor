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
#include "Projection.h"
#include "Layer.h"
#include "Features.h"
#include "MerkaartorPreferences.h"
#include "MasPaintStyle.h"
#include "PictureViewerDialog.h"

#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>

#include <QMainWindow>
#include <QProgressDialog>
#include <QPainter>
#include <QSvgGenerator>
#include <QFileDialog>

NativeRenderDialog::NativeRenderDialog(Document *aDoc, const CoordBox& aCoordBox, QWidget *parent)
    :QObject(parent), theDoc(aDoc), theOrigBox(aCoordBox)
{
    thePrinter = new QPrinter();
    thePrinter->setDocName(aDoc->title());

    mapview = new MapView(NULL);
    mapview->setDocument(theDoc);

    preview = new QPrintPreviewDialog( thePrinter, parent );
    QMainWindow* mw = preview->findChild<QMainWindow*>();
    prtW = dynamic_cast<QPrintPreviewWidget*>(mw->centralWidget());

    QWidget* myWidget = new QWidget(preview);
    ui.setupUi(myWidget);
    ui.verticalLayout->addWidget(prtW);
    mw->setCentralWidget(myWidget);

    connect(ui.cbShowNodes, SIGNAL(toggled(bool)), prtW, SLOT(updatePreview()));
    connect(ui.cbShowRelations, SIGNAL(toggled(bool)), prtW, SLOT(updatePreview()));
    connect(ui.cbShowGrid, SIGNAL(toggled(bool)), prtW, SLOT(updatePreview()));
    connect(ui.cbShowScale, SIGNAL(toggled(bool)), prtW, SLOT(updatePreview()));
    connect(ui.cbShowUnstyled, SIGNAL(toggled(bool)), prtW, SLOT(updatePreview()));
    connect(ui.sbMinLat, SIGNAL(valueChanged(double)), prtW, SLOT(updatePreview()));
    connect(ui.sbMaxLat, SIGNAL(valueChanged(double)), prtW, SLOT(updatePreview()));
    connect(ui.sbMinLon, SIGNAL(valueChanged(double)), prtW, SLOT(updatePreview()));
    connect(ui.sbMaxLon, SIGNAL(valueChanged(double)), prtW, SLOT(updatePreview()));

    connect(ui.btExportPDF, SIGNAL(clicked()), SLOT(exportPDF()));
    connect(ui.btExportSVG, SIGNAL(clicked()), SLOT(exportSVG()));
    connect(ui.btExportRaster, SIGNAL(clicked()), SLOT(exportRaster()));

    connect( preview, SIGNAL(paintRequested(QPrinter*)), SLOT(print(QPrinter*)) );
    setBoundingBox(aCoordBox);
    setOptions(M_PREFS->getRenderOptions());
}

RendererOptions NativeRenderDialog::options()
{
    RendererOptions opt;
    opt.options |= RendererOptions::ForPrinting;
    opt.options |= RendererOptions::BackgroundVisible;
    opt.options |= RendererOptions::ForegroundVisible;
    opt.options |= RendererOptions::TouchupVisible;
    opt.options |= RendererOptions::NamesVisible;

    if (ui.cbShowNodes->isChecked())
        opt.options |= RendererOptions::NodesVisible;
    if (ui.cbShowRelations->isChecked())
        opt.options |= RendererOptions::RelationsVisible;
    if (ui.cbShowScale->isChecked())
        opt.options |= RendererOptions::ScaleVisible;
    if (ui.cbShowGrid->isChecked())
        opt.options |= RendererOptions::LatLonGridVisible;
    if (!ui.cbShowUnstyled->isChecked())
        opt.options |= RendererOptions::UnstyledHidden;

    return opt;
}

void NativeRenderDialog::setOptions(RendererOptions aOpt)
{
    ui.cbShowNodes->setChecked(aOpt.options & RendererOptions::NodesVisible);
    ui.cbShowRelations->setChecked(aOpt.options & RendererOptions::RelationsVisible);
    ui.cbShowScale->setChecked(aOpt.options & RendererOptions::ScaleVisible);
    ui.cbShowGrid->setChecked(aOpt.options & RendererOptions::LatLonGridVisible);
    ui.cbShowUnstyled->setChecked(!(aOpt.options & RendererOptions::UnstyledHidden));

    prtW->updatePreview();
}

CoordBox NativeRenderDialog::boundingBox()
{
    CoordBox VP(Coord(
                    ui.sbMinLon->value(),
                    ui.sbMinLat->value()
            ), Coord(
                    ui.sbMaxLon->value(),
                    ui.sbMaxLat->value()
                    ));
    return VP;
}

void NativeRenderDialog::setBoundingBox(CoordBox aBBox)
{
    ui.sbMinLat->setValue(aBBox.bottomLeft().y());
    ui.sbMaxLat->setValue(aBBox.topLeft().y());
    ui.sbMinLon->setValue(aBBox.topLeft().x());
    ui.sbMaxLon->setValue(aBBox.topRight().x());

    prtW->updatePreview();
}

int NativeRenderDialog::exec()
{
    return preview->exec();
}

void NativeRenderDialog::print(QPrinter* prt)
{
    QPainter P(prt);
    P.setRenderHint(QPainter::Antialiasing);
    QRect theR = prt->pageRect();
    theR.moveTo(0, 0);
    render(P, theR, options());
}

void NativeRenderDialog::render(QPainter& P, QRect theR, RendererOptions opt)
{
    P.setClipRect(theR);
    P.setClipping(true);
    P.setRenderHint(QPainter::Antialiasing);

    mapview->setGeometry(theR);
    mapview->setViewport(boundingBox(), theR);
    mapview->setRenderOptions(opt);
    mapview->invalidate(true, false);
    mapview->drawFeatures(P);
    if (opt.options & RendererOptions::ScaleVisible)
        mapview->drawScale(P);
    if (opt.options & RendererOptions::LatLonGridVisible)
        mapview->drawLatLonGrid(P);
}

void NativeRenderDialog::exportPDF()
{
    QString s;
    QFileDialog dlg(NULL, tr("Output filename"), QString("%1/%2.pdf").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("PDF files (*.pdf)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("pdf");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            s = dlg.selectedFiles()[0];
    }
//    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("PDF files (*.pdf)"));
    if (s.isNull())
        return;

#if QT_VERSION >= 0x040500
    QPrinter* prt = preview->printer();
#else
    QPrinter* prt = thePrinter;
#endif
    prt->setOutputFormat(QPrinter::PdfFormat);
    prt->setOutputFileName(s);

    QPainter P(prt);
    P.setRenderHint(QPainter::Antialiasing);
    QRect theR = prt->pageRect();
    theR.moveTo(0, 0);
    RendererOptions opt = options();
    opt.options |= RendererOptions::PrintAllLabels;
    render(P, theR, opt);
}

void NativeRenderDialog::exportRaster()
{
    QString s;
    QFileDialog dlg(NULL, tr("Output filename"), QString("%1/%2.png").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Image files (*.png *.jpg)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("png");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            s = dlg.selectedFiles()[0];
    }
//    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("Image files (*.png *.jpg)"));
    if (s.isNull())
        return;
#if QT_VERSION >= 0x040500
    QRect theR = preview->printer()->pageRect();
#else
    QRect theR = thePrinter->pageRect();
#endif
    theR.moveTo(0, 0);

    QPixmap pix(theR.size());
    if (M_PREFS->getUseShapefileForBackground())
        pix.fill(M_PREFS->getWaterColor());
    else if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
        pix.fill(M_PREFS->getBgColor());
    else
        pix.fill(M_STYLE->getGlobalPainter().getBackgroundColor());

    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);
    render(P, theR, options());

    pix.save(s);
}

void NativeRenderDialog::exportSVG()
{
    QString s;
    QFileDialog dlg(NULL, tr("Output filename"), QString("%1/%2.svg").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("SVG files (*.svg)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("svg");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            s = dlg.selectedFiles()[0];
    }
//    QString s = QFileDialog::getSaveFileName(NULL,tr("Output filename"),"",tr("SVG files (*.svg)"));
    if (s.isNull())
        return;

    QSvgGenerator svgg;
#if QT_VERSION >= 0x040500
    QRect theR = preview->printer()->pageRect();
#else
    QRect theR = thePrinter->pageRect();
#endif
    theR.moveTo(0, 0);
    svgg.setSize(theR.size());
    svgg.setFileName(s);
#if QT_VERSION >= 0x040500
        svgg.setViewBox(theR);
#endif

    QPainter P(&svgg);
    P.setRenderHint(QPainter::Antialiasing);
    RendererOptions opt = options();
    opt.options |= RendererOptions::PrintAllLabels;

    render(P, theR, opt);
}

