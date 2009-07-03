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

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	loadPrefs();
}

TMSPreferencesDialog::~TMSPreferencesDialog()
{
}

void TMSPreferencesDialog::addServer(const TmsServer & srv)
{
	theTmsServers.push_back(srv);
	if (!srv.deleted) {
		QListWidgetItem* item = new QListWidgetItem(srv.TmsName);
		item->setData(Qt::UserRole, (int) theTmsServers.size()-1);
		lvTmsServers->addItem(item);
	}
}

void TMSPreferencesDialog::on_btApplyTmsServer_clicked(void)
{
	int idx = static_cast<int>(lvTmsServers->currentItem()->data(Qt::UserRole).toInt());
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

void TMSPreferencesDialog::on_btAddTmsServer_clicked(void)
{
	addServer(TmsServer(edTmsName->text(), edTmsAdr->text(), edTmsPath->text(), sbTileSize->value(), sbMinZoom->value(), sbMaxZoom->value()));
	lvTmsServers->setCurrentRow(lvTmsServers->count() - 1);
	on_lvTmsServers_itemSelectionChanged();
}

void TMSPreferencesDialog::on_btDelTmsServer_clicked(void)
{
	int idx = static_cast<int>(lvTmsServers->currentItem()->data(Qt::UserRole).toInt());
	if (idx >= theTmsServers.size())
		return;

	theTmsServers[idx].deleted = true;
	delete lvTmsServers->takeItem(idx);
	on_lvTmsServers_itemSelectionChanged();
}

void TMSPreferencesDialog::on_lvTmsServers_itemSelectionChanged()
{
	QListWidgetItem* it = lvTmsServers->item(lvTmsServers->currentRow());
	
	int idx = it->data(Qt::UserRole).toInt();
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
	on_lvTmsServers_itemSelectionChanged();
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
	TmsServerList* L = MerkaartorPreferences::instance()->getTmsServers();
	TmsServerListIterator i(*L);
	while (i.hasNext()) {
		i.next();
		addServer(i.value());
	}
}

void TMSPreferencesDialog::savePrefs()
{
	TmsServerList* L = MerkaartorPreferences::instance()->getTmsServers();
	L->clear();
	for (int i = 0; i < theTmsServers.size(); ++i) {
		TmsServer S(theTmsServers[i]);
		L->insert(theTmsServers[i].TmsName, S);
	}
	//MerkaartorPreferences::instance()->setSelectedTmsServer(getSelectedServer());
	M_PREFS->save();
}

