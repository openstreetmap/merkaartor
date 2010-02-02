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

#include "Utils/PictureViewerDialog.h"

#include <QProgressDialog>
#include <QPainter>
#include <QSvgGenerator>

NativeRenderDialog::NativeRenderDialog(Document *aDoc, const CoordBox& aCoordBox, QWidget *parent)
	:QDialog(parent), theDoc(aDoc)
{
	setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	buttonBox->addButton(tr("Proceed..."), QDialogButtonBox::ActionRole);
	Sets = new QSettings();
	Sets->beginGroup("NativeRenderDialog");

	rbSVG->setChecked(Sets->value("rbSVG", true).toBool());
	rbBitmap->setChecked(Sets->value("rbBitmap", false).toBool());

	cbShowScale->setCheckState((Qt::CheckState)Sets->value("cbShowScale", "1").toInt());
	cbShowGrid->setCheckState((Qt::CheckState)Sets->value("cbShowGrid", "1").toInt());
	cbShowBorders->setCheckState((Qt::CheckState)Sets->value("cbShowBorders", "1").toInt());
	cbShowLicense->setCheckState((Qt::CheckState)Sets->value("cbShowLicense", "1").toInt());

	sbMinLat->setValue(intToAng(aCoordBox.bottomLeft().lat()));
	sbMaxLat->setValue(intToAng(aCoordBox.topLeft().lat()));
	sbMinLon->setValue(intToAng(aCoordBox.topLeft().lon()));
	sbMaxLon->setValue(intToAng(aCoordBox.topRight().lon()));

	sbPreviewHeight->blockSignals(true);
	sbPreviewHeight->setValue(Sets->value("sbPreviewHeight", "600").toInt());
	sbPreviewHeight->blockSignals(false);
	sbPreviewWidth->setValue(Sets->value("sbPreviewWidth", "800").toInt());

	calcRatio();

	resize(1,1);
}

void NativeRenderDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
		Sets->setValue("rbSVG", rbSVG->isChecked());
		Sets->setValue("rbBitmap", rbBitmap->isChecked());
		Sets->setValue("cbShowScale", cbShowScale->checkState());
		Sets->setValue("cbShowGrid", cbShowGrid->checkState());
		Sets->setValue("cbShowBorders", cbShowBorders->checkState());
		Sets->setValue("cbShowLicense", cbShowLicense->checkState());
		Sets->setValue("sbPreviewWidth", sbPreviewWidth->value());
		Sets->setValue("sbPreviewHeight", sbPreviewHeight->value());

		render();
	}
}

void NativeRenderDialog::render()
{
	QSvgGenerator svgg;
	QPixmap bitmap;
	QPainter P;

	int w = sbPreviewWidth->value();
	int h = sbPreviewHeight->value();

	if (rbSVG->isChecked()) {
		svgg.setFileName(QDir::tempPath()+"/tmp.svg");
		svgg.setSize(QSize(w,h));
#if QT_VERSION >= 0x040500
		svgg.setViewBox(QRect(0, 0, w, h));
#endif

		P.begin(&svgg);
	} else {
		QPixmap pix(w, h);
		bitmap = pix;
		P.begin(&bitmap);
		P.setRenderHint(QPainter::Antialiasing);
	}

	MainWindow* mw = dynamic_cast<MainWindow*>(parentWidget());
	MapView* vw = new MapView(NULL);
	vw->setDocument(mw->document());

	CoordBox VP(Coord(
		angToInt(sbMinLat->value()),
		angToInt(sbMinLon->value())
		), Coord(
		angToInt(sbMaxLat->value()),
		angToInt(sbMaxLon->value())
	));

	Projection aProj;
	QRect theR(0, 0, w, h);
	vw->setGeometry(theR);
	vw->setViewport(VP, theR);
	P.setClipping(true);
	P.setClipRect(theR);

#ifndef Q_OS_SYMBIAN
	QApplication::setOverrideCursor(Qt::BusyCursor);
#endif
	QProgressDialog progress(tr("Working. Please Wait..."), tr("Cancel"), 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setCancelButton(NULL);
	progress.show();

	QApplication::processEvents();

	double oldTileToRegionThreshold = M_PREFS->getTileToRegionThreshold();
	M_PREFS->setTileToRegionThreshold(360.0);

	vw->invalidate(true, false);
	vw->buildFeatureSet();
	vw->drawBackground(P, aProj);
	vw->drawFeatures(P, aProj);

	M_PREFS->setTileToRegionThreshold(oldTileToRegionThreshold);
	P.end();

	delete vw;

	progress.reset();
#ifndef Q_OS_SYMBIAN
	QApplication::restoreOverrideCursor();
#endif

	if (rbSVG->isChecked()) {
		PictureViewerDialog vwDlg(tr("Native rendering"), QDir::tempPath()+"/tmp.svg", this);
		vwDlg.exec();
	} else {
		PictureViewerDialog vwDlg(tr("Native rendering"), bitmap, this);
		vwDlg.exec();
	}
}

void NativeRenderDialog::calcRatio()
{
	CoordBox theB(Coord(
		angToInt(sbMinLat->value()),
		angToInt(sbMinLon->value())
		), Coord(
		angToInt(sbMaxLat->value()),
		angToInt(sbMaxLon->value())
	));
	Projection theProj;
	//int w = sbPreviewWidth->value();
	//int h = sbPreviewHeight->value();
	ratio = (theB.latDiff() / theProj.latAnglePerM()) / (theB.lonDiff() / theProj.lonAnglePerM((theB.bottomLeft().lat() + theB.topRight().lat())/2));
}

void NativeRenderDialog::on_sbMinLat_valueChanged(double /* v */)
{
	calcRatio();
}

void NativeRenderDialog::on_sbMinLon_valueChanged(double /* v */)
{
	calcRatio();
}

void NativeRenderDialog::on_sbMaxLat_valueChanged(double /* v */)
{
	calcRatio();
}

void NativeRenderDialog::on_sbMaxLon_valueChanged(double /* v */)
{
	calcRatio();
}

void NativeRenderDialog::on_sbPreviewWidth_valueChanged(int v)
{
	sbPreviewHeight->blockSignals(true);
	sbPreviewHeight->setValue(int(v*ratio));
	sbPreviewHeight->blockSignals(false);
}

void NativeRenderDialog::on_sbPreviewHeight_valueChanged(int v)
{
	sbPreviewWidth->blockSignals(true);
	sbPreviewWidth->setValue(int(v/ratio));
	sbPreviewWidth->blockSignals(false);
}
