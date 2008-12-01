//
// C++ Implementation: WMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/WMSPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

WMSPreferencesDialog::WMSPreferencesDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	loadPrefs();
}

WMSPreferencesDialog::~WMSPreferencesDialog()
{
}

void WMSPreferencesDialog::addServer(const WmsServer & srv)
{
	theWmsServers.push_back(srv);
	lvWmsServers->addItem(srv.WmsName);
}

void WMSPreferencesDialog::on_btApplyWmsServer_clicked(void)
{
	unsigned int idx = static_cast<unsigned int>(lvWmsServers->currentRow());
	if (idx >= theWmsServers.size())
		return;

	WmsServer& WS(theWmsServers[idx]);
	WS.WmsName = edWmsName->text();
	WS.WmsAdress = edWmsAdr->text();
	WS.WmsPath = edWmsPath->text();
	WS.WmsLayers = edWmsLayers->text();
	WS.WmsProjections = edWmsProj->text();
	WS.WmsStyles = edWmsStyles->text();
	WS.WmsImgFormat = edWmsImgFormat->text();
}

void WMSPreferencesDialog::on_btShowCapabilities_clicked(void)
{
	if ((edWmsAdr->text() == "") || (edWmsPath->text() == "")) {
		QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Address and Path cannot be blank."), QMessageBox::Ok);
	}

	http = new QHttp(this);
	connect (http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
		this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

	QUrl url("http://" + edWmsAdr->text() + "?" + edWmsPath->text() + "request=GetCapabilities");
//	QUrl url("http://localhost/");

	QHttpRequestHeader header("GET", url.path() + url.encodedQuery());
	qDebug() << header.toString();
	const char *userAgent = "Mozilla/9.876 (X11; U; Linux 2.2.12-20 i686, en) Gecko/25250101 Netscape/5.432b1";

	header.setValue("Host", url.host());
	header.setValue("User-Agent", userAgent);

	http->setHost(url.host(), url.port() == -1 ? 80 : url.port());

	if (MerkaartorPreferences::instance()->getProxyUse())
		http->setProxy(MerkaartorPreferences::instance()->getProxyHost(), MerkaartorPreferences::instance()->getProxyPort());

	httpGetId = http->request(header);
}

void WMSPreferencesDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
	qDebug() << responseHeader.toString();
	if (responseHeader.statusCode() != 200) {
		QMessageBox::information(this, tr("Merkaartor: GetCapabilities"),
							  tr("Download failed: %1.")
							  .arg(responseHeader.reasonPhrase()));
		http->abort();
		return;
	}
}

void WMSPreferencesDialog::httpRequestFinished(bool error)
{
	if (error) {
		QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Error reading capabilities.\n") + http->errorString(), QMessageBox::Ok);
	} else {
		QVBoxLayout *mainLayout = new QVBoxLayout;
		QTextEdit* edit = new QTextEdit();
		edit->setPlainText(QString(http->readAll()));
		mainLayout->addWidget(edit);

		QDialog* dlg = new QDialog(this);
		dlg->setLayout(mainLayout);
		dlg->show();
		//delete dlg;
	}
}

void WMSPreferencesDialog::on_btAddWmsServer_clicked(void)
{
	addServer(WmsServer(edWmsName->text(), edWmsAdr->text(), edWmsPath->text(),
		edWmsLayers->text(), edWmsProj->text(), edWmsStyles->text(), edWmsImgFormat->text()));
	lvWmsServers->setCurrentRow(theWmsServers.size() - 1);
	on_lvWmsServers_itemClicked(lvWmsServers->item(lvWmsServers->currentRow()));
}

void WMSPreferencesDialog::on_btDelWmsServer_clicked(void)
{
	unsigned int idx = static_cast<unsigned int>(lvWmsServers->currentRow());
	if (idx >= theWmsServers.size())
		return;

	theWmsServers.erase(theWmsServers.begin() + idx);
	delete lvWmsServers->takeItem(idx);
	if (idx && (idx >= theWmsServers.size()))
		--idx;
	lvWmsServers->setCurrentRow(idx);
	on_lvWmsServers_itemClicked(lvWmsServers->item(lvWmsServers->currentRow()));
}

void WMSPreferencesDialog::on_lvWmsServers_itemClicked(QListWidgetItem* it)
{
	unsigned int idx = static_cast<unsigned int>(lvWmsServers->row(it));
	if (idx >= theWmsServers.size())
		return;

	WmsServer& WS(theWmsServers[idx]);
	edWmsName->setText(WS.WmsName);
	edWmsAdr->setText(WS.WmsAdress);
	edWmsPath->setText(WS.WmsPath);
	edWmsLayers->setText(WS.WmsLayers);
	edWmsProj->setText(WS.WmsProjections);
	edWmsStyles->setText(WS.WmsStyles);
	edWmsImgFormat->setText(WS.WmsImgFormat);

	selectedServer = WS.WmsName;
}

QString WMSPreferencesDialog::getSelectedServer()
{
	return selectedServer;
}

void WMSPreferencesDialog::setSelectedServer(QString theValue)
{
	QList<QListWidgetItem *> L = lvWmsServers->findItems(theValue, Qt::MatchExactly);
	lvWmsServers->setCurrentItem(L[0]);
	on_lvWmsServers_itemClicked(L[0]);
}

void WMSPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
		savePrefs();
	} else
		if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
			savePrefs();
			this->accept();
		}
}

void WMSPreferencesDialog::loadPrefs()
{
	WmsServerList L = MerkaartorPreferences::instance()->getWmsServers();
	WmsServerListIterator i(L);
	while (i.hasNext()) {
		i.next();
		addServer(i.value());
	}
	setSelectedServer(MerkaartorPreferences::instance()->getSelectedWmsServer());
}

void WMSPreferencesDialog::savePrefs()
{
	WmsServerList& L = MerkaartorPreferences::instance()->getWmsServers();
	L.clear();
	for (unsigned int i = 0; i < theWmsServers.size(); ++i) {
		WmsServer S(theWmsServers[i]);
		L.insert(theWmsServers[i].WmsName, S);
	}
	MerkaartorPreferences::instance()->setSelectedWmsServer(getSelectedServer());
	MerkaartorPreferences::instance()->save();
}

