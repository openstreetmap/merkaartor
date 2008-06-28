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
#include "Preferences/MerkaartorPreferences.h"
#include "PaintStyle/EditPaintStyle.h"

#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"

#include <QFileDialog>
#include <QColorDialog>

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
	if (s == ":/Styles/Mapnik.mas")
		StyleMapnik->setChecked(true);
	else if (s== ":/Styles/Classic.mas")
		StyleClassic->setChecked(true);
	else
	{
		StyleCustom->setChecked(true);
		CustomStyleName->setEnabled(true);
		BrowseStyle->setEnabled(true);
	}

	sbZoomInPerc->setValue(MerkaartorPreferences::instance()->getZoomInPerc());
	sbZoomOutPerc->setValue(MerkaartorPreferences::instance()->getZoomOutPerc());
	cbProjection->setCurrentIndex(MerkaartorPreferences::instance()->getProjectionType());

	sbAlphaLow->setValue(MerkaartorPreferences::instance()->getAlpha("Low"));
	sbAlphaHigh->setValue(MerkaartorPreferences::instance()->getAlpha("High"));
	edBgColor->setText(QVariant(MerkaartorPreferences::instance()->getBgColor()).toString());

	cbAutoSaveDoc->setChecked(MerkaartorPreferences::instance()->getAutoSaveDoc());
}

void PreferencesDialog::savePrefs()
{
	MerkaartorPreferences::instance()->setUse06Api(bbUse06Api->isChecked());
	MerkaartorPreferences::instance()->setOsmWebsite(edOsmUrl->text());
	MerkaartorPreferences::instance()->setOsmUser(edOsmUser->text());
	MerkaartorPreferences::instance()->setOsmPassword(edOsmPwd->text());
	MerkaartorPreferences::instance()->setProxyUse(bbUseProxy->isChecked());
	MerkaartorPreferences::instance()->setProxyHost(edProxyHost->text());
	MerkaartorPreferences::instance()->setProxyPort(edProxyPort->text().toInt());
	MerkaartorPreferences::instance()->setBgType((ImageBackgroundType)cbMapAdapter->currentIndex());

	MerkaartorPreferences::instance()->setCacheDir(edCacheDir->text());
	MerkaartorPreferences::instance()->setCacheSize(sbCacheSize->value());

	QString NewStyle;

	if (StyleMapnik->isChecked())
		NewStyle = ":/Styles/Mapnik.mas";
	else if (StyleClassic->isChecked())
		NewStyle = ":/Styles/Classic.mas";
	else
		NewStyle = CustomStyleName->text();

	if (NewStyle != MerkaartorPreferences::instance()->getDefaultStyle())
	{
		MerkaartorPreferences::instance()->setDefaultStyle(NewStyle);
		loadPainters(MerkaartorPreferences::instance()->getDefaultStyle());
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
