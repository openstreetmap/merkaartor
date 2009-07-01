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
#include "Maps/DownloadOSM.h"
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

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QUrl u;
	if (!M_PREFS->getWorldOsbUri().isEmpty()) {
		//u = QUrl(M_PREFS->getWorldOsbUri());
		//if (!u.isValid()) {
		//	u = QUrl::fromLocalFile(M_PREFS->getWorldOsbUri());
		//	if (!u.isValid()) {
		//		u = QUrl();
		//	}
		//}

		WorldURI->setText(M_PREFS->getWorldOsbUri());
		readWorld();
	}

	cbAutoload->setChecked(M_PREFS->getWorldOsbAutoload());
	cbAutoshow->setChecked(M_PREFS->getWorldOsbAutoshow());

	theProgressDialog = NULL;
	theProgressBar = pbInfo;
	theProgressLabel = lbInfo;

	theProgressLabel->setVisible(false);
	theProgressBar->setVisible(false);
}

WorldOsbManager::~WorldOsbManager()
{
	M_PREFS->setWorldOsbAutoload(cbAutoload->isChecked());
	M_PREFS->setWorldOsbAutoshow(cbAutoshow->isChecked());
}

void WorldOsbManager::setViewport(const QRectF& theViewport)
{
	slippy->setViewportArea(theViewport);
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
		//QUrl u = QUrl::fromLocalFile(s);
		//WorldURI->setText(u.toString());
		WorldURI->setText(s);
		readWorld();
		//M_PREFS->setWorldOsbUri(u.toString());
		M_PREFS->setWorldOsbUri(s);
	}
}

void WorldOsbManager::DoIt()
{
	if (WorldURI->text().isEmpty()) {
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
	QFile::remove((WorldURI->text()+ "/"+ QString::number(rg) + ".osb"));

	ImportExportOsmBin* osb = new ImportExportOsmBin(NULL);

	QDataStream ds;
	WorldFile = new QFile(WorldURI->text() + "/world.osb");
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
	QString osmWebsite, osmUser, osmPwd;

	theProgressLabel->setText("");
	theProgressBar->reset();

	MapDocument * aDoc = new MapDocument();
	DrawingMapLayer* aLayer = new DrawingMapLayer("Tmp");
	aDoc->add(aLayer);

	osmWebsite = M_PREFS->getOsmWebsite();
	osmUser = M_PREFS->getOsmUser();
	osmPwd = M_PREFS->getOsmPassword();

	if (!downloadOSM(this, osmUser, osmPwd, rg , aDoc, aLayer)) {
		aDoc->remove(aLayer);
		delete aLayer;
		delete aDoc;
		return false;
	}

	ImportExportOsmBin* osb = new ImportExportOsmBin(aDoc);
	if (!osb->saveFile(WorldURI->text()+ "/"+ QString::number(rg) + ".osb")) {
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
	WorldFile = new QFile(WorldURI->text() + "/world.osb");
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
	delete osb;

	slippy->ExistingRegions[rg] = true;
	slippy->SelectedRegions[rg] = false;
	slippy->DateRegions[rg] = QDateTime::currentDateTime();
	slippy->update();
	
	theProgressLabel->setText("");
	theProgressBar->reset();

	return true;
}

void WorldOsbManager::readWorld()
{
	ImportExportOsmBin theOsb(NULL);
	if (!theOsb.loadFile(WorldURI->text() + "/world.osb"))
		return;
	if (!theOsb.import(NULL))
		return;

	QMapIterator<qint32, quint64> it(theOsb.theRegionToc);
	while (it.hasNext()) {
		it.next();

		slippy->ExistingRegions[it.key()] = true;
		QFileInfo fi(QString("%1/%2.osb").arg(WorldURI->text()).arg(it.key()));
		slippy->DateRegions[it.key()] = fi.lastModified();
	}
}
