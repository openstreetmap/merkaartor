//
// C++ Interface: TMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TMSPREFERENCESDIALOG_H
#define TMSPREFERENCESDIALOG_H

#include <QWidget>
#include <QHttp>
#include <QBuffer>


#include <ui_TMSPreferencesDialog.h>
#include "Preferences/MerkaartorPreferences.h"

#include <QList>

/**
	@author cbro <cbro@semperpax.com>
*/

class TMSPreferencesDialog : public QDialog, public Ui::TMSPreferencesDialog
{
	Q_OBJECT

public:
	TMSPreferencesDialog(QWidget* parent = 0);
	~TMSPreferencesDialog();

	void addServer(const TmsServer & srv);

public slots:
	void on_btApplyTmsServer_clicked();
	void on_btAddTmsServer_clicked();
	void on_btDelTmsServer_clicked();
	void on_lvTmsServers_itemClicked(QListWidgetItem* it);
	void on_buttonBox_clicked(QAbstractButton * button);

private:
	void loadPrefs();
	void savePrefs();
public:
	QList<TmsServer> theTmsServers;
	QString getSelectedServer();
	void setSelectedServer(QString theValue);

private:
	QString selectedServer;
	int httpGetId;
	QBuffer* buf;

};

#endif
