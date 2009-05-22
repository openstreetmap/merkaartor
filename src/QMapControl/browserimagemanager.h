/***************************************************************************
 *   Copyright (C) 2008 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
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
#ifndef BROWSERIMAGEMANAGER_H
#define BROWSERIMAGEMANAGER_H

#include <QObject>
#include <QPixmap>

#include <QtNetwork/QtNetwork>
#include "qwebframe.h"
#include "qwebhistory.h"
#include "qwebhistoryinterface.h"
#include "qwebkitglobal.h"
#include "qwebpage.h"
#include "qwebpluginfactory.h"
#include "qwebsettings.h"
#include "qwebview.h"

#include <QThread>

#include "IImageManager.h"
#include "IMapAdapter.h"

/**
	@author Chris Browet <cbro@semperpax.com>
*/

class BrowserWebPage : public QWebPage
{
	friend class BrowserImageManager;

	protected:
		virtual void javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID );
		virtual void javaScriptAlert ( QWebFrame * frame, const QString & msg ) ;
		void launchRequest ( const QUrl & url );

	private:
		double tllat, tllon;
		double brlat, brlon;

		int ox, oy, sw, sh;
};

#ifdef BROWSERIMAGEMANAGER_IS_THREADED
class BrowserImageManager : public QThread, public IImageManager
#else
class BrowserImageManager : public QObject, public IImageManager
#endif
{
	Q_OBJECT;
	public:
		BrowserImageManager(QObject* parent = 0);
		BrowserImageManager(const BrowserImageManager&);
		BrowserImageManager& operator=(const BrowserImageManager&);
		~BrowserImageManager();

		//! returns a QPixmap of the asked image
		/*!
		 * If this component doesn´t have the image a network query gets started to load it.
		 * @param host the host of the image
		 * @param path the path to the image
		 * @return the pixmap of the asked image
		 */
		//QPixmap getImage(const QString& host, const QString& path);
		QPixmap getImage(IMapAdapter* anAdapter, int x, int y, int z);
		QPixmap getImage(IMapAdapter* anAdapter, QString url);

		//QPixmap prefetchImage(const QString& host, const QString& path);
		QPixmap prefetchImage(IMapAdapter* anAdapter, int x, int y, int z);

		void receivedImage(const QPixmap& pixmap, const QString& url);

		/*!
		 * This method is called by MapNetwork, after all images in its queue were loaded.
		 * The BrowserImageManager emits a signal, which is used in MapControl to remove the zoom image.
		 * The zoom image should be removed on Tile Images with transparency.
		 * Else the zoom image stay visible behind the newly loaded tiles.
		 */
		void loadingQueueEmpty();

		/*!
		 * Aborts all current loading threads.
		 * This is useful when changing the zoom-factor, though newly needed images loads faster
		 */
		void abortLoading();

	private:
		QPixmap emptyPixmap;
		QPixmap errorPixmap;
		QList<QString> prefetch;

		QQueue<LoadingRequest> loadingRequests;
		bool requestActive;
		int requestDuration;
		void launchRequest();

		static BrowserImageManager* m_BrowserImageManagerInstance;

		BrowserWebPage* page;
		QWebFrame *frame;
		QNetworkAccessManager* qnam;

	signals:
		void imageRequested();
		void imageReceived();
		void loadingFinished();

	private slots:
		void pageLoadFinished(bool);
		void slotLoadProgress(int p);

#ifdef BROWSERIMAGEMANAGER_IS_THREADED
	private slots:
		void checkRequests();

	protected:
		virtual void run();
#else
	private:
		QTimer* timeoutTimer;

	private slots:
		void timeout();

#endif // BROWSERIMAGEMANAGER_IS_THREADED
};

#endif
