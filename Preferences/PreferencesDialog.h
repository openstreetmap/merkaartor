//
// C++ Interface: PreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QWidget>
#include <QHttp>
#include <QBuffer>


#include "GeneratedFiles/ui_PreferencesDialog.h"

/**
	@author cbro <cbro@semperpax.com>
*/

class WmsServer
{
public:
	WmsServer();
	WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles);

public:
	QString WmsName;
	QString WmsAdress;
	QString WmsPath;
	QString WmsLayers;
	QString WmsProjections;
	QString WmsStyles;
};

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
	Q_OBJECT

public:
	PreferencesDialog(QWidget* parent = 0);
	~PreferencesDialog();

	void addServer(const WmsServer & srv);

public slots:
	void on_btApplyWmsServer_clicked();
	void on_btAddWmsServer_clicked();
	void on_btDelWmsServer_clicked();
	void on_btShowCapabilities_clicked();
	void on_lvWmsServers_itemClicked(QListWidgetItem* it);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);
	void httpRequestFinished(bool error);
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_cbMapAdapter_currentIndexChanged(int index);
	void on_BrowseStyle_clicked();

private:
	void loadPrefs();
	void savePrefs();
public:
	std::vector<WmsServer> theWmsServers;
	int getSelectedServer();
	void setSelectedServer(int theValue);

private:
	int selectedServer;
	int httpGetId;
	QHttp *http;
	QBuffer* buf;

};

#endif
