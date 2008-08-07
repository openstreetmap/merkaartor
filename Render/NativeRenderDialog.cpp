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

#include "Map/MapDocument.h"
#include "PaintStyle/EditPaintStyle.h"
#include "Map/Projection.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Preferences/MerkaartorPreferences.h"

#include "Utils/PictureViewerDialog.h"

#include <QProgressDialog>
#include <QPainter>

NativeRenderDialog::NativeRenderDialog(MapDocument *aDoc, const CoordBox& aCoordBox, QWidget *parent)
    :theDoc(aDoc), QDialog(parent)
{
	setupUi(this);

	buttonBox->addButton("Proceed...", QDialogButtonBox::ActionRole);
	Sets = new QSettings();
	Sets->beginGroup("NativeRenderDialog");

	cbShowScale->setCheckState((Qt::CheckState)Sets->value("cbShowScale", "1").toInt());
	cbShowGrid->setCheckState((Qt::CheckState)Sets->value("cbShowGrid", "1").toInt());
	cbShowBorders->setCheckState((Qt::CheckState)Sets->value("cbShowBorders", "1").toInt());
	cbShowLicense->setCheckState((Qt::CheckState)Sets->value("cbShowLicense", "1").toInt());

	sbMinLat->setValue(radToAng(aCoordBox.bottomLeft().lat()));
	sbMaxLat->setValue(radToAng(aCoordBox.topLeft().lat()));
	sbMinLon->setValue(radToAng(aCoordBox.topLeft().lon()));
	sbMaxLon->setValue(radToAng(aCoordBox.topRight().lon()));

	calcRatio();
	sbPreviewHeight->blockSignals(true);
	sbPreviewHeight->setValue(Sets->value("sbPreviewHeight", "600").toInt());
	sbPreviewHeight->blockSignals(false);
	sbPreviewWidth->setValue(Sets->value("sbPreviewWidth", "800").toInt());

	resize(1,1);
}

void NativeRenderDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
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
	int w = sbPreviewWidth->value();
	int h = sbPreviewHeight->value();

	QPixmap pix(w, h);
	pix.fill(MerkaartorPreferences::instance()->getBgColor());

	QPainter P(&pix);
	P.setRenderHint(QPainter::Antialiasing);

	Projection theProj;
	CoordBox VP(Coord(
		angToRad(sbMinLat->value()), 
		angToRad(sbMinLon->value())
		), Coord(
		angToRad(sbMaxLat->value()), 
		angToRad(sbMaxLon->value())
	));

	theProj.setViewport(VP, QRect(0, 0, w, h));

	QApplication::setOverrideCursor(Qt::BusyCursor);
	QProgressDialog progress("Working. Please Wait...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setCancelButton(NULL);
	progress.show();

	QApplication::processEvents();

	for (VisibleFeatureIterator i(theDoc); !i.isEnd(); ++i)
	{
		i.get()->draw(P, theProj);
	}

	EditPaintStyle EP(P, theProj);
	for (unsigned int i = 0; i < EP.size(); ++i)
	{
		PaintStyleLayer *Current = EP.get(i);
		for (VisibleFeatureIterator i(theDoc); !i.isEnd(); ++i)
		{
			if (theProj.viewport().disjunctFrom((i.get())->boundingBox())) continue;
			P.setOpacity(i.get()->layer()->getAlpha());
			if (Road * R = dynamic_cast < Road * >(i.get()))
				Current->draw(R);
			else if (TrackPoint * Pt = dynamic_cast < TrackPoint * >(i.get()))
				Current->draw(Pt);
			else if (Relation * RR = dynamic_cast < Relation * >(i.get()))
				Current->draw(RR);
		}
	}

	P.end();
	progress.reset();
	QApplication::restoreOverrideCursor();

	PictureViewerDialog vwDlg("Native rendering", pix, this);
	vwDlg.exec();
}

void NativeRenderDialog::calcRatio()
{
	CoordBox theB(Coord(
		angToRad(sbMinLat->value()), 
		angToRad(sbMinLon->value())
		), Coord(
		angToRad(sbMaxLat->value()), 
		angToRad(sbMaxLon->value())
	));
	Projection theProj;
	theProj.setViewport(theB, QRect(0, 0, 800, 600));
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
	sbPreviewHeight->setValue(v*ratio);
	sbPreviewHeight->blockSignals(false);
}

void NativeRenderDialog::on_sbPreviewHeight_valueChanged(int v)
{
	sbPreviewWidth->blockSignals(true);
	sbPreviewWidth->setValue(v/ratio);
	sbPreviewWidth->blockSignals(false);
}
