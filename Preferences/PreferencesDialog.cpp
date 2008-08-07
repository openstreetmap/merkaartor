//
// C++ Implementation: PreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/PreferencesDialog.h"
#include "Preferences/WMSPreferencesDialog.h"
#include "Preferences/TMSPreferencesDialog.h"
#include "PaintStyle/EditPaintStyle.h"

#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>

PreferencesDialog::PreferencesDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	for (int i=0; i < MerkaartorPreferences::instance()->getBgTypes().size(); ++i) {
		cbMapAdapter->insertItem(i, MerkaartorPreferences::instance()->getBgTypes()[i]);
	}
	for (int i=0; i < MerkaartorPreferences::instance()->getProjectionTypes().size(); ++i) {
		cbProjection->insertItem(i, MerkaartorPreferences::instance()->getProjectionTypes()[i]);
	}
	QDir intStyles(BUILTIN_STYLES_DIR);
	for (int i=0; i < intStyles.entryList().size(); ++i) {
		cbStyles->addItem(intStyles.entryList().at(i));
	}
	loadPrefs();
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
	if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
		savePrefs();
		emit(preferencesChanged());
	} else
		if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
			savePrefs();
			emit(preferencesChanged());
			this->accept();
		}
}

void PreferencesDialog::loadPrefs()
{
	edOsmUrl->setText(MerkaartorPreferences::instance()->getOsmWebsite());
	edOsmUser->setText(MerkaartorPreferences::instance()->getOsmUser());
    edOsmPwd->setText(MerkaartorPreferences::instance()->getOsmPassword());

	edGpsPort->setText(M_PREFS->getGpsPort());

	bbUseProxy->setChecked(MerkaartorPreferences::instance()->getProxyUse());
	edProxyHost->setText(MerkaartorPreferences::instance()->getProxyHost());
	edProxyPort->setText(QString().setNum(MerkaartorPreferences::instance()->getProxyPort()));
	bbUse06Api->setChecked(MerkaartorPreferences::instance()->use06Api());

	edCacheDir->setText(MerkaartorPreferences::instance()->getCacheDir());
	sbCacheSize->setValue(MerkaartorPreferences::instance()->getCacheSize());

	cbMapAdapter->setCurrentIndex(MerkaartorPreferences::instance()->getBgType());
	switch (MerkaartorPreferences::instance()->getBgType()) {
		case Bg_Tms:
		case Bg_Wms:
			//grpWmsServers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			btAdapterSetup->setEnabled(true);
			break;
		default:
			btAdapterSetup->setEnabled(false);
	}

	QString s = MerkaartorPreferences::instance()->getDefaultStyle();
	QString cs = MerkaartorPreferences::instance()->getCustomStyle();
	CustomStyleName->setText(cs);
	if (s.startsWith(BUILTIN_STYLES_DIR)) {
		StyleBuiltin->setChecked(true);
		cbStyles->setEnabled(true);
		cbStyles->setCurrentIndex(cbStyles->findText(s.remove(QString(BUILTIN_STYLES_DIR) + "/")));
	} else {
		StyleCustom->setChecked(true);
		CustomStyleName->setEnabled(true);
		BrowseStyle->setEnabled(true);
	}
	cbDisableStyleForTracks->setChecked(MerkaartorPreferences::instance()->getDisableStyleForTracks());

	sbZoomInPerc->setValue(MerkaartorPreferences::instance()->getZoomInPerc());
	sbZoomOutPerc->setValue(MerkaartorPreferences::instance()->getZoomOutPerc());
	cbProjection->setCurrentIndex(MerkaartorPreferences::instance()->getProjectionType());

	sbAlphaLow->setValue(MerkaartorPreferences::instance()->getAlpha("Low"));
	sbAlphaHigh->setValue(MerkaartorPreferences::instance()->getAlpha("High"));
	edBgColor->setText(QVariant(MerkaartorPreferences::instance()->getBgColor()).toString());

	cbAutoSaveDoc->setChecked(MerkaartorPreferences::instance()->getAutoSaveDoc());
	cbAutoExtractTracks->setChecked(MerkaartorPreferences::instance()->getAutoExtractTracks());

	ToolList* tl = MerkaartorPreferences::instance()->getTools();
	ToolListIterator i(*tl);
	while (i.hasNext()) {
		i.next();
		Tool t(i.value().ToolName, i.value().ToolPath);
		theTools.push_back(t);
		lvTools->addItem(t.ToolName);
	}
}

void PreferencesDialog::savePrefs()
{
	MerkaartorPreferences::instance()->setUse06Api(bbUse06Api->isChecked());
	MerkaartorPreferences::instance()->setOsmWebsite(edOsmUrl->text());
	MerkaartorPreferences::instance()->setOsmUser(edOsmUser->text());
	MerkaartorPreferences::instance()->setOsmPassword(edOsmPwd->text());
	M_PREFS->setGpsPort(edGpsPort->text());

	MerkaartorPreferences::instance()->setProxyUse(bbUseProxy->isChecked());
	MerkaartorPreferences::instance()->setProxyHost(edProxyHost->text());
	MerkaartorPreferences::instance()->setProxyPort(edProxyPort->text().toInt());
	MerkaartorPreferences::instance()->setBgType((ImageBackgroundType)cbMapAdapter->currentIndex());

	MerkaartorPreferences::instance()->setCacheDir(edCacheDir->text());
	MerkaartorPreferences::instance()->setCacheSize(sbCacheSize->value());

	QString NewStyle;

	if (StyleBuiltin->isChecked())
		NewStyle = QString(BUILTIN_STYLES_DIR) + "/" + cbStyles->currentText();
	else
		NewStyle = CustomStyleName->text();

	bool PainterToInvalidate = false;
	if (NewStyle != MerkaartorPreferences::instance()->getDefaultStyle())
	{
		MerkaartorPreferences::instance()->setDefaultStyle(NewStyle);
		loadPainters(MerkaartorPreferences::instance()->getDefaultStyle());
		PainterToInvalidate = true;
	}
	if (cbDisableStyleForTracks->isChecked() != MerkaartorPreferences::instance()->getDisableStyleForTracks()) {
		MerkaartorPreferences::instance()->setDisableStyleForTracks(cbDisableStyleForTracks->isChecked());	
		PainterToInvalidate = true;
	}
	if (PainterToInvalidate) {
		for (FeatureIterator it(((MainWindow*)parent())->document()); !it.isEnd(); ++it)
		{
			it.get()->invalidatePainter();
		}
	}

	MerkaartorPreferences::instance()->setCustomStyle(CustomStyleName->text());
	MerkaartorPreferences::instance()->setZoomInPerc(sbZoomInPerc->text().toInt());
	MerkaartorPreferences::instance()->setZoomOutPerc(sbZoomOutPerc->text().toInt());
	MerkaartorPreferences::instance()->setProjectionType((ProjectionType)cbProjection->currentIndex());
	MerkaartorPreferences::instance()->getAlphaPtr()->insert("Low", sbAlphaLow->value());
	MerkaartorPreferences::instance()->getAlphaPtr()->insert("High", sbAlphaHigh->value());
	MerkaartorPreferences::instance()->setBgColor(QVariant(edBgColor->text()).value<QColor>());

	MerkaartorPreferences::instance()->setAutoSaveDoc(cbAutoSaveDoc->isChecked());
	MerkaartorPreferences::instance()->setAutoExtractTracks(cbAutoExtractTracks->isChecked());

	ToolList* tl = MerkaartorPreferences::instance()->getTools();
	tl->clear();
	for (int i = 0; i < theTools.size(); ++i) {
		Tool t(theTools[i]);
		tl->insert(theTools[i].ToolName, t);
	}

	MerkaartorPreferences::instance()->save();
}

void PreferencesDialog::on_cbMapAdapter_currentIndexChanged(int index)
{
	//grpWmsServers->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	btAdapterSetup->setEnabled(false);

	switch (index) {
		case Bg_Tms:
		case Bg_Wms:
			//grpWmsServers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			btAdapterSetup->setEnabled(true);
			break;
	}
	//layout()->activate();
	//QApplication::processEvents();
	//setFixedSize(minimumSizeHint());

}

void PreferencesDialog::on_BrowseStyle_clicked()
{
	QString s = QFileDialog::getOpenFileName(this,tr("Custom style"),"",tr("Merkaartor map style (*.mas)"));
	if (!s.isNull())
		CustomStyleName->setText(QDir::toNativeSeparators(s));
}

void PreferencesDialog::on_btAdapterSetup_clicked()
{
	WMSPreferencesDialog* WMSPref;
	TMSPreferencesDialog* TMSPref;
	switch (cbMapAdapter->currentIndex()) {
		case Bg_Wms:
			//grpWmsServers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			WMSPref = new WMSPreferencesDialog();
			if (WMSPref->exec() == QDialog::Accepted) {
			}
			break;
		case Bg_Tms:
			//grpTmsServers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			TMSPref = new TMSPreferencesDialog();
			if (TMSPref->exec() == QDialog::Accepted) {
			}
			break;
	}
}

void PreferencesDialog::on_btColorChooser_clicked()
{
	QColor color = QColorDialog::getColor(Qt::white, this);
	if (color.isValid()) {
		edBgColor->setText(QVariant(color).toString());
	}
}

/* Tools */
void PreferencesDialog::on_btAddTool_clicked(void)
{
	for (int i=0; i<theTools.size(); ++i) {
		if (theTools[i].ToolName == edToolName->text()) {
		QMessageBox::critical(this, tr("Tool already exists"), 
			tr("A tool of this name already exists.\nPlease select another name or click the <Apply> button if you want to modify the existing one"), QMessageBox::Ok);
		return;
		}
	}
	Tool t(edToolName->text(), edToolPath->text());
	theTools.push_back(t);
	lvTools->addItem(t.ToolName);

	lvTools->setCurrentRow(theTools.size() - 1);
	on_lvTools_itemClicked(lvTools->item(lvTools->currentRow()));
}

void PreferencesDialog::on_btDelTool_clicked(void)
{
	int idx = static_cast<unsigned int>(lvTools->currentRow());
	if (idx >= theTools.size())
		return;

	if (theTools[idx].ToolName == "Inkscape") {
		QMessageBox::critical(this, tr("Cannot delete preset tool"), 
			tr("Cannot delete preset tool \"%1\"").arg(theTools[idx].ToolName), QMessageBox::Ok);
		return;
	}
	theTools.erase(theTools.begin() + idx);
	delete lvTools->takeItem(idx);
	if (idx && (idx >= theTools.size()))
		--idx;
	lvTools->setCurrentRow(idx);
	on_lvTools_itemClicked(lvTools->item(lvTools->currentRow()));
}

void PreferencesDialog::on_btApplyTool_clicked(void)
{
	int idx = static_cast<unsigned int>(lvTools->currentRow());
	if (idx >= theTools.size())
		return;

	if ((theTools[idx].ToolName == "Inkscape") && edToolName->text() != theTools[idx].ToolName) {
		QMessageBox::critical(this, tr("Cannot modify preset tool name"), 
			tr("Cannot modify preset tool \"%1\"'s name").arg(theTools[idx].ToolName), QMessageBox::Ok);
		return;
	}
	Tool& t(theTools[idx]);
	t.ToolName = edToolName->text();
	t.ToolPath = edToolPath->text();

	lvTools->item(lvTools->currentRow())->setText(edToolName->text());
}

void PreferencesDialog::on_lvTools_itemClicked(QListWidgetItem* it)
{
	int idx = static_cast<unsigned int>(lvTools->row(it));
	if (idx >= theTools.size())
		return;

	Tool& t(theTools[idx]);
	edToolName->setText(t.ToolName);
	edToolPath->setText(t.ToolPath);
}

void PreferencesDialog::on_btBrowse_clicked()
{
	QString s = QFileDialog::getOpenFileName(this,tr("Select tool executable"));
	if (!s.isNull()) {
		edToolPath->setText(s);
	}
}


