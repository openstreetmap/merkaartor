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
#include <QFileDialog>
#include <QMessageBox>

WorldOsbManager::WorldOsbManager(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
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
}

void WorldOsbManager::on_WorldDirectoryBrowse_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this,tr("Select OSB World directory"));
	if (!s.isNull()) {
		WorldDirectory->setText(s);
	}
}


void WorldOsbManager::DoIt()
{
	if (WorldDirectory->text().isEmpty()) {
		QMessageBox::critical(this, tr("Invalid OSB World directory name"), 
			tr("Please provide a valid directory name."), QMessageBox::Ok);
		return;
	}
	QHashIterator<quint32, bool> it(slippy->SelectedRegions);
	while (it.hasNext()) {
		it.next();

		if (it.value())
			generateRegion(it.key());
	}

}

void WorldOsbManager::generateRegion(quint32 rg)
{
	QString osmWebsite, osmUser, osmPwd, proxyHost;
	int proxyPort;
	bool useProxy;

	MapDocument * aDoc = new MapDocument();
	DrawingMapLayer* aLayer = new DrawingMapLayer("Tmp");
	aDoc->add(aLayer);

	osmWebsite = M_PREFS->getOsmWebsite();
	osmUser = M_PREFS->getOsmUser();
	osmPwd = M_PREFS->getOsmPassword();

	useProxy = M_PREFS->getProxyUse();
	proxyHost = M_PREFS->getProxyHost();
	proxyPort = M_PREFS->getProxyPort();

	downloadOSM((MainWindow*) parent(), osmUser, osmPwd, useProxy, proxyHost, proxyPort, rg , aDoc, aLayer);

	ImportExportOsmBin osb(aDoc);
	if (osb.saveFile(WorldDirectory->text()+ "/"+ QString::number(rg) + ".osb")) {
		osb.export_(aLayer->get(), rg);
	}

	aDoc->remove(aLayer);
	delete aLayer;
	delete aDoc;
}
