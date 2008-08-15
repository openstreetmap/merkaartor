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
#include <QPainter>

static void makeBoundaryIcon(QToolButton* bt, QColor C)
{
	QPixmap pm(36, 18);
	pm.fill(QColor(255, 255, 255));
	QPainter p(&pm);
	p.setPen(C);
	p.setBrush(C);
	p.drawRect(0, 6, 36, 6);
	bt->setIcon(pm);
}

PreferencesDialog::PreferencesDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	for (int i=0; i < M_PREFS->getBgTypes().size(); ++i) {
		cbMapAdapter->insertItem(i, M_PREFS->getBgTypes()[i]);
	}
	for (int i=0; i < M_PREFS->getProjectionTypes().size(); ++i) {
		cbProjection->insertItem(i, M_PREFS->getProjectionTypes()[i]);
	}
	QDir intStyles(BUILTIN_STYLES_DIR);
	for (int i=0; i < intStyles.entryList().size(); ++i) {
		cbStyles->addItem(intStyles.entryList().at(i));
	}
	resize(1,1);
	QApplication::processEvents();

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
	edOsmUrl->setText(M_PREFS->getOsmWebsite());
	edOsmUser->setText(M_PREFS->getOsmUser());
    edOsmPwd->setText(M_PREFS->getOsmPassword());

	edGpsPort->setText(M_PREFS->getGpsPort());

	bbUseProxy->setChecked(M_PREFS->getProxyUse());
	edProxyHost->setText(M_PREFS->getProxyHost());
	edProxyPort->setText(QString().setNum(M_PREFS->getProxyPort()));
	bbUse06Api->setChecked(M_PREFS->use06Api());

	edCacheDir->setText(M_PREFS->getCacheDir());
	sbCacheSize->setValue(M_PREFS->getCacheSize());

	cbMapAdapter->setCurrentIndex(M_PREFS->getBgType());
	switch (M_PREFS->getBgType()) {
		case Bg_Tms:
		case Bg_Wms:
			//grpWmsServers->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			btAdapterSetup->setEnabled(true);
			break;
		default:
			btAdapterSetup->setEnabled(false);
	}

	QString s = M_PREFS->getDefaultStyle();
	QString cs = M_PREFS->getCustomStyle();
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
	cbDisableStyleForTracks->setChecked(M_PREFS->getDisableStyleForTracks());

	sbZoomInPerc->setValue(M_PREFS->getZoomInPerc());
	sbZoomOutPerc->setValue(M_PREFS->getZoomOutPerc());
	cbProjection->setCurrentIndex(M_PREFS->getProjectionType());

	sbAlphaLow->setValue(M_PREFS->getAlpha("Low"));
	sbAlphaHigh->setValue(M_PREFS->getAlpha("High"));
	BgColor = M_PREFS->getBgColor();
	FocusColor = M_PREFS->getFocusColor();
	HoverColor = M_PREFS->getHoverColor();
	RelationsColor = M_PREFS->getRelationsColor();
	makeBoundaryIcon(btBgColor, BgColor);
	makeBoundaryIcon(btHoverColor, HoverColor);
	makeBoundaryIcon(btFocusColor, FocusColor);
	makeBoundaryIcon(btRelationsColor, RelationsColor);

	cbAutoSaveDoc->setChecked(M_PREFS->getAutoSaveDoc());
	cbAutoExtractTracks->setChecked(M_PREFS->getAutoExtractTracks());

	ToolList* tl = M_PREFS->getTools();
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
	M_PREFS->setUse06Api(bbUse06Api->isChecked());
	M_PREFS->setOsmWebsite(edOsmUrl->text());
	M_PREFS->setOsmUser(edOsmUser->text());
	M_PREFS->setOsmPassword(edOsmPwd->text());
	M_PREFS->setGpsPort(edGpsPort->text());

	M_PREFS->setProxyUse(bbUseProxy->isChecked());
	M_PREFS->setProxyHost(edProxyHost->text());
	M_PREFS->setProxyPort(edProxyPort->text().toInt());
	M_PREFS->setBgType((ImageBackgroundType)cbMapAdapter->currentIndex());

	M_PREFS->setCacheDir(edCacheDir->text());
	M_PREFS->setCacheSize(sbCacheSize->value());

	QString NewStyle;

	if (StyleBuiltin->isChecked())
		NewStyle = QString(BUILTIN_STYLES_DIR) + "/" + cbStyles->currentText();
	else
		NewStyle = CustomStyleName->text();

	bool PainterToInvalidate = false;
	if (NewStyle != M_PREFS->getDefaultStyle())
	{
		M_PREFS->setDefaultStyle(NewStyle);
		loadPainters(M_PREFS->getDefaultStyle());
		PainterToInvalidate = true;
	}
	if (cbDisableStyleForTracks->isChecked() != M_PREFS->getDisableStyleForTracks()) {
		M_PREFS->setDisableStyleForTracks(cbDisableStyleForTracks->isChecked());	
		PainterToInvalidate = true;
	}
	if (PainterToInvalidate) {
		for (FeatureIterator it(((MainWindow*)parent())->document()); !it.isEnd(); ++it)
		{
			it.get()->invalidatePainter();
		}
	}

	M_PREFS->setCustomStyle(CustomStyleName->text());
	M_PREFS->setZoomInPerc(sbZoomInPerc->text().toInt());
	M_PREFS->setZoomOutPerc(sbZoomOutPerc->text().toInt());
	M_PREFS->setProjectionType((ProjectionType)cbProjection->currentIndex());
	M_PREFS->getAlphaPtr()->insert("Low", sbAlphaLow->value());
	M_PREFS->getAlphaPtr()->insert("High", sbAlphaHigh->value());
	M_PREFS->setBgColor(BgColor);
	M_PREFS->setFocusColor(FocusColor);
	M_PREFS->setHoverColor(HoverColor);
	M_PREFS->setRelationsColor(RelationsColor);

	M_PREFS->setAutoSaveDoc(cbAutoSaveDoc->isChecked());
	M_PREFS->setAutoExtractTracks(cbAutoExtractTracks->isChecked());

	ToolList* tl = M_PREFS->getTools();
	tl->clear();
	for (int i = 0; i < theTools.size(); ++i) {
		Tool t(theTools[i]);
		tl->insert(theTools[i].ToolName, t);
	}

	M_PREFS->save();
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

void PreferencesDialog::on_btBgColor_clicked()
{
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(BgColor.rgba(), &OK, this);
	if (OK) {
		BgColor = QColor::fromRgba(rgb);
		makeBoundaryIcon(btBgColor, BgColor);
	}
}

void PreferencesDialog::on_btFocusColor_clicked()
{
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FocusColor.rgba(), &OK, this);
	if (OK) {
		FocusColor = QColor::fromRgba(rgb);
		makeBoundaryIcon(btFocusColor, FocusColor);
	}
}

void PreferencesDialog::on_btHoverColor_clicked()
{
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(HoverColor.rgba(), &OK, this);
	if (OK) {
		HoverColor = QColor::fromRgba(rgb);
		makeBoundaryIcon(btHoverColor, HoverColor);
	}
}

void PreferencesDialog::on_btRelationsColor_clicked()
{
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(RelationsColor.rgba(), &OK, this);
	if (OK) {
		RelationsColor = QColor::fromRgba(rgb);
		makeBoundaryIcon(btRelationsColor, RelationsColor);
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


