//
// C++ Implementation: TMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/TMSPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

TMSPreferencesDialog::TMSPreferencesDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	loadPrefs();
}

TMSPreferencesDialog::~TMSPreferencesDialog()
{
}

void TMSPreferencesDialog::addServer(const TmsServer & srv)
{
	theTmsServers.push_back(srv);
	lvTmsServers->addItem(srv.TmsName);
}

void TMSPreferencesDialog::on_btApplyTmsServer_clicked(void)
{
	unsigned int idx = static_cast<unsigned int>(lvTmsServers->currentRow());
	if (idx >= theTmsServers.size())
		return;

	TmsServer& WS(theTmsServers[idx]);
	WS.TmsName = edTmsName->text();
	WS.TmsAdress = edTmsAdr->text();
	WS.TmsPath = edTmsPath->text();
	WS.TmsTileSize = sbTileSize->value();
	WS.TmsMinZoom = sbMinZoom->value();
	WS.TmsMaxZoom = sbMaxZoom->value();

	lvTmsServers->currentItem()->setText(WS.TmsName);
	selectedServer = WS.TmsName;
}

void TMSPreferencesDialog::on_btShowCapabilities_clicked(void)
{
	if ((edTmsAdr->text() == "") || (edTmsPath->text() == "")) {
		QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Address and Path cannot be blank."), QMessageBox::Ok);
	}

	http = new QHttp(this);
	connect (http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
		this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

	QUrl url("http://" + edTmsAdr->text() + "?" + edTmsPath->text() + "request=GetCapabilities");
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

void TMSPreferencesDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
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

void TMSPreferencesDialog::httpRequestFinished(bool error)
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

void TMSPreferencesDialog::on_btAddTmsServer_clicked(void)
{
	addServer(TmsServer(edTmsName->text(), edTmsAdr->text(), edTmsPath->text(), sbTileSize->value(), sbMinZoom->value(), sbMaxZoom->value()));
	lvTmsServers->setCurrentRow(theTmsServers.size() - 1);
	on_lvTmsServers_itemClicked(lvTmsServers->item(lvTmsServers->currentRow()));
}

void TMSPreferencesDialog::on_btDelTmsServer_clicked(void)
{
	unsigned int idx = static_cast<unsigned int>(lvTmsServers->currentRow());
	if (idx >= theTmsServers.size())
		return;

	theTmsServers.erase(theTmsServers.begin() + idx);
	delete lvTmsServers->takeItem(idx);
	if (idx && (idx >= theTmsServers.size()))
		--idx;
	lvTmsServers->setCurrentRow(idx);
	on_lvTmsServers_itemClicked(lvTmsServers->item(lvTmsServers->currentRow()));
}

void TMSPreferencesDialog::on_lvTmsServers_itemClicked(QListWidgetItem* it)
{
	unsigned int idx = static_cast<unsigned int>(lvTmsServers->row(it));
	if (idx >= theTmsServers.size())
		return;

	TmsServer& WS(theTmsServers[idx]);
	edTmsName->setText(WS.TmsName);
	edTmsAdr->setText(WS.TmsAdress);
	edTmsPath->setText(WS.TmsPath);
	sbTileSize->setValue(WS.TmsTileSize);
	sbMinZoom->setValue(WS.TmsMinZoom);
	sbMaxZoom->setValue(WS.TmsMaxZoom);

	selectedServer = WS.TmsName;
}

QString TMSPreferencesDialog::getSelectedServer()
{
	return selectedServer;
}

void TMSPreferencesDialog::setSelectedServer(QString theValue)
{
	QList<QListWidgetItem *> L = lvTmsServers->findItems(theValue, Qt::MatchExactly);
	lvTmsServers->setCurrentItem(L[0]);
	on_lvTmsServers_itemClicked(L[0]);
}

void TMSPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
		savePrefs();
	} else
		if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
			savePrefs();
			this->accept();
		}
}

void TMSPreferencesDialog::loadPrefs()
{
	TmsServerList L = MerkaartorPreferences::instance()->getTmsServers();
	TmsServerListIterator i(L);
	while (i.hasNext()) {
		i.next();
		addServer(i.value());
	}
	setSelectedServer(MerkaartorPreferences::instance()->getSelectedTmsServer());
}

void TMSPreferencesDialog::savePrefs()
{
	TmsServerList& L = MerkaartorPreferences::instance()->getTmsServers();
	L.clear();
	for (unsigned int i = 0; i < theTmsServers.size(); ++i) {
		TmsServer S(theTmsServers[i]);
		L.insert(theTmsServers[i].TmsName, S);
	}
	MerkaartorPreferences::instance()->setSelectedTmsServer(getSelectedServer());
	MerkaartorPreferences::instance()->save();
}

