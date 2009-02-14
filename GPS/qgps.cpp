/***************************************************************************
 *   Copyright (C) 2005 by Robin Gingras                                   *
 *   neozenkai@cox.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <Qt>
#include <QStatusBar>
#include <QSettings>
#include <QDateTime>

#include "qgps.h"
#include "qgpsdevice.h"
#include "qgpssatellitetracker.h"
#include "SatelliteStrengthView.h"

#include <iostream>

//#include "Preferences/MerkaartorPreferences.h"

QGPS::QGPS(QWidget *parent)
    : MDockAncestor(parent), gpsDevice(0)
{
	setupUi(getWidget());
	setObjectName("GPSMainWindow");

    int w = fontMetrics().width("360N 60'60\" W");
    txtLatitude->setFixedWidth(w);
    txtLongitude->setFixedWidth(w);
    txtAltitude->setFixedWidth(w);
    txtSpeed->setFixedWidth(w);
	txtNumSats->setFixedWidth(w);
	txtFixType->setFixedWidth(w);
	resetGpsStatus();

	retranslateUi();

    //loadSettings();

//    gpsDevice = new QGPSDevice(serialPort);

    //connect(actStartGps, SIGNAL(activated()), gpsDevice, SLOT(startDevice()));
    //connect(actStopGps, SIGNAL(activated()), gpsDevice, SLOT(stopDevice()));

    // The Qt documentation says that using signals and slots between
    // threaded classes is not recommended! TODO: find a better way!
}

void QGPS::setGpsDevice(QGPSDevice * aDevice)
{
	if (gpsDevice)
		disconnect(gpsDevice, SIGNAL(updateStatus()), this, SLOT(updateGpsStatus()));
	gpsDevice = aDevice;
}

void QGPS::startGps()
{
	if (gpsDevice) {
		if (isVisible())
			connect(gpsDevice, SIGNAL(updateStatus()), this, SLOT(updateGpsStatus()));
		gpsDevice->startDevice();
	}
}

void QGPS::stopGps()
{
	if (gpsDevice) {
	    gpsDevice->stopDevice();
		disconnect(gpsDevice, SIGNAL(updateStatus()), this, SLOT(updateGpsStatus()));
	}
}

void QGPS::resetGpsStatus()
{
    txtLatitude->setText("");
    txtLongitude->setText("");
    txtAltitude->setText("");
    txtSpeed->setText("");
	txtNumSats->setText("");
	txtFixType->setText(tr("Invalid"));
	
	lblFixStatus->setText(tr("No Position Fix"));
	lblFixTime->setText(tr("No UTC Time"));

	satTracker->setSatellites(std::vector<Satellite>());
}

void QGPS::updateGpsStatus()
{
    QString latCardinal, longCardinal, varCardinal;

    if(gpsDevice->latCardinal() == QGPSDevice::CardinalNorth)
        latCardinal = "N";
    else
        latCardinal = "S";

    if(gpsDevice->longCardinal() == QGPSDevice::CardinalEast)
        longCardinal = "E";
    else
        longCardinal = "W";

    if(gpsDevice->varCardinal() == QGPSDevice::CardinalEast)
        varCardinal = "E";
    else
        varCardinal = "W";

    txtLatitude->setText(
        QString("%1 %2\' %3\" %4")
            .arg(gpsDevice->latDegrees())
            .arg(gpsDevice->latMinutes())
            .arg(gpsDevice->latSeconds())
            .arg(latCardinal));

    txtLongitude->setText(
        QString("%1 %2\' %3\" %4")
            .arg(gpsDevice->longDegrees())
            .arg(gpsDevice->longMinutes())
            .arg(gpsDevice->longSeconds())
            .arg(longCardinal));

    txtAltitude->setText(
        QString("%1 %2")
            .arg(gpsDevice->altitude())
            .arg(tr("Meters")));

    txtSpeed->setText(
        QString("%1 %2")
            .arg(gpsDevice->speed())
            .arg(tr("km/h")));

	txtNumSats->setText(
        QString("%1").arg(gpsDevice->numSatellites()));

	switch (gpsDevice->fixType()) {
		case QGPSDevice::FixUnavailable:
			txtFixType->setText(tr("Unavailable"));
			break;
		case QGPSDevice::FixInvalid:
			txtFixType->setText(tr("Invalid"));
			break;
		case QGPSDevice::Fix2D:
			txtFixType->setText(tr("2D"));
			break;
		case QGPSDevice::Fix3D:
			txtFixType->setText(tr("3D"));
			break;
	}

	switch (gpsDevice->fixStatus()) {
		case QGPSDevice::StatusActive:
			lblFixStatus->setText(tr("Position Fix available"));
			break;
		case QGPSDevice::StatusVoid:
			lblFixStatus->setText(tr("No Position Fix"));
			break;
	}

	if (!gpsDevice->dateTime().isValid())
		lblFixTime->setText(tr("No UTC Time"));
	else
		lblFixTime->setText(gpsDevice->dateTime().toString() + " UTC");

	std::vector<Satellite> List;
	for(int i = 1; i < 50; i ++)
	{
		int b, x, y;
		gpsDevice->satInfo(i, b, x, y);
		if (y || b || x)
		{
			Satellite S;
			S.Azimuth = x;
			S.Elevation = b;
			S.SignalStrength = y;
			S.Id = i;
			List.push_back(S);
		}
	}
	StrengthView->setSatellites(List);
	satTracker->setSatellites(List);
    satTracker->setHeading((int)gpsDevice->heading());
}

void QGPS::showEvent ( QShowEvent * anEvent )
{
	QWidget::showEvent(anEvent);

	if (gpsDevice)
		connect(gpsDevice, SIGNAL(updateStatus()), this, SLOT(updateGpsStatus()));
}

void QGPS::hideEvent ( QHideEvent * anEvent )
{
	QWidget::hideEvent(anEvent);

	if (gpsDevice) 
		disconnect(gpsDevice, SIGNAL(updateStatus()), this, SLOT(updateGpsStatus()));
}

void QGPS::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	MDockAncestor::changeEvent(event);
}

void QGPS::retranslateUi()
{
	setWindowTitle("GPS");
	lblFixStatus->setText(tr("No Position Fix"));
	lblFixTime->setText(tr("No UTC Time"));

}
