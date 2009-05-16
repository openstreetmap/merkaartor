/***************************************************************************
 *   Copyright (C) 2007 by Kai Winter   *
 *   kaiwinter@gmx.de   *
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
#ifndef IIMAGEMANAGER_H
#define IIMAGEMANAGER_H

#include <QPixmap>
#include <QDebug>
#include <QDir>

class IMapAdapter;
class LoadingRequest
{
	public:
		LoadingRequest(QString h, QString H, QString U) : hash(h), host(H), url(U) {};
		bool operator==(const LoadingRequest& LR) const {
			if (hash != LR.hash)
				return false;
			if (host != LR.host)
				return false;
			if (url != LR.url)
				return false;
			return true;
		}
	QString hash;
	QString host;
	QString url;
};

/**
	@author Kai Winter <kaiwinter@gmx.de>
*/
class IImageManager
{
	public:
		IImageManager();
		virtual ~IImageManager() {};

		//! returns a QPixmap of the asked image
		/*!
		 * If this component doesn´t have the image a network query gets started to load it.
		 * @param host the host of the image
		 * @param path the path to the image
		 * @return the pixmap of the asked image
		 */
		//QPixmap getImage(const QString& host, const QString& path);
		virtual QPixmap getImage(IMapAdapter* anAdapter, int x, int y, int z) = 0;
		virtual QPixmap getImage(IMapAdapter* anAdapter, QString url) = 0;

		//QPixmap prefetchImage(const QString& host, const QString& path);
		virtual QPixmap prefetchImage(IMapAdapter* anAdapter, int x, int y, int z) = 0;

		virtual void receivedImage(const QPixmap& pixmap, const QString& url) = 0;

		/*!
		 * This method is called by MapNetwork, after all images in its queue were loaded.
		 * The ImageManager emits a signal, which is used in MapControl to remove the zoom image.
		 * The zoom image should be removed on Tile Images with transparency.
		 * Else the zoom image stay visible behind the newly loaded tiles.
		 */
		virtual void loadingQueueEmpty() = 0;

		/*!
		 * Aborts all current loading threads.
		 * This is useful when changing the zoom-factor, though newly needed images loads faster
		 */
		virtual void abortLoading() = 0;

		void setCacheDir(const QDir& path);
		void setCacheMaxSize(int max);

	protected:

		QDir cacheDir;
		QFileInfoList cacheInfo;
		int cacheSize;
		int	cacheMaxSize;

		bool useDiskCache(QString filename);
		void adaptCache();
};

#endif
