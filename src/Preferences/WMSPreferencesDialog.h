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
#include <QNetworkAccessManager>
#include <QBuffer>


#include <ui_WMSPreferencesDialog.h>
#include "MerkaartorPreferences.h"
#include "Global.h"

#include <QList>
#include <QtXml>

/**
    @author cbro <cbro@semperpax.com>
*/

class WmsUrlValidator: public QValidator
{
public:
    explicit WmsUrlValidator(QObject * parent=0)
        : QValidator(parent)
    {}

    State validate ( QString & input, int & /*pos*/ ) const
    {
        QUrl theUrl(input);

        if (!theUrl.isValid())
            return QValidator::Intermediate;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        QUrlQuery theQuery(theUrl);
#define theQuery theQuery
#else
#define theQuery theUrl
#endif

        if (theQuery.hasQueryItem("BBOX"))
            theQuery.removeQueryItem("BBOX");
        if (theQuery.hasQueryItem("REQUEST"))
            theQuery.removeQueryItem("REQUEST");
        if (theQuery.hasQueryItem("WIDTH"))
            theQuery.removeQueryItem("WIDTH");
        if (theQuery.hasQueryItem("HEIGHT"))
            theQuery.removeQueryItem("HEIGHT");
        if (theQuery.hasQueryItem("TRANSPARENT"))
            theQuery.removeQueryItem("TRANSPARENT");
        if (theQuery.hasQueryItem("tiled"))
            theQuery.removeQueryItem("tiled");

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        theUrl.setQuery(theQuery);
#endif
        input = theUrl.toString();
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
    void on_btApplyWmsServer_clicked();
    void on_btAddWmsServer_clicked();
    void on_btDelWmsServer_clicked();
    void showCapabilities();
    void on_lvWmsServers_itemSelectionChanged();
    void on_cbWmsProj_activated(const QString & text);
    void on_cbWmsStyle_activated ( int index );
    void on_cbWmsStyle_editTextChanged(const QString & newText);
    void on_cbWmsImgFormat_activated(const QString &/*text*/);
    void httpRequestFinished(QNetworkReply * reply);
    void on_buttonBox_clicked(QAbstractButton * button);
    void on_tvWmsLayers_itemChanged(QTreeWidgetItem *, int);
    void on_tvWmsLayers_currentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous );

private:
    enum WMSStatus {
       Empty,
       Caching,
       Error,
       ErrorNetwork,
       ErrorGetCapabilities,
    };

    void updateUrl();
    void loadPrefs();
    void savePrefs();
    void requestCapabilities(QUrl url);
    QTreeWidgetItem * parseLayer(const QDomElement& aLayerElem,
                                  QTreeWidgetItem* aLayerItem);
    void parseVendorSpecific(QDomElement& vendorElem);
    void parseTileSet(QDomElement& tilesetElem, WmscLayer& aLayer);

    void setTreeCheckState(Qt::CheckState state, QTreeWidgetItem *twi);
    void setStatus( WMSStatus status, QString message = QString() );

    void refreshStyles();


public:
    QList<WmsServer> theWmsServers;
    QString getSelectedServer();
    void setSelectedServer(QString theValue);

private:
    void generateWmscLayer();

    QString selectedServer;
    QNetworkAccessManager m_networkManager;
    QNetworkReply* curReply;
    int isTiled;
    QList<WmscLayer> wmscLayers;
    WmscLayer selWmscLayer;
    WmsUrlValidator wmsValid;
    QMap <QString, QStringList> srsList;
    QMap <QString, QList <QPair<QString, QString> > > styleList;
    QMap <QString, QString> styles;
    QString curLayer;
};

#endif
