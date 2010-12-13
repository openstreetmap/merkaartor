#include "Global.h"

#include "PaintStyle/PaintStyleEditor.h"
#include "PaintStyle/Painter.h"
#include "MainWindow.h"
#include "Document.h"

#include "Utils/SelectionDialog.h"

#include <QtGui/QCheckBox>
#include <QtGui/QColorDialog>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QIcon>
#include <QtGui/QListWidget>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QToolButton>

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

PaintStyleEditor::PaintStyleEditor(QWidget *aParent, const GlobalPainter& aGlobalPainter, const QList<Painter>& aPainters)
    : QDialog(aParent), theGlobalPainter(aGlobalPainter), thePainters(aPainters), FreezeUpdate(true)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    BackgroundColor->setIconSize(QSize(36, 18));
    ForegroundColor->setIconSize(QSize(36, 18));
    TouchupColor->setIconSize(QSize(36, 18));
    TrafficDirectionMarksColor->setIconSize(QSize(36, 18));;
    FillColor->setIconSize(QSize(36, 18));
    LabelColor->setIconSize(QSize(36, 18));
    LabelBackgroundlColor->setIconSize(QSize(36, 18));
    GlobalBackgroundColor->setIconSize(QSize(36, 18));
    GlobalNodesColor->setIconSize(QSize(36, 18));

    updatePaintList();

    LowerZoomBoundary->setSpecialValueText(tr("Always"));
    UpperZoomBoundary->setSpecialValueText(tr("Always"));

    DrawGlobalBackground->setChecked(theGlobalPainter.getDrawBackground());
    makeBoundaryIcon(GlobalBackgroundColor, theGlobalPainter.getBackgroundColor());
    on_PaintList_itemSelectionChanged();

    DrawGlobalNodes->setChecked(theGlobalPainter.getDrawNodes());
    makeBoundaryIcon(GlobalNodesColor, theGlobalPainter.getNodesColor());
    GlobalNodesProportional->setEnabled(theGlobalPainter.getDrawNodes());
    GlobalNodesFixed->setEnabled(theGlobalPainter.getDrawNodes());
    GlobalNodesProportional->setValue(theGlobalPainter.NodesProportional);
    GlobalNodesFixed->setValue(theGlobalPainter.NodesFixed);

    FreezeUpdate = false;

    resize(1, 1);
}

void PaintStyleEditor::updatePaintList()
{
    QListWidgetItem* it;
    QString curName;
    if (PaintList->currentItem())
        curName = PaintList->currentItem()->text();
    PaintList->clear();
    for (int i = 0; i < thePainters.size(); ++i) {
        it = new QListWidgetItem(thePainters[i].userName());
        it->setData(Qt::UserRole, i);
        if (edFilter->text().isEmpty()) {
            PaintList->addItem(it);
        } else
            if (thePainters[i].userName().contains(edFilter->text(), Qt::CaseInsensitive)) {
            it = new QListWidgetItem(thePainters[i].userName());
            it->setData(Qt::UserRole, i);
            PaintList->addItem(it);
        }
        if (!curName.isEmpty() && thePainters[i].userName().contains(curName, Qt::CaseInsensitive))
            PaintList->setCurrentItem(it);
    }
    if (curName.isEmpty())
        PaintList->setCurrentRow(0);
}

void PaintStyleEditor::on_AddButton_clicked()
{
    thePainters.push_back(FeaturePainter());
    QListWidgetItem* it = new QListWidgetItem(thePainters[thePainters.size()-1].userName());
    it->setData(Qt::UserRole, thePainters.size()-1);
    PaintList->addItem(it);
    PaintList->setCurrentItem(it);
    on_PaintList_itemSelectionChanged();
}

void PaintStyleEditor::on_DuplicateButton_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx < 0 || idx >= thePainters.size())
        return;
    //QList<FeaturePainter>::iterator theIterator = thePainters.begin();
    thePainters.insert(thePainters.begin() + idx, Painter(thePainters[idx]));
    idx++;
    it = new QListWidgetItem(thePainters[idx].userName());
    it->setData(Qt::UserRole, idx);
    PaintList->insertItem(PaintList->currentRow()+1, it);
    PaintList->setCurrentItem(it);
    on_PaintList_itemSelectionChanged();
}

void PaintStyleEditor::on_RemoveButton_clicked()
{
    FreezeUpdate = true;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx < 0 || idx >= thePainters.size())
        return;
    thePainters.erase(thePainters.begin() + idx);
    delete PaintList->takeItem(idx);
    if (idx && (idx >= thePainters.size()))
        --idx;
    PaintList->setCurrentRow(idx);
    on_PaintList_itemSelectionChanged();
}

void PaintStyleEditor::on_btUp_clicked()
{
    if (PaintList->currentRow() <= 0)
        return;

    int idx = PaintList->item(PaintList->currentRow())->data(Qt::UserRole).toInt();
    int idxup = PaintList->item(PaintList->currentRow()-1)->data(Qt::UserRole).toInt();

    Painter fp = thePainters[idxup];
    thePainters[idxup] = thePainters[idx];
    thePainters[idx] = fp;
    PaintList->item(PaintList->currentRow()-1)->setText(thePainters[idxup].userName());
    PaintList->item(PaintList->currentRow())->setText(thePainters[idx].userName());
    PaintList->setCurrentRow(PaintList->currentRow()-1);
}

void PaintStyleEditor::on_btDown_clicked()
{
    if (PaintList->currentRow() >= PaintList->count()-1)
        return;

    int idx = PaintList->item(PaintList->currentRow())->data(Qt::UserRole).toInt();
    int idxdn = PaintList->item(PaintList->currentRow()+1)->data(Qt::UserRole).toInt();

    Painter fp = thePainters[idxdn];
    thePainters[idxdn] = thePainters[idx];
    thePainters[idx] = fp;
    PaintList->item(PaintList->currentRow()+1)->setText(thePainters[idxdn].userName());
    PaintList->item(PaintList->currentRow())->setText(thePainters[idx].userName());
    PaintList->setCurrentRow(PaintList->currentRow()+1);
}

void PaintStyleEditor::on_PaintList_itemSelectionChanged()
{
    QListWidgetItem* it = PaintList->currentItem();

    int idx = -1;
    if (it)
        idx = it->data(Qt::UserRole).toInt();

    if (idx < 0) {
        FreezeUpdate = false;

        editArea->setEnabled(false);
        DuplicateButton->setEnabled(false);
        RemoveButton->setEnabled(false);
        return;
    } else {
        editArea->setEnabled(true);
        DuplicateButton->setEnabled(true);
        RemoveButton->setEnabled(true);
    }
    if (idx >= thePainters.size())
        return;

    FreezeUpdate = true;
    Painter& FP(thePainters[idx]);
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
    BackgroundInterior->setChecked(FP.getBackgroundInterior());
    BackgroundExterior->setChecked(FP.getBackgroundExterior());
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
    DrawTrafficDirectionMarks->setChecked(FP.DrawTrafficDirectionMarks);
    makeBoundaryIcon(TrafficDirectionMarksColor, FP.TrafficDirectionMarksColor);
    DrawFill->setChecked(FP.fillColor().isValid());
    makeBoundaryIcon(FillColor, FP.fillColor());
    DrawFillIcon->setChecked(FP.ForegroundFillUseIcon);;
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
    LabelArea->setChecked(FP.getLabelArea());

    updatePagesIcons();

    FreezeUpdate = false;
}

void PaintStyleEditor::updatePagesIcons()
{
    if (DrawForeground->isChecked() || DrawFill->isChecked())
        tbStyle->setItemIcon(0, QIcon(":/Icons/actions/software-update-available.png"));
    else
        tbStyle->setItemIcon(0, QIcon());

    if (DrawBackground->isChecked())
        tbStyle->setItemIcon(1, QIcon(":/Icons/actions/software-update-available.png"));
    else
        tbStyle->setItemIcon(1, QIcon());

    if (DrawTouchup->isChecked() || DrawIcon->isChecked())
        tbStyle->setItemIcon(2, QIcon(":/Icons/actions/software-update-available.png"));
    else
        tbStyle->setItemIcon(2, QIcon());

    if (DrawLabel->isChecked())
        tbStyle->setItemIcon(3, QIcon(":/Icons/actions/software-update-available.png"));
    else
        tbStyle->setItemIcon(3, QIcon());
}

void PaintStyleEditor::on_TagSelection_editingFinished()
{
    refreshPainter();
}

void PaintStyleEditor::on_LowerZoomBoundary_valueChanged()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QPair<double, double> Result(0, 0);
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

void PaintStyleEditor::on_DrawGlobalBackground_clicked(bool b)
{
    theGlobalPainter.backgroundActive(b);
}

void PaintStyleEditor::on_GlobalBackgroundColor_clicked()
{
    QColor rgb = QColorDialog::getColor(theGlobalPainter.getBackgroundColor(), this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(GlobalBackgroundColor, rgb);
        theGlobalPainter.background(rgb);
    }
}

void PaintStyleEditor::on_DrawGlobalNodes_clicked(bool b)
{
    theGlobalPainter.nodesActive(b);
}

void PaintStyleEditor::on_GlobalNodesColor_clicked()
{
    QColor rgb = QColorDialog::getColor(theGlobalPainter.getNodesColor(), this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(GlobalNodesColor, rgb);
        theGlobalPainter.nodes(rgb);
    }
}

void PaintStyleEditor::on_GlobalNodesProportional_valueChanged()
{
    theGlobalPainter.NodesProportional = GlobalNodesProportional->value();
}

void PaintStyleEditor::on_GlobalNodesFixed_valueChanged()
{
    theGlobalPainter.NodesFixed = GlobalNodesFixed->value();
}

void PaintStyleEditor::on_DrawBackground_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].backgroundActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_BackgroundColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.backgroundBoundary().Color, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(BackgroundColor, rgb);
        FP.background(rgb, ProportionalBackground->value(), FixedBackground->value());
    }
}

void PaintStyleEditor::on_ProportionalBackground_valueChanged()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    FP.background(FP.backgroundBoundary().Color, ProportionalBackground->value(), FixedBackground->value());
}

void PaintStyleEditor::on_FixedBackground_valueChanged()
{
    on_ProportionalBackground_valueChanged();
}

void PaintStyleEditor::on_DrawForeground_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].foregroundActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_BackgroundInterior_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].BackgroundInterior = b;

    if (b) {
        BackgroundExterior->blockSignals(true);
        BackgroundExterior->setChecked(false);
        thePainters[idx].BackgroundExterior = false;
        BackgroundExterior->blockSignals(false);
    }
}

void PaintStyleEditor::on_BackgroundExterior_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].BackgroundExterior = b;

    if (b) {
        BackgroundInterior->blockSignals(true);
        BackgroundInterior->setChecked(false);
        thePainters[idx].BackgroundInterior = false;
        BackgroundInterior->blockSignals(false);
    }
}

void PaintStyleEditor::on_ForegroundColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.foregroundBoundary().Color, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(ForegroundColor, rgb);
        FP.foreground(rgb, ProportionalForeground->value(), FixedForeground->value());
    }
}

void PaintStyleEditor::on_DrawFillIcon_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);

    FP.ForegroundFillUseIcon = b;
}

void PaintStyleEditor::on_ProportionalForeground_valueChanged()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
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
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
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
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].touchupActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_TouchupColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.touchupBoundary().Color, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(TouchupColor, rgb);
        FP.touchup(rgb, ProportionalTouchup->value(), FixedTouchup->value());
    }
}

void PaintStyleEditor::on_DrawTrafficDirectionMarks_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].drawTrafficDirectionMarks(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_TrafficDirectionMarksColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.TrafficDirectionMarksColor, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(TrafficDirectionMarksColor, rgb);
        FP.TrafficDirectionMarksColor = rgb;
    }
}

void PaintStyleEditor::on_ProportionalTouchup_valueChanged()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
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
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
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
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].fillActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_FillColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.fillColor(), this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(FillColor, rgb);
        FP.foregroundFill(rgb);
    }
}

void PaintStyleEditor::on_DrawIcon_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].iconActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_IconName_textEdited()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
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
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelActive(b);

    updatePagesIcons();
}

void PaintStyleEditor::on_LabelHalo_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelHalo(b);
}

void PaintStyleEditor::on_LabelArea_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelArea(b);
}

void PaintStyleEditor::on_LabelTag_textEdited()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelTag(LabelTag->text());
}

void PaintStyleEditor::on_LabelBackgroundTag_textEdited()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelBackgroundTag(LabelBackgroundTag->text());
}

void PaintStyleEditor::on_LabelColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.labelBoundary().Color, this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(LabelColor, rgb);
        FP.label(rgb, ProportionalLabel->value(), FixedLabel->value());
    }
}

void PaintStyleEditor::on_ProportionalLabel_valueChanged()
{
    if (FreezeUpdate)
        return;
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    FP.label(FP.labelBoundary().Color, ProportionalLabel->value(), FixedLabel->value());
}

void PaintStyleEditor::on_FixedLabel_valueChanged()
{
    on_ProportionalLabel_valueChanged();
}

void PaintStyleEditor::on_DrawLabelBackground_clicked(bool b)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].labelBackgroundActive(b);
}

void PaintStyleEditor::on_LabelBackgroundlColor_clicked()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    QColor rgb = QColorDialog::getColor(FP.labelBackgroundColor(), this
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                                        , tr("Select Color"), QColorDialog::ShowAlphaChannel
#endif
                                        );
    if (rgb.isValid()) {
        makeBoundaryIcon(LabelBackgroundlColor, rgb);
        FP.labelBackground(rgb);
    }
}

void PaintStyleEditor::on_LabelFont_currentFontChanged(const QFont & font)
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();

    if (idx >= thePainters.size())
        return;
    thePainters[idx].setLabelFont(font.toString());
}

void PaintStyleEditor::refreshPainter()
{
    QListWidgetItem* it = PaintList->currentItem();
    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= thePainters.size())
        return;
    Painter& FP(thePainters[idx]);
    FP.setSelector(TagSelection->text());
    PaintList->currentItem()->setText(FP.userName());
}

void PaintStyleEditor::on_buttonBox_clicked(QAbstractButton * button)
{
    if (buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
        emit(stylesApplied(&theGlobalPainter, &thePainters));
    }
}

void PaintStyleEditor::on_edFilter_textChanged(const QString &/*text*/)
{
    updatePaintList();
}

void PaintStyleEditor::on_btSelectorHelper_clicked()
{
    SelectionDialog* Sel = new SelectionDialog(g_Merk_MainWindow, false);
    if (!Sel)
        return;

    Sel->edTagQuery->setText(TagSelection->text());
    if (Sel->exec() == QDialog::Accepted) {
        TagSelection->setText(Sel->edTagQuery->text());
    }
}
