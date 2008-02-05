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
#include <QCompleter>


PaintStyleEditor::PaintStyleEditor(QWidget *aParent, const std::vector<FeaturePainter>& aPainters)
		: QDialog(aParent), thePainters(aPainters), FreezeUpdate(true)
{
	setupUi(this);
	BackgroundColor->setIconSize(QSize(36, 18));
	ForegroundColor->setIconSize(QSize(36, 18));
	TouchupColor->setIconSize(QSize(36, 18));
	FillColor->setIconSize(QSize(36, 18));
	for (unsigned int i = 0; i < thePainters.size(); ++i)
		PaintList->addItem(thePainters[i].userName());
	PaintList->setCurrentRow(0);
	on_PaintList_itemClicked(PaintList->item(PaintList->currentRow()));
	LowerZoomBoundary->setSpecialValueText(tr("Always"));
	UpperZoomBoundary->setSpecialValueText(tr("Always"));
	FreezeUpdate = false;

	QCompleter* completer;
	MainWindow* mw = (MainWindow *)aParent;

	completer = new QCompleter(mw->document()->getTagList(), (QObject *)this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	Key->setCompleter(completer);
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

void PaintStyleEditor::on_PaintList_itemClicked(QListWidgetItem* it)
{
	FreezeUpdate = true;
	unsigned int idx = static_cast<unsigned int>(PaintList->row(it));
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	if (FP.tagSelectors().size()) {
		Key->setText(FP.tagSelectors()[0].first);
		Value1->setText(FP.tagSelectors()[0].second);
		if ((FP.tagSelectors().size() > 1) && (Key->text() == FP.tagSelectors()[0].first))
			Value2->setText(FP.tagSelectors()[1].second);
		else
			Value2->setText("");
	} else {
		Key->setText("");
		Value1->setText("");
		Value2->setText("");
	}
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
	DrawIcon->setChecked(!FP.iconName().isEmpty());
	IconName->setText(FP.iconName());
	FreezeUpdate = false;
}

void PaintStyleEditor::on_Key_editingFinished()
{
	updatePaintList();

	MainWindow* mw = (MainWindow *)(this->parent());

	QCompleter* completer;
	QCompleter* completer2;

	QStringList sl = mw->document()->getTagValueList(Key->text());
	completer = new QCompleter(sl, (QObject *)this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	if (Value1->completer())
		delete Value1->completer();
	Value1->setCompleter(completer);

	completer2 = new QCompleter(completer);
	if (Value2->completer())
		delete Value2->completer();
	Value2->setCompleter(completer2);
}

void PaintStyleEditor::on_Value1_editingFinished()
{
	updatePaintList();
}

void PaintStyleEditor::on_Value2_editingFinished()
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

void PaintStyleEditor::on_IconName_textEdited()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.trackPointIcon(IconName->text());
}

void PaintStyleEditor::updatePaintList()
{
	unsigned int idx = static_cast<unsigned int>(PaintList->currentRow());
	if (idx >= thePainters.size())
		return;
	FeaturePainter& FP(thePainters[idx]);
	FP.clearSelectors();
	if (Value2->text().isEmpty())
		FP.selectOnTag(Key->text(), Value1->text());
	else
		FP.selectOnTag(Key->text(), Value1->text(), Value2->text());
	PaintList->currentItem()->setText(FP.userName());
}
