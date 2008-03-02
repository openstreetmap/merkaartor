//
// C++ Implementation: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "MerkaartorPreferences.h"

MerkaartorPreferences* MerkaartorPreferences::m_prefInstance = 0;

MerkaartorPreferences::MerkaartorPreferences()
{
	Sets = new QSettings();

	bgTypes.insert(Bg_None, "None");
	bgTypes.insert(Bg_Wms, "WMS adapter");
	bgTypes.insert(Bg_OSM, "OSM adapter");
#ifdef yahoo_illegal
	bgTypes.insert(Bg_Yahoo_illegal, "Illegal Yahoo adapter");
#endif
}

MerkaartorPreferences::~MerkaartorPreferences()
{
	delete Sets;
}

bool MerkaartorPreferences::getRightSideDriving() const
{
	return Sets->value("roadstructure/rightsidedriving",true).toBool();
}

void MerkaartorPreferences::setRightSideDriving(bool theValue)
{
	Sets->setValue("roadstructure/rightsidedriving", theValue);
}

double MerkaartorPreferences::getDoubleRoadDistance() const
{
	return Sets->value("roadstructure/doubleroaddistance","20").toDouble();
}

void MerkaartorPreferences::setDoubleRoadDistance(double theValue)
{
	Sets->setValue("roadstructure/doubleroaddistance", theValue);
}

QString MerkaartorPreferences::getWorkingDir() const
{
	return Sets->value("general/workingdir", "").toString();
}

void MerkaartorPreferences::setWorkingDir(const QString & theValue)
{
	Sets->setValue("general/workingdir", theValue);
}

QString MerkaartorPreferences::getOsmWebsite() const
{
	return Sets->value("osm/Website", "www.openstreetmap.org").toString();
}

void MerkaartorPreferences::setOsmWebsite(const QString & theValue)
{
	Sets->setValue("osm/Website", theValue);
}

QString MerkaartorPreferences::getOsmUser() const
{
	return Sets->value("osm/User").toString();
}

void MerkaartorPreferences::setOsmUser(const QString & theValue)
{
	Sets->setValue("osm/User", theValue);
}

QString MerkaartorPreferences::getOsmPassword() const
{
	return Sets->value("osm/Password").toString();
}

void MerkaartorPreferences::setOsmPassword(const QString & theValue)
{
	Sets->setValue("osm/Password", theValue);
}

bool MerkaartorPreferences::getProxyUse() const
{
	return Sets->value("proxy/Use").toBool();
}

void MerkaartorPreferences::setProxyUse(bool theValue)
{
	Sets->setValue("proxy/Use", theValue);
}

QString MerkaartorPreferences::getProxyHost() const
{
	return Sets->value("proxy/Host").toString();
}

void MerkaartorPreferences::setProxyHost(const QString & theValue)
{
	Sets->setValue("proxy/Host", theValue);
}

int MerkaartorPreferences::getProxyPort() const
{
	return Sets->value("proxy/Port").toInt();
}

void MerkaartorPreferences::setProxyPort(int theValue)
{
	Sets->setValue("proxy/Port", theValue);
}

QStringList MerkaartorPreferences::getBookmarks() const
{
	return Sets->value("downloadosm/bookmarks").toStringList();
}

void MerkaartorPreferences::setBookmarks(const QStringList & theValue)
{
	Sets->setValue("downloadosm/bookmarks", theValue);
}

QStringList MerkaartorPreferences::getWmsServers() const
{
	return Sets->value("WSM/servers").toStringList();
}

void MerkaartorPreferences::setWmsServers(const QStringList & theValue)
{
	Sets->setValue("WSM/servers", theValue);
}

int MerkaartorPreferences::getSelectedWmsServer() const
{
	return Sets->value("WSM/selected").toInt();
}

void MerkaartorPreferences::setSelectedWmsServer(int theValue)
{
	Sets->setValue("WSM/selected", theValue);
}

bool MerkaartorPreferences::getBgVisible() const
{
	return Sets->value("backgroundImage/Visible", false).toBool();
}

void MerkaartorPreferences::setBgVisible(bool theValue)
{
	Sets->setValue("backgroundImage/Visible", theValue);
}

void MerkaartorPreferences::setBgType(ImageBackgroundType theValue)
{
	Sets->setValue("backgroundImage/Type", theValue);
}

ImageBackgroundType MerkaartorPreferences::getBgType() const
{
	return (ImageBackgroundType)Sets->value("backgroundImage/Type", 0).toInt();
}

QStringList MerkaartorPreferences::getBgTypes()
{
	return bgTypes;
}


void MerkaartorPreferences::save()
{
 	Sets->sync();
}

/* Search */
void MerkaartorPreferences::setLastMaxSearchResults(int theValue)
{
	Sets->setValue("search/LastMax", theValue);
}

int MerkaartorPreferences::getLastMaxSearchResults() const
{
	return (ImageBackgroundType)Sets->value("search/LastMax", 100).toInt();
}

void MerkaartorPreferences::setLastSearchName(const QString & theValue)
{
	Sets->setValue("search/LastName", theValue);
}

QString MerkaartorPreferences::getLastSearchName() const
{
	return Sets->value("search/LastName").toString();
}

void MerkaartorPreferences::setLastSearchKey(const QString & theValue)
{
	Sets->setValue("search/LastKey", theValue);
}

QString MerkaartorPreferences::getLastSearchKey() const
{
	return Sets->value("search/LastKey").toString();
}

void MerkaartorPreferences::setLastSearchValue(const QString & theValue)
{
	Sets->setValue("search/LastValue", theValue);
}

QString MerkaartorPreferences::getLastSearchValue() const
{
	return Sets->value("search/LastValue").toString();
}
