//
// C++ Implementation: WorldOsbManager
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "WorldOsbManager.h"

#include "ImportExport/ImportExportOsmBin.h"
#include "Map/DownloadOSM.h"
#include "Preferences/MerkaartorPreferences.h"


#include <QDialog>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

WorldOsbManager::WorldOsbManager(QWidget *parent)
	:QDialog(parent), WorldFile(0)
{
	setupUi(this);

	if (!M_PREFS->getLastWorldOsbDir().isEmpty()) {
		WorldDirectory->setText(M_PREFS->getLastWorldOsbDir());
		readWorld();
	}

	theProgressDialog = NULL;
	theProgressBar = pbInfo;
	theProgressLabel = lbInfo;

	theProgressLabel->setVisible(false);
	theProgressBar->setVisible(false);
}

void WorldOsbManager::on_cbShowGrid_toggled(bool checked)
{
	slippy->setShowGrid(checked);
	slippy->update();
}

void WorldOsbManager::on_buttonBox_clicked(QAbstractButton * button)
{
	if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
		DoIt();
	} else
		if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
			DoIt();
		}
	update();
}

void WorldOsbManager::on_WorldDirectoryBrowse_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this,tr("Select OSB World directory"));
	if (!s.isNull()) {
		WorldDirectory->setText(s);
		readWorld();
		M_PREFS->setLastWorldOsbDir(s);
	}
}


void WorldOsbManager::DoIt()
{
	if (WorldDirectory->text().isEmpty()) {
		QMessageBox::critical(this, tr("Invalid OSB World directory name"), 
			tr("Please provide a valid directory name."), QMessageBox::Ok);
		return;
	}

	theProgressLabel->setVisible(true);
	theProgressBar->setVisible(true);

	QHashIterator<quint32, bool> it(slippy->SelectedRegions);
	while (it.hasNext()) {
		it.next();

		if (it.value())
			if (!generateRegion(it.key()))
				QMessageBox::critical(this, tr("Region generation error"), 
					tr("Error while generating region %1").arg(it.key()), QMessageBox::Ok);
	}
	theProgressLabel->setVisible(false);
	theProgressBar->setVisible(false);

	QHashIterator<quint32, bool> itd(slippy->DeleteRegions);
	while (itd.hasNext()) {
		itd.next();

		if (itd.value())
			deleteRegion(itd.key());
	}

}

bool WorldOsbManager::deleteRegion(quint32 rg)
{
	QFile::remove((WorldDirectory->text()+ "/"+ QString::number(rg) + ".osb"));

	ImportExportOsmBin* osb = new ImportExportOsmBin(NULL);

	QDataStream ds;
	WorldFile = new QFile(WorldDirectory->text() + "/world.osb");
	ds.setDevice(WorldFile);

	WorldFile->open(QIODevice::ReadOnly);
	if (WorldFile->isOpen())
		osb->readWorld(ds);
	WorldFile->close();

	WorldFile->open(QIODevice::WriteOnly);
	osb->removeWorldRegion(rg);
	osb->writeWorld(ds);
	WorldFile->close();

	slippy->DeleteRegions[rg] = false;
	slippy->ExistingRegions[rg] = false;

	return true;
}

bool WorldOsbManager::generateRegion(quint32 rg)
{
	QString osmWebsite, osmUser, osmPwd, proxyHost;
	int proxyPort;
	bool useProxy;

	theProgressLabel->setText("");
	theProgressBar->reset();

	MapDocument * aDoc = new MapDocument();
	DrawingMapLayer* aLayer = new DrawingMapLayer("Tmp");
	aDoc->add(aLayer);

	osmWebsite = M_PREFS->getOsmWebsite();
	osmUser = M_PREFS->getOsmUser();
	osmPwd = M_PREFS->getOsmPassword();

	useProxy = M_PREFS->getProxyUse();
	proxyHost = M_PREFS->getProxyHost();
	proxyPort = M_PREFS->getProxyPort();

	if (!downloadOSM(this, osmUser, osmPwd, useProxy, proxyHost, proxyPort, rg , aDoc, aLayer)) {
		aDoc->remove(aLayer);
		delete aLayer;
		delete aDoc;
		return false;
	}

	ImportExportOsmBin* osb = new ImportExportOsmBin(aDoc);
	if (!osb->saveFile(WorldDirectory->text()+ "/"+ QString::number(rg) + ".osb")) {
		aDoc->remove(aLayer);
		delete aLayer;
		delete aDoc;
		delete osb;
		return false;
	}

	if (!osb->export_(aLayer->get(), rg)) {
		aDoc->remove(aLayer);
		delete aLayer;
		delete aDoc;
		delete osb;
		return false;
	}
	delete osb;

	osb = new ImportExportOsmBin(aDoc);

	QDataStream ds;
	WorldFile = new QFile(WorldDirectory->text() + "/world.osb");
	ds.setDevice(WorldFile);

	WorldFile->open(QIODevice::ReadOnly);
	if (WorldFile->isOpen())
		osb->readWorld(ds);
	WorldFile->close();

	WorldFile->open(QIODevice::WriteOnly);
	osb->addWorldRegion(rg);
	osb->writeWorld(ds);
	WorldFile->close();

	aDoc->remove(aLayer);
	delete aLayer;
	delete aDoc;

	slippy->ExistingRegions[rg] = true;
	slippy->SelectedRegions[rg] = false;
	slippy->DateRegions[rg] = QDateTime::currentDateTime();
	
	theProgressLabel->setText("");
	theProgressBar->reset();

	return true;
}

void WorldOsbManager::readWorld()
{
	ImportExportOsmBin theOsb(NULL);
	if (!theOsb.loadFile(WorldDirectory->text() + "/world.osb"))
		return;
	if (!theOsb.import(NULL))
		return;

	QMapIterator<qint32, quint64> it(theOsb.theRegionToc);
	while (it.hasNext()) {
		it.next();

		slippy->ExistingRegions[it.key()] = true;
		QFileInfo fi(QString("%1/%2.osb").arg(WorldDirectory->text()).arg(it.key()));
		slippy->DateRegions[it.key()] = fi.lastModified();
	}
}
