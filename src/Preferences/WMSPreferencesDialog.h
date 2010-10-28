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

#include <QList>
#include <QtXml>

/**
    @author cbro <cbro@semperpax.com>
*/

class WmsUrlValidator: public QValidator
{
public:
    State validate ( QString & input, int & /*pos*/ ) const
    {
        QUrl u(input);
        if (!u.isValid())
            return QValidator::Intermediate;

        if (u.hasEncodedQueryItem("BBOX"))
            u.removeEncodedQueryItem("BBOX");
        if (u.hasEncodedQueryItem("REQUEST"))
            u.removeEncodedQueryItem("REQUEST");
        if (u.hasEncodedQueryItem("WIDTH"))
            u.removeEncodedQueryItem("WIDTH");
        if (u.hasEncodedQueryItem("HEIGHT"))
            u.removeEncodedQueryItem("HEIGHT");
        if (u.hasEncodedQueryItem("TRANSPARENT"))
            u.removeEncodedQueryItem("TRANSPARENT");
        if (u.hasEncodedQueryItem("tiled"))
            u.removeEncodedQueryItem("tiled");

        input = u.toString();
        return QValidator::Acceptable;
    }

//    virtual void fixup ( QString & input ) const
//    {
//        QUrl u(input);
//        if (!u.isValid())
//            return;
//        if (u.hasEncodedQueryItem("BBOX")) {
//            u.removeEncodedQueryItem("BBOX");
//        }
//        input = u.toString();
//    }

};

class WMSPreferencesDialog : public QDialog, public Ui::WMSPreferencesDialog
{
    Q_OBJECT

public:
    WMSPreferencesDialog(QWidget* parent = 0);
    ~WMSPreferencesDialog();

    void addServer(const WmsServer & srv);

public slots:
    void on_edWmsUrl_textEdited(const QString & newText);
    void on_edWmsUrl_editingFinished();
    void on_edWmsStyles_textEdited(const QString & newText);
    void on_btApplyWmsServer_clicked();
    void on_btAddWmsServer_clicked();
    void on_btDelWmsServer_clicked();
    void on_btShowCapabilities_clicked();
    void on_lvWmsServers_itemSelectionChanged();
    void on_cbWmsProj_currentIndexChanged(const QString & text);
    void on_cbWmsImgFormat_currentIndexChanged(const QString &/*text*/);
    void readResponseHeader(const QHttpResponseHeader &responseHeader);
    void httpRequestFinished(bool error);
    void on_buttonBox_clicked(QAbstractButton * button);
    void on_tvWmsLayers_itemChanged(QTreeWidgetItem *, int);

private:
    void updateUrl();
    void loadPrefs();
    void savePrefs();
    void requestCapabilities(QUrl url);
    QTreeWidgetItem * parseLayers(QDomElement& aLayerElem, QTreeWidgetItem* aLayerItem);
    void parseVendorSpecific(QDomElement& vendorElem);
    void parseTileSet(QDomElement& tilesetElem, WmscLayer& aLayer);

public:
    QList<WmsServer> theWmsServers;
    QString getSelectedServer();
    void setSelectedServer(QString theValue);

private:
    void generateWmscLayer();

    QString selectedServer;
    int httpGetId;
    QHttp *http;
    QBuffer* buf;
    int isTiled;
    QList<WmscLayer> wmscLayers;
    WmscLayer selWmscLayer;
    WmsUrlValidator wmsValid;
};

#endif
