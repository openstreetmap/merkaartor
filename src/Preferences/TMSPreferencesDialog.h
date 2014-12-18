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
#include <QBuffer>
#include <QNetworkAccessManager>


#include <ui_TMSPreferencesDialog.h>
#include "MerkaartorPreferences.h"

#include <QList>

class TileService
{
public:
    QString Title;
    QUrl Url;
    QString SRS;
    QString Format;
    bool Origin;
    CoordBox BBox;
    int TileSize;
    int MinZoom;
    int MaxZoom;
};
typedef QHash<QString, TileService> TileServiceList;

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
    void on_lvTmsServers_itemSelectionChanged();
    void on_lvTmsServices_itemSelectionChanged();
    void on_buttonBox_clicked(QAbstractButton * button);

    void on_btGetServices_clicked();

private:
    void loadPrefs();
    void savePrefs();

    QNetworkReply* sendRequest(QUrl url);

private slots:
    void httpRequestFinished( QNetworkReply *reply);

public:
    QList<TmsServer> theTmsServers;
    QString getSelectedServer();
    void setSelectedServer(QString theValue);

private:
    QString selectedServer;
    QNetworkAccessManager http;
    QBuffer* buf;
    TileServiceList services;

};

#endif
