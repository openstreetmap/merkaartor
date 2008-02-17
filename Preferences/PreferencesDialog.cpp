//
// C++ Implementation: PreferencesDialog
//
// Description: 
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/PreferencesDialog.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QMessageBox>
#include <QUrl>
#include <QTextStream>
#include <QTextEdit>
#include <QComboBox>


WmsServer::WmsServer()
{
	WmsServer("New Server", "", "", "", "", "");
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles)
	: WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles)
{
	if (Name == "") {
		WmsName = "New Server";
	}
}

PreferencesDialog::PreferencesDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	for (int i=0; i < MerkaartorPreferences::instance()->getBgTypes().size(); ++i) {
		cbMapAdapter->insertItem(i, MerkaartorPreferences::instance()->getBgTypes()[i]);
	}

	loadPrefs();
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::addServer(const WmsServer & srv)
{
	theWmsServers.push_back(srv);
	lvWmsServers->addItem(theWmsServers[theWmsServers.size()-1].WmsName);
}

void PreferencesDialog::on_btApplyWmsServer_clicked(void)
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
}

void PreferencesDialog::on_btShowCapabilities_clicked(void)
{
	if ((edWmsAdr->text() == "") || (edWmsPath->text() == "")) {
		QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Adress and Path cannot be blank."), QMessageBox::Ok);
	}

	http = new QHttp(this);
	connect (http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
		this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

	QUrl url("http://" + edWmsAdr->text() + "?" + edWmsPath->text() + "request=GetCapabilities");
//	QUrl url("http://localhost/");

	QHttpRequestHeader header("GET", url.path() + url.encodedQuery());
	qDebug() << header.toString();
	char *userAgent = "Mozilla/9.876 (X11; U; Linux 2.2.12-20 i686, en) Gecko/25250101 Netscape/5.432b1";
	
	header.setValue("Host", url.host());
	header.setValue("User-Agent", userAgent);
	
	http->setHost(url.host(), url.port() == -1 ? 80 : url.port());
	
	if (MerkaartorPreferences::instance()->getProxyUse())
		http->setProxy(MerkaartorPreferences::instance()->getProxyHost(), MerkaartorPreferences::instance()->getProxyPort());

	httpGetId = http->request(header);
}

void PreferencesDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
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

void PreferencesDialog::httpRequestFinished(bool error)
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

void PreferencesDialog::on_btAddWmsServer_clicked(void)
{
	addServer(WmsServer(edWmsName->text(), edWmsAdr->text(), edWmsPath->text(), 
		edWmsLayers->text(), edWmsProj->text(), edWmsStyles->text()));
	lvWmsServers->setCurrentRow(theWmsServers.size() - 1);
	on_lvWmsServers_itemClicked(lvWmsServers->item(lvWmsServers->currentRow()));
}

void PreferencesDialog::on_btDelWmsServer_clicked(void)
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

void PreferencesDialog::on_lvWmsServers_itemClicked(QListWidgetItem* it)
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

	selectedServer = idx;
}

int PreferencesDialog::getSelectedServer()
{
	return selectedServer;
}

void PreferencesDialog::setSelectedServer(int theValue)
{
	lvWmsServers->setCurrentRow(theValue);
	on_lvWmsServers_itemClicked(lvWmsServers->item(lvWmsServers->currentRow()));
}

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
		savePrefs();
	} else
		if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
			savePrefs();
			this->accept();
		}
}

void PreferencesDialog::loadPrefs()
{
	edOsmUrl->setText(MerkaartorPreferences::instance()->getOsmWebsite());
	edOsmUser->setText(MerkaartorPreferences::instance()->getOsmUser());
    edOsmPwd->setText(MerkaartorPreferences::instance()->getOsmPassword());

	bbUseProxy->setChecked(MerkaartorPreferences::instance()->getProxyUse());
	edProxyHost->setText(MerkaartorPreferences::instance()->getProxyHost());
	edProxyPort->setText(QString().setNum(MerkaartorPreferences::instance()->getProxyPort()));

	QStringList Servers = MerkaartorPreferences::instance()->getWmsServers();
	for (int i=0; i<Servers.size(); i+=6)
		addServer(WmsServer(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5]));
	setSelectedServer(MerkaartorPreferences::instance()->getSelectedWmsServer());

	cbMapAdapter->setCurrentIndex(MerkaartorPreferences::instance()->getBgType());
	grpWmsServers->setVisible((MerkaartorPreferences::instance()->getBgType() == Bg_Wms));
}

void PreferencesDialog::savePrefs()
{
	MerkaartorPreferences::instance()->setOsmWebsite(edOsmUrl->text());
	MerkaartorPreferences::instance()->setOsmUser(edOsmUser->text());
	MerkaartorPreferences::instance()->setOsmPassword(edOsmPwd->text());
	MerkaartorPreferences::instance()->setProxyUse(bbUseProxy->isChecked());
	MerkaartorPreferences::instance()->setProxyHost(edProxyHost->text());
	MerkaartorPreferences::instance()->setProxyPort(edProxyPort->text().toInt());
	MerkaartorPreferences::instance()->setBgType((ImageBackgroundType)cbMapAdapter->currentIndex());

	QStringList Servers;
	for (unsigned int i = 0; i < theWmsServers.size(); ++i) {
		Servers.append(theWmsServers[i].WmsName);
		Servers.append(theWmsServers[i].WmsAdress);
		Servers.append(theWmsServers[i].WmsPath);
		Servers.append(theWmsServers[i].WmsLayers);
		Servers.append(theWmsServers[i].WmsProjections);
		Servers.append(theWmsServers[i].WmsStyles);
	}
	MerkaartorPreferences::instance()->setWmsServers(Servers);
	MerkaartorPreferences::instance()->setSelectedWmsServer(getSelectedServer());
}

void PreferencesDialog::on_cbMapAdapter_currentIndexChanged(int index)
{
	grpWmsServers->setVisible(false);

	switch (index) {
		case Bg_Wms:
			grpWmsServers->setVisible(true);
			break;
	}
}
