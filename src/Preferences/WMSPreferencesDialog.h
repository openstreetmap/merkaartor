//
// C++ Interface: WMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WMSPREFERENCESDIALOG_H
#define WMSPREFERENCESDIALOG_H

#include <QWidget>
#include <QHttp>
#include <QBuffer>


#include <ui_WMSPreferencesDialog.h>
#include "Preferences/MerkaartorPreferences.h"

#include <vector>

/**
	@author cbro <cbro@semperpax.com>
*/

class WMSPreferencesDialog : public QDialog, public Ui::WMSPreferencesDialog
{
	Q_OBJECT

public:
	WMSPreferencesDialog(QWidget* parent = 0);
	~WMSPreferencesDialog();

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

private:
	void loadPrefs();
	void savePrefs();
	void requestCapabilities(QUrl url);

public:
	QVector<WmsServer> theWmsServers;
	QString getSelectedServer();
	void setSelectedServer(QString theValue);

private:
	QString selectedServer;
	int httpGetId;
	QHttp *http;
	QBuffer* buf;

};

#endif
