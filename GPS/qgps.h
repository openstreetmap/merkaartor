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


#ifndef QGPS_H
#define QGPS_H

#include <QDockWidget>

#include "ui_qgpsmainwindowui.h"

class QGPSDevice;
class QLabel;
class QString;

class QGPS : public QDockWidget, public Ui_QGPSMainWindowUI
{
    Q_OBJECT
    
    public:
    
        QGPS(QWidget *parent = 0);

		void setGpsDevice(QGPSDevice * aDevice);
		QGPSDevice* getGpsDevice() { return gpsDevice; }
        void resetGpsStatus();
        
    private:
    
        QGPSDevice  *gpsDevice;      
        QString     serialPort;
        
    public slots:
    
        void updateGpsStatus();
        void startGps();
        void stopGps();

	protected:

		virtual void showEvent ( QShowEvent * anEvent );
		virtual void hideEvent ( QHideEvent * anEvent );
        
};

#endif
