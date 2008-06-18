//
// C++ Interface: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, bvh, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MERKAARTORPREFERENCES_H
#define MERKAARTORPREFERENCES_H

#include <QtCore>
#include <QtCore/QSettings>
#include <QColor>

#include "Map/Coord.h"
class QMainWindow;

#define WORLD_COORDBOX CoordBox(Coord(1.3, -1.3), Coord(-1.3, 1.3))

/**
	@author cbro <cbro@semperpax.com>
*/

enum ImageBackgroundType {
	Bg_None,
	Bg_Wms,
 	Bg_Tms
#ifdef YAHOO
 	, Bg_Yahoo
#endif
#ifdef YAHOO_ILLEGAL
	, Bg_Yahoo_illegal
#endif
#ifdef GOOGLE_ILLEGAL
	, Bg_Google_illegal
#endif
#ifdef MSLIVEMAP_ILLEGAL
	, Bg_MsVirtualEarth_illegal
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

class TmsServer
{
	public:
		TmsServer();
		TmsServer(QString Name, QString Adress, QString Path, int tileSize, int minZoom, int maxZoom);

	public:
		QString TmsName;
		QString TmsAdress;
		QString TmsPath;
		int TmsTileSize;
		int TmsMinZoom;
		int TmsMaxZoom;
};
typedef QMap<QString, TmsServer> TmsServerList;
typedef QMapIterator<QString, TmsServer> TmsServerListIterator;

class MerkaartorPreferences : public QObject
{
Q_OBJECT
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

	bool use06Api() const;
	void setUse06Api(bool b);

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

	void setSelectedWmsServer(const QString & theValue);
	QString getSelectedWmsServer() const;
	WmsServerList* getWmsServers() const;

	void setSelectedTmsServer(const QString & theValue);
	QString getSelectedTmsServer() const;
	TmsServerList* getTmsServers() const;

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

	QStringList getAlphaList() const;
	void setAlphaList();
	qreal getAlpha(QString lvl);
	QHash<QString, qreal>* getAlphaPtr();
	QColor getBgColor() const;
	void setBgColor(const QColor theValue);

	QColor getFocusColor() const;
	QColor getHoverColor() const;

	void setDownloadedVisible(bool theValue);
	bool getDownloadedVisible() const;

	/* MainWindow state */
	void saveMainWindowState(const class QMainWindow * mainWindow);
	void restoreMainWindowState(class QMainWindow * mainWindow) const;

	void setInitialPosition(const CoordBox & coordBox);
	CoordBox getInitialPosition();

	bool getDrawTileBoundary();

	/* Data */
	void setAutoSaveDoc(bool theValue);
	bool getAutoSaveDoc() const;

protected:
	bool Use06Api;
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
	void setTmsServers();
	void initialize();

private:
	QHash<QString, qreal> alpha;
	WmsServerList* theWmsServerList;
	TmsServerList* theTmsServerList;
	QSettings * Sets;
	QStringList bgTypes;
	QStringList projTypes;
	static MerkaartorPreferences* m_prefInstance;
};

#endif
