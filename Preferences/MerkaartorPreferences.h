//
// C++ Interface: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MERKAARTORPREFERENCES_H
#define MERKAARTORPREFERENCES_H

#include <QtCore>
#include <QtCore/QSettings>

/**
	@author cbro <cbro@semperpax.com>
*/

enum ImageBackgroundType {
	Bg_None,
	Bg_Wms
#ifdef yahoo_illegal
	, Bg_Yahoo_illegal
#endif
};

class MerkaartorPreferences
{
public:
	MerkaartorPreferences();
	~MerkaartorPreferences();

	static MerkaartorPreferences* instance() {
		if (!m_prefInstance) {
			m_prefInstance = new MerkaartorPreferences;
		}

		return m_prefInstance;
	}


	void setRightSideDriving(bool theValue);
	bool getRightSideDriving() const;

	void setDoubleRoadDistance(double theValue);
	double getDoubleRoadDistance() const;

	void setWorkingDir(const QString & theValue);
	QString getWorkingDir() const;

	void setOsmWebsite(const QString & theValue);
	QString getOsmWebsite() const;

	void setOsmUser(const QString & theValue);
	QString getOsmUser() const;

	void setOsmPassword(const QString & theValue);
	QString getOsmPassword() const;

	void setProxyUse(bool theValue);
	bool getProxyUse() const;

	void setProxyHost(const QString & theValue);
	QString getProxyHost() const;

	void setBookmarks(const QStringList & theValue);
	QStringList getBookmarks() const;

	void setWmsServers(const QStringList & theValue);
	QStringList getWmsServers() const;

	void setSelectedWmsServer(int theValue);
	int getSelectedWmsServer() const;

	void setProxyPort(int theValue);
	int getProxyPort() const;

	void setBgVisible(bool theValue);
	bool getBgVisible() const;

	void setBgType(ImageBackgroundType theValue);
	ImageBackgroundType getBgType() const;

	QStringList getBgTypes();



protected:
	bool RightSideDriving;
	double DoubleRoadDistance;
	QString WorkingDir;
	QString OsmWebsite;
	QString OsmUser;
	QString OsmPassword;
	bool ProxyUse;
	QString ProxyHost;
	int ProxyPort;
	QStringList Bookmarks;

	void save();

private:
	QSettings * Sets;
	QStringList bgTypes;
	static MerkaartorPreferences* m_prefInstance;
};

#endif
