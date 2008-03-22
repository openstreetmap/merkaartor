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

#define REVISION "1"
/**
	@author cbro <cbro@semperpax.com>
*/

enum ImageBackgroundType {
	Bg_None,
	Bg_Wms,
 	Bg_OSM
#ifdef yahoo_illegal
	, Bg_Yahoo_illegal
#endif
#ifdef google_illegal
	, Bg_Google_illegal
#endif
};

enum ProjectionType {
 	Proj_Background,
	Proj_Merkaartor
};

class WmsServer
{
	public:
		WmsServer();
		WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat);

	public:
		QString WmsName;
		QString WmsAdress;
		QString WmsPath;
		QString WmsLayers;
		QString WmsProjections;
		QString WmsStyles;
		QString WmsImgFormat;
};
typedef QMap<QString, WmsServer> WmsServerList;
typedef QMapIterator<QString, WmsServer> WmsServerListIterator;

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

	void save();

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

	WmsServerList* getWmsServers() const;

	void setSelectedWmsServer(const QString & theValue);
	QString getSelectedWmsServer() const;

	void setProxyPort(int theValue);
	int getProxyPort() const;

	void setBgVisible(bool theValue);
	bool getBgVisible() const;

	void setBgType(ImageBackgroundType theValue);
	ImageBackgroundType getBgType() const;
	QStringList getBgTypes();

	/* Tile Cache */
	void setCacheDir(const QString & theValue);
	QString getCacheDir() const;
	void setCacheSize(int theValue);
	int getCacheSize() const;

	/* Search */
	void setLastMaxSearchResults(int theValue);
	int getLastMaxSearchResults() const;
	void setLastSearchName(const QString & theValue);
	QString getLastSearchName() const;
	void setLastSearchKey(const QString & theValue);
	QString getLastSearchKey() const;
	void setLastSearchValue(const QString & theValue);
	QString getLastSearchValue() const;

	/* Visual */
	QString getDefaultStyle() const;
	void setDefaultStyle(const QString& aString);

	void setZoomInPerc(int theValue);
	int getZoomInPerc() const;
	void setZoomOutPerc(int theValue);
	int getZoomOutPerc() const;

	void setProjectionType(ProjectionType theValue);
	ProjectionType getProjectionType() const;
	QStringList getProjectionTypes();

protected:
	QString version;
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

	void setWmsServers();
	void initialize();

private:
	WmsServerList* theWmsServerList;
	QSettings * Sets;
	QStringList bgTypes;
	QStringList projTypes;
	static MerkaartorPreferences* m_prefInstance;
};

#endif
