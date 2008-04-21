//
// C++ Implementation: InfoDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "InfoDock.h"
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Map/DownloadOSM.h"

#include <QMessageBox>

InfoDock::InfoDock(MainWindow* aParent)
	: QDockWidget(aParent), Main(aParent), theText(new QTextBrowser(this))
{
	setMinimumSize(220,100);
	setWindowTitle(tr("Info"));
	setObjectName("infoDock");

	theText->setReadOnly(true);
	theText->setOpenLinks(false);
	setWidget(theText);

	connect(theText, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(on_anchorClicked(const QUrl &)));
}


InfoDock::~InfoDock()
{
}

void InfoDock::setHtml(QString html)
{
	theText->setHtml(html);
}

void InfoDock::on_anchorClicked(const QUrl & link)
{
	QHttp http;
	QString data;

	QString osmWebsite = MerkaartorPreferences::instance()->getOsmWebsite();
	QString osmUser = MerkaartorPreferences::instance()->getOsmUser();
	QString osmPwd = MerkaartorPreferences::instance()->getOsmPassword();

	bool useProxy = MerkaartorPreferences::instance()->getProxyUse();
	QString proxyHost = MerkaartorPreferences::instance()->getProxyHost();
	int proxyPort = MerkaartorPreferences::instance()->getProxyPort();

	Downloader theDownloader(osmWebsite, osmUser, osmPwd, useProxy, proxyHost, proxyPort);

	if (theDownloader.request("GET", link.path(), data)) {
		QTextBrowser* b = new QTextBrowser;
		b->append(theDownloader.content());
		b->setAttribute(Qt::WA_DeleteOnClose,true);
		b->resize(640, 480);
		b->show();
		b->raise();
	} else {
		QMessageBox::warning(Main,QApplication::translate("Downloader","Download failed"),QApplication::translate("Downloader","Unexpected http status code (%1)").arg(theDownloader.resultCode()));
	}
}


