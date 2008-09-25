#include "PaintStyle/PaintStyleEditor.h"
#include "PaintStyle/PaintStyle.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"

#include <QtGui/QCheckBox>
#include <QtGui/QColorDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QIcon>
#include <QtGui/QListWidget>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QToolButton>


PaintStyleEditor::PaintStyleEditor(QWidget *aParent, const std::vector<FeaturePainter>& aPainters)
		: QDialog(aParent), thePainters(aPainters), FreezeUpdate(true)
{
	setupUi(this);
	BackgroundColor->setIconSize(QSize(36, 18));
	ForegroundColor->setIconSize(QSize(36, 18));
	TouchupColor->setIconSize(QSize(36, 18));
	FillColor->setIconSize(QSize(36, 18));
	LabelColor->setIconSize(QSize(36, 18));
	LabelBackgroundlColor->setIconSize(QSize(36, 18));
	for (unsigned int i = 0; i < thePainters.size(); ++i)
		PaintList->addItem(thePainters[i].userName());
	PaintList->setCurrentRow(0);
	LowerZoomBoundary->setSpecialValueText(tr("Always"));
	UpperZoomBoundary->setSpecialValueText(tr("Always"));

	on_PaintList_itemClicked(PaintList->item(PaintList->currentRow()));

	FreezeUpdate = false;

	resize(1, 1);
}

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

void PaintStyleEditor::on_AddButton_clicked()
{
	thePainters.push_back(FeaturePainter());
	PaintList->addItem(thePainters[thePainters.size()-1].userName());
	PaintList->setCurrentRow(thePainters.size() - 1);
	on_PaintList_itemClicked(PaintList->item(PaintList->currentRow()));
}

void PaintStyleEditor::on_DuplicateButton_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	std::vector<FeaturePainter>::iterator theIterator = thePainters.begin();
	thePainters.insert(thePainters.begin() + idx, FeaturePainter(thePainters[idx]));
	idx++;
	PaintList->insertItem(idx, thePainters[idx].userName());
	PaintList->setCurrentRow(idx);
	on_PaintList_itemClicked(PaintList->item(PaintList->currentRow()));
}

void PaintStyleEditor::on_RemoveButton_clicked()
{
	FreezeUpdate = true;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters.erase(thePainters.begin() + idx);
	delete PaintList->takeItem(idx);
	if (idx && (idx >= thePainters.size()))
		--idx;
	PaintList->setCurrentRow(idx);
	on_PaintList_itemClicked(PaintList->item(PaintList->currentRow()));
}

void PaintStyleEditor::on_btUp_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx <= 0)
		return;
	FeaturePainter fp = thePainters[idx-1];
	thePainters[idx-1] = thePainters[idx];
	thePainters[idx] = fp;
	PaintList->item(idx-1)->setText(thePainters[idx-1].userName());
	PaintList->item(idx)->setText(thePainters[idx].userName());
	PaintList->setCurrentRow(idx-1);
}

void PaintStyleEditor::on_btDown_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter fp = thePainters[idx+1];
	thePainters[idx+1] = thePainters[idx];
	thePainters[idx] = fp;
	PaintList->item(idx+1)->setText(thePainters[idx+1].userName());
	PaintList->item(idx)->setText(thePainters[idx].userName());
	PaintList->setCurrentRow(idx+1);
}

void PaintStyleEditor::on_PaintList_itemClicked(QListWidgetItem* it)
{
	FreezeUpdate = true;
	unsigned int idx = static_cast<unsigned int>(PaintList->row(it));
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	TagSelection->setText(FP.userName());
	if (FP.zoomBoundaries().first == 0)
		LowerZoomBoundary->setValue(0);
	else
		LowerZoomBoundary->setValue(1 / FP.zoomBoundaries().first);
	if ((FP.zoomBoundaries().second == 0) || (FP.zoomBoundaries().second > 10e5))
		UpperZoomBoundary->setValue(0);
	else
		UpperZoomBoundary->setValue(1 / FP.zoomBoundaries().second);
	DrawBackground->setChecked(FP.backgroundBoundary().Draw);
	ProportionalBackground->setValue(FP.backgroundBoundary().Proportional);
	FixedBackground->setValue(FP.backgroundBoundary().Fixed);
	makeBoundaryIcon(BackgroundColor, FP.backgroundBoundary().Color);
	DrawForeground->setChecked(FP.foregroundBoundary().Draw);
	ProportionalForeground->setValue(FP.foregroundBoundary().Proportional);
	FixedForeground->setValue(FP.foregroundBoundary().Fixed);
	makeBoundaryIcon(ForegroundColor, FP.foregroundBoundary().Color);
	ForegroundDashed->setChecked(FP.foregroundBoundary().Dashed);
	ForegroundDashOn->setValue(FP.foregroundBoundary().DashOn);
	ForegroundDashOff->setValue(FP.foregroundBoundary().DashOff);
	DrawTouchup->setChecked(FP.touchupBoundary().Draw);
	ProportionalTouchup->setValue(FP.touchupBoundary().Proportional);
	FixedTouchup->setValue(FP.touchupBoundary().Fixed);
	makeBoundaryIcon(TouchupColor, FP.touchupBoundary().Color);
	TouchupDashed->setChecked(FP.touchupBoundary().Dashed);
	TouchupDashOn->setValue(FP.touchupBoundary().DashOn);
	TouchupDashOff->setValue(FP.touchupBoundary().DashOff);
	DrawFill->setChecked(FP.fillColor().isValid());
	makeBoundaryIcon(FillColor, FP.fillColor());
	DrawIcon->setChecked(FP.icon().Draw);
	IconName->setText(FP.icon().Name);
	ProportionalIcon->setValue(FP.icon().Proportional);
	FixedIcon->setValue(FP.icon().Fixed);
	DrawLabel->setChecked(FP.labelBoundary().Draw);
	makeBoundaryIcon(LabelColor, FP.labelBoundary().Color);
	ProportionalLabel->setValue(FP.labelBoundary().Proportional);
	FixedLabel->setValue(FP.labelBoundary().Fixed);
	DrawLabelBackground->setChecked(FP.labelBackgroundColor().isValid());
	makeBoundaryIcon(LabelBackgroundlColor, FP.labelBackgroundColor());
	LabelFont->setCurrentFont(FP.getLabelFont());
	LabelTag->setText(FP.getLabelTag());
	LabelBackgroundTag->setText(FP.getLabelBackgroundTag());
	LabelHalo->setChecked(FP.getLabelHalo());
	
	FreezeUpdate = false;
}

void PaintStyleEditor::on_TagSelection_editingFinished()
{
	updatePaintList();
}

void PaintStyleEditor::on_LowerZoomBoundary_valueChanged()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	std::pair<double, double> Result(0, 0);
	if (LowerZoomBoundary->value() > 10e-6)
		Result.first = 1 / LowerZoomBoundary->value();
	if (UpperZoomBoundary->value() > 10e-6)
		Result.second = 1 / UpperZoomBoundary->value();
	else
		Result.second = 10e6;
	FP.zoomBoundary(Result.first, Result.second);
}

void PaintStyleEditor::on_UpperZoomBoundary_valueChanged()
{
	on_LowerZoomBoundary_valueChanged();
}

void PaintStyleEditor::on_DrawBackground_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].backgroundActive(b);
}

void PaintStyleEditor::on_BackgroundColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.backgroundBoundary().Color.rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(BackgroundColor, QColor::fromRgba(rgb));
		FP.background(QColor::fromRgba(rgb), ProportionalBackground->value(), FixedBackground->value());
	}
}

void PaintStyleEditor::on_ProportionalBackground_valueChanged()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.background(FP.backgroundBoundary().Color, ProportionalBackground->value(), FixedBackground->value());
}

void PaintStyleEditor::on_FixedBackground_valueChanged()
{
	on_ProportionalBackground_valueChanged();
}

void PaintStyleEditor::on_DrawForeground_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].foregroundActive(b);
}


void PaintStyleEditor::on_ForegroundColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.foregroundBoundary().Color.rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(ForegroundColor, QColor::fromRgba(rgb));
		FP.foreground(QColor::fromRgba(rgb), ProportionalForeground->value(), FixedForeground->value());
	}
}

void PaintStyleEditor::on_ProportionalForeground_valueChanged()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.foreground(FP.foregroundBoundary().Color, ProportionalForeground->value(), FixedForeground->value());
}

void PaintStyleEditor::on_FixedForeground_valueChanged()
{
	on_ProportionalForeground_valueChanged();
}


void PaintStyleEditor::on_ForegroundDashed_clicked()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	if (ForegroundDashed->isChecked())
		FP.foregroundDash(ForegroundDashOn->value(), ForegroundDashOff->value());
	else
		FP.clearForegroundDash();
}

void PaintStyleEditor::on_ForegroundDashOff_valueChanged()
{
	on_ForegroundDashed_clicked();
}

void PaintStyleEditor::on_ForegroundDashOn_valueChanged()
{
	on_ForegroundDashed_clicked();
}

void PaintStyleEditor::on_DrawTouchup_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].touchupActive(b);
}

void PaintStyleEditor::on_TouchupColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.touchupBoundary().Color.rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(TouchupColor, QColor::fromRgba(rgb));
		FP.touchup(QColor::fromRgba(rgb), ProportionalTouchup->value(), FixedTouchup->value());
	}
}

void PaintStyleEditor::on_ProportionalTouchup_valueChanged()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.touchup(FP.touchupBoundary().Color, ProportionalTouchup->value(), FixedTouchup->value());
}

void PaintStyleEditor::on_FixedTouchup_valueChanged()
{
	on_ProportionalTouchup_valueChanged();
}


void PaintStyleEditor::on_TouchupDashed_clicked()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	if (TouchupDashed->isChecked())
		FP.touchupDash(TouchupDashOn->value(), TouchupDashOff->value());
	else
		FP.clearTouchupDash();
}

void PaintStyleEditor::on_TouchupDashOff_valueChanged()
{
	on_TouchupDashed_clicked();
}

void PaintStyleEditor::on_TouchupDashOn_valueChanged()
{
	on_TouchupDashed_clicked();
}

void PaintStyleEditor::on_DrawFill_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].fillActive(b);
}

void PaintStyleEditor::on_FillColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.fillColor().rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(FillColor, QColor::fromRgba(rgb));
		FP.foregroundFill(QColor::fromRgba(rgb));
	}
}

void PaintStyleEditor::on_DrawIcon_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].iconActive(b);
}

void PaintStyleEditor::on_IconName_textEdited()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.setIcon(IconName->text(), ProportionalIcon->value(), FixedIcon->value());
}

void PaintStyleEditor::on_ProportionalIcon_valueChanged()
{
	on_IconName_textEdited();
}

void PaintStyleEditor::on_FixedIcon_valueChanged()
{
	on_IconName_textEdited();
}

void PaintStyleEditor::on_DrawLabel_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].labelActive(b);
}

void PaintStyleEditor::on_LabelHalo_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].labelHalo(b);
}

void PaintStyleEditor::on_LabelTag_textEdited()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].labelTag(LabelTag->text());
}

void PaintStyleEditor::on_LabelBackgroundTag_textEdited()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].labelBackgroundTag(LabelBackgroundTag->text());
}

void PaintStyleEditor::on_LabelColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.labelBoundary().Color.rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(LabelColor, QColor::fromRgba(rgb));
		FP.label(QColor::fromRgba(rgb), ProportionalLabel->value(), FixedLabel->value());
	}
}

void PaintStyleEditor::on_ProportionalLabel_valueChanged()
{
	if (FreezeUpdate)
		return;
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.label(FP.labelBoundary().Color, ProportionalLabel->value(), FixedLabel->value());
}

void PaintStyleEditor::on_FixedLabel_valueChanged()
{
	on_ProportionalLabel_valueChanged();
}

void PaintStyleEditor::on_DrawLabelBackground_clicked(bool b)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].labelBackgroundActive(b);
}

void PaintStyleEditor::on_LabelBackgroundlColor_clicked()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	bool OK = false;
	QRgb rgb = QColorDialog::getRgba(FP.labelBackgroundColor().rgba(), &OK, this);
	if (OK) {
		makeBoundaryIcon(LabelBackgroundlColor, QColor::fromRgba(rgb));
		FP.labelBackground(QColor::fromRgba(rgb));
	}
}

void PaintStyleEditor::on_LabelFont_currentFontChanged(const QFont & font)
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	thePainters[idx].setLabelFont(font.toString());
}

void PaintStyleEditor::updatePaintList()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.setSelector(TagSelection->text());
	PaintList->currentItem()->setText(FP.userName());
}

void PaintStyleEditor::on_buttonBox_clicked(QAbstractButton * button)
{
	if (buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
		emit(stylesApplied(&thePainters));
	}
}

