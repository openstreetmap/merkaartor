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

#include "MerkaartorPreferences.h"
#include "namefinderwidget.h"
#include "OsmLink.h"

GotoDialog::GotoDialog(MapView* aView, QWidget *parent)
    :QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    theViewport = aView->viewport();
    theProjection = &aView->projection();
    theRect = aView->rect();

    theCenter = theViewport.center();
    int OsmZoom = int((log((360.0 / theViewport.latDiff())) / log(2.0)) + 1);
    OsmZoom = qMin(OsmZoom, 18);
    OsmZoom = qMax(OsmZoom, 1);

    int idx = 0;
    int selIdx = -1;
    qreal dist = 6372.795;
    qreal d;
    BookmarkList* Bookmarks = M_PREFS->getBookmarks();
    BookmarkListIterator i(*Bookmarks);
    while (i.hasNext()) {
        i.next();

        if (i.value().deleted == false) {
            coordBookmark->addItem(i.key());
            CoordBox C = i.value().Coordinates;
            if ((d = C.center().distanceFrom(theViewport.center())) < dist) {
                dist = d;
                selIdx = idx;
            }
        }
        ++idx;
    }
    coordBookmark->setCurrentIndex(selIdx);

    connect(cbCoordShowProjected, SIGNAL(toggled(bool)), SLOT(fillCoordinates()));

    searchWidget = new NameFinder::NameFinderWidget(this);
    connect(searchWidget, SIGNAL(selectionChanged()), this, SLOT(searchWidget_selectionChanged()));
    connect(searchWidget, SIGNAL(doubleClicked()), this, SLOT(searchWidget_doubleClicked()));
    connect(searchWidget, SIGNAL(done()), this, SLOT(searchWidget_done()));
    verticalLayout_4->addWidget(searchWidget);

    coordLink->setText( QString("http://www.openstreetmap.org/?lat=%1&lon=%2&zoom=%3")
        .arg(COORD2STRING(theViewport.center().y()))
        .arg(COORD2STRING(theViewport.center().x()))
        .arg(QString::number(OsmZoom))
        );
    coordOsmApi->setText( QString("http://www.openstreetmap.org/api/%1/map?bbox=%2,%3,%4,%5")
        .arg(M_PREFS->apiVersion())
        .arg(COORD2STRING(theViewport.bottomLeft().x()))
        .arg(COORD2STRING(theViewport.bottomLeft().y()))
        .arg(COORD2STRING(theViewport.topRight().x()))
        .arg(COORD2STRING(theViewport.topRight().y()))
        );
    coordOsmXApi->setText( QString("http://xapi.openstreetmap.org/api/0.5/*[bbox=%1,%2,%3,%4]")
        .arg(COORD2STRING(theViewport.bottomLeft().x()))
        .arg(COORD2STRING(theViewport.bottomLeft().y()))
        .arg(COORD2STRING(theViewport.topRight().x()))
        .arg(COORD2STRING(theViewport.topRight().y()))
        );
    fillCoordinates();
}

void GotoDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
        if (rbBookmark->isChecked()) {
            theNewViewport = M_PREFS->getBookmarks()->value(coordBookmark->currentText()).Coordinates;
        } else
        if (rbLink->isChecked()) {
            OsmLink ol(coordLink->text());
            if (!ol.isValid()) {
                QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid OSM url"),
                    QApplication::translate("GotoDialog", "The specified url is invalid!"));
                return;
            }
            theNewViewport = ol.getCoordBox();
        } else
        if (rbCoord->isChecked()) {
            QStringList tokens = coordCoord->text().split(",");
            if (tokens.size() < 4) {
                QMessageBox::warning(this, QApplication::translate("GotoDialog", "Invalid Coordinates format"),
                    QApplication::translate("GotoDialog", "Coordinates must be: '<left lon>, <bottom lat>, <right lon>, <top lat>'"));
                return;
            }
            theNewViewport = CoordBox(Coord(tokens[0].toDouble(), tokens[1].toDouble()), Coord(tokens[2].toDouble(), tokens[3].toDouble()));
            if (cbCoordShowProjected->isChecked())
                theNewViewport = theProjection->fromProjectedRectF(theNewViewport);
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
                                    tokens[1].toDouble() - tokens[3].toDouble() / 2,
                                    tokens[0].toDouble() - tokens[2].toDouble() / 2),
                                Coord(
                                    tokens[1].toDouble() + tokens[3].toDouble() / 2,
                                    tokens[0].toDouble() + tokens[2].toDouble() / 2)
                                );
            if (cbCoordShowProjected->isChecked())
                theNewViewport = theProjection->fromProjectedRectF(theNewViewport);
        }
        accept();
    }
}

void GotoDialog::searchWidget_selectionChanged()
{
//    QPointF centerPoint = searchWidget->selectedCoords();
//    int zoom = searchWidget->selectedZoom();
//    // The API doesn't like request for too large bounding boxes so reset to a default
//    if (zoom < 15)
//        zoom = 15;
//    coordLink->setText( QString("http://www.openstreetmap.org/?lat=%1&lon=%2&zoom=%3")
//        .arg(QString::number(centerPoint.x(), 'f', 4))
//        .arg(QString::number(centerPoint.y(), 'f', 4))
//                .arg(QString::number(zoom))
//        );
//    rbLink->setChecked(true);

    rbCoord->setChecked(true);
    coordCoord->setText(QString("%1, %2, %3, %4")
                        .arg(searchWidget->selectedBbox().left())
                        .arg(searchWidget->selectedBbox().top())
                        .arg(searchWidget->selectedBbox().right())
                        .arg(searchWidget->selectedBbox().bottom())
                        );

}

void GotoDialog::on_NameFinderEdit_textChanged(const QString & text)
{
    if (!text.isEmpty()) {
        searchButton->setDefault(true);
    } else {
        searchButton->setDefault(false);
        buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    }
}

void GotoDialog::on_searchButton_clicked()
{
        QString searchString = NameFinderEdit->text();
        QPointF C = theCenter;

        searchWidget->search(searchString, C);
        searchButton->setEnabled(false);
}

void GotoDialog::searchWidget_doubleClicked()
{
    buttonBox->button(QDialogButtonBox::Ok)->click();
}

void GotoDialog::searchWidget_done()
{
    searchButton->setEnabled(true);
}

void GotoDialog::changeEvent(QEvent * event)
{
        if (event->type() == QEvent::LanguageChange)
            retranslateUi(this);
}

void GotoDialog::fillCoordinates()
{
    CoordBox bbox = theViewport;
    if (cbCoordShowProjected->isChecked())
        bbox = theProjection->toProjectedRectF(theViewport, theRect);

    coordCoord->setText( QString("%1, %2, %3, %4")
        .arg(COORD2STRING(bbox.bottomLeft().x()))
        .arg(COORD2STRING(bbox.bottomLeft().y()))
        .arg(COORD2STRING(bbox.topRight().x()))
        .arg(COORD2STRING(bbox.topRight().y()))
        );
    coordSpan->setText( QString("%1, %2, %3, %4")
        .arg(COORD2STRING(bbox.center().y()))
        .arg(COORD2STRING(bbox.center().x()))
        .arg(COORD2STRING(bbox.latDiff()))
        .arg(COORD2STRING(bbox.lonDiff()))
        );
}

