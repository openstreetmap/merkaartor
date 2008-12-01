//
// C++ Implementation: GotoDialog
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "GotoDialog.h"

#include <QMessageBox>

#include "Preferences/MerkaartorPreferences.h"

#include "NameFinder/namefinderwidget.h"

GotoDialog::GotoDialog(const Projection& aProj, QWidget *parent)
	:QDialog(parent), theProjection(aProj)
{
	setupUi(this);

	CoordBox B = theProjection.viewport();
	int OsmZoom = (log((360.0 / intToAng(B.latDiff()))) / log(2.0)) + 1;
	OsmZoom = qMin(OsmZoom, 18);
	OsmZoom = qMax(OsmZoom, 1);

	int idx = 0;
	int selIdx = -1;
	double dist = 6372.795;
	double d;
	QMap<QString, CoordBox> Bookmarks = M_PREFS->getBookmarks();
	QMapIterator<QString, CoordBox> i(Bookmarks);
	while (i.hasNext()) {
		i.next();

		coordBookmark->addItem(i.key());
		CoordBox C = i.value();
		if ((d = C.center().distanceFrom(B.center())) < dist) {
			dist = d;
			selIdx = idx;
		}
		++idx;
	}
	coordBookmark->setCurrentIndex(selIdx);
	
	searchWidget = new NameFinder::NameFinderWidget(this);
	connect(searchWidget, SIGNAL(selectionChanged()), this, SLOT(searchWidget_selectionChanged()));
	verticalLayout_4->addWidget(searchWidget);

	coordOSM->setText( QString("http://www.openstreetmap.org/?lat=%1&lon=%2&zoom=%3")
		.arg(QString::number(intToAng(B.center().lat()), 'f', 4))
		.arg(QString::number(intToAng(B.center().lon()), 'f', 4))
		.arg(QString::number(OsmZoom))
		);
	coordOsmApi->setText( QString("http://www.openstreetmap.org/api/0.5/map?bbox=%1,%2,%3,%4")
		.arg(QString::number(intToAng(B.bottomLeft().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.bottomLeft().lat()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lat()), 'f', 4))
		);
	coordOsmXApi->setText( QString("http://xapi.openstreetmap.org/api/0.5/*[bbox=%1,%2,%3,%4]")
		.arg(QString::number(intToAng(B.bottomLeft().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.bottomLeft().lat()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lat()), 'f', 4))
		);
	coordCoord->setText( QString("%1, %2, %3, %4")
		.arg(QString::number(intToAng(B.bottomLeft().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.bottomLeft().lat()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.topRight().lat()), 'f', 4))
		);
	coordSpan->setText( QString("%1, %2, %3, %4")
		.arg(QString::number(intToAng(B.center().lat()), 'f', 4))
		.arg(QString::number(intToAng(B.center().lon()), 'f', 4))
		.arg(QString::number(intToAng(B.latDiff()), 'f', 4))
		.arg(QString::number(intToAng(B.lonDiff()), 'f', 4))
		);

	resize(1,1);
}

void GotoDialog::on_searchButton_clicked()
{
		QString foo = NameFinderEdit->text();
		searchWidget->search(foo);
}
void GotoDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
		if (rbBookmark->isChecked()) {
			theNewViewport = M_PREFS->getBookmarks()[coordBookmark->currentText()];
		} else
		if (rbOSM->isChecked()) {
			QUrl url = QUrl(coordOSM->text()); 
			if (!url.isValid()) {
				QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid OSM url"),
					QApplication::translate("GotoDialog", "The specified url is invalid!"));
				return;
			}
			double lat = url.queryItemValue("lat").toDouble(); 
			double lon = url.queryItemValue("lon").toDouble(); 
			if (lat == 0.0 || lon == 0.0) {
				QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid OSM url"),
					QApplication::translate("GotoDialog", "The specified url is invalid!"));
				return;
			}
			int zoom = url.queryItemValue("zoom").toInt();

			if (zoom < 1 || zoom > 18) // use default when not in bounds
				zoom = 15;

			/* term to calculate the angle from the zoom-value */
			double zoomLat = 360.0 / (double)(1 << zoom);
			double zoomLon = zoomLat / fabs(cos(angToRad(lat)));
			/* the following line is equal to the line above. (just for explanation) */
			//double zoomLon = zoomLat / aParent->view()->projection().latAnglePerM() * aParent->view()->projection().lonAnglePerM(angToRad(lat));

			/* the OSM link contains the coordinates from the middle of the visible map so we have to add and sub zoomLon/zoomLat */
			theNewViewport = CoordBox(Coord(angToInt(lat-zoomLat), angToInt(lon-zoomLon)), Coord(angToInt(lat+zoomLat), angToInt(lon+zoomLon)));
		} else
		if (rbCoord->isChecked()) {
			QStringList tokens = coordCoord->text().split(",");
			if (tokens.size() < 4) {
				QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid Coordinates format"),
					QApplication::translate("GotoDialog", "Coordinates must be: '<left lon>, <bottom lat>, <right lon>, <top lat>'"));
				return;
			}
			theNewViewport = CoordBox(Coord(angToInt(tokens[1].toDouble()), angToInt(tokens[0].toDouble())), Coord(angToInt(tokens[3].toDouble()), angToInt(tokens[2].toDouble())));
		} else
		if (rbSpan->isChecked()) {
			QStringList tokens = coordSpan->text().split(",");
			if (tokens.size() < 4) {
				QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid Coordinates format"),
					QApplication::translate("GotoDialog", "Coordinates must be: '<center lat>, <center lon>, <span lat>, <span lon>'"));
				return;
			}
			theNewViewport = CoordBox(
								Coord(
									angToInt(tokens[0].toDouble() - tokens[2].toDouble() / 2),
									angToInt(tokens[1].toDouble() - tokens[3].toDouble() / 2)),
								Coord(
									angToInt(tokens[0].toDouble() + tokens[2].toDouble() / 2),
									angToInt(tokens[1].toDouble() + tokens[3].toDouble() / 2))
								);
		}
		accept();
	}
}

void GotoDialog::searchWidget_selectionChanged()
{
	QPointF centerPoint = searchWidget->selectedCoords();
	coordOSM->setText( QString("http://www.openstreetmap.org/?lat=%1&lon=%2&zoom=%3")
		.arg(QString::number(centerPoint.x(), 'f', 4))
		.arg(QString::number(centerPoint.y(), 'f', 4))
		.arg(QString::number(15))
		);
	rbOSM->setChecked(true);
	
}


