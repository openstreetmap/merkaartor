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
#include "IImageManager.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QDateTime>

IImageManager::IImageManager() 
	: cacheSize(0), cacheMaxSize(0)

{
}

void IImageManager::setCacheDir(const QDir& path)
{
	cacheDir = path;
	cacheSize = 0;
	if (!cacheDir.exists()) {
		cacheDir.mkpath(cacheDir.absolutePath());
	} else {
		cacheInfo = cacheDir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
		for (int i=0; i<cacheInfo.size(); i++) {
			cacheSize += cacheInfo[i].size();
		}
	}
}

void IImageManager::setCacheMaxSize(int max)
{
	cacheMaxSize = max*1024*1024;
}

bool IImageManager::useDiskCache(QString filename)
{
	// qDebug() << cacheDir.absolutePath() << filename;

	if (!cacheMaxSize)
		return false;

	if (!cacheDir.exists(filename))
		return false;

	if (M_PREFS->getOfflineMode())
		return true;
	
	int random = qrand() % 100;
	QFileInfo info(cacheDir.absolutePath() + "/" + filename);
	int days = info.lastModified().daysTo(QDateTime::currentDateTime());

	return  random < (10 * days) ? false : true;
}

void IImageManager::adaptCache()
{
	QFileInfo info;
	while (cacheSize > cacheMaxSize) {
		info = cacheInfo.takeFirst();
		cacheDir.remove(info.fileName());
		cacheSize -= info.size();
	}
}

