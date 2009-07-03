#ifndef MERKAARTOR_PAINTSTYLE_EDITOR_H_
#define MERKAARTOR_PAINTSTYLE_EDITOR_H_

#include <ui_PaintStyleEditor.h>
#include "PaintStyle/PaintStyle.h"

#include <QtGui/QDialog>

#include <QList>

class PaintStyleEditor : public QDialog, public Ui::PaintStyleEditor
{
	Q_OBJECT

	public:
		PaintStyleEditor(QWidget* aParent, const GlobalPainter& aGlobalPainter, const QList<FeaturePainter>& aPainters);

	public slots:
		void on_DrawGlobalBackground_clicked(bool b);
		void on_GlobalBackgroundColor_clicked();

		void on_PaintList_itemSelectionChanged();
		void on_TagSelection_editingFinished();
		void on_LowerZoomBoundary_valueChanged();
		void on_UpperZoomBoundary_valueChanged();
		void on_DrawBackground_clicked(bool b);
		void on_BackgroundColor_clicked();
		void on_ProportionalBackground_valueChanged();
		void on_FixedBackground_valueChanged();
		void on_DrawForeground_clicked(bool b);
		void on_ForegroundColor_clicked();
		void on_ProportionalForeground_valueChanged();
		void on_FixedForeground_valueChanged();
		void on_ForegroundDashed_clicked();
		void on_ForegroundDashOn_valueChanged();
		void on_ForegroundDashOff_valueChanged();
		void on_DrawTouchup_clicked(bool b);
		void on_TouchupColor_clicked();
		void on_ProportionalTouchup_valueChanged();
		void on_FixedTouchup_valueChanged();
		void on_TouchupDashed_clicked();
		void on_TouchupDashOn_valueChanged();
		void on_TouchupDashOff_valueChanged();
		void on_DrawFill_clicked(bool b);
		void on_FillColor_clicked();
		void on_DrawIcon_clicked(bool b);
		void on_IconName_textEdited();
		void on_ProportionalIcon_valueChanged();
		void on_FixedIcon_valueChanged();
		void on_DrawLabel_clicked(bool b);
		void on_LabelColor_clicked();
		void on_LabelTag_textEdited();
		void on_ProportionalLabel_valueChanged();
		void on_FixedLabel_valueChanged();
		void on_DrawLabelBackground_clicked(bool b);
		void on_LabelBackgroundlColor_clicked();
		void on_LabelFont_currentFontChanged(const QFont & font);
		void on_LabelBackgroundTag_textEdited();
		void on_AddButton_clicked();
		void on_RemoveButton_clicked();
		void on_DuplicateButton_clicked();
		void on_btUp_clicked();
		void on_btDown_clicked();
		void on_buttonBox_clicked(QAbstractButton * button);
		void on_LabelHalo_clicked(bool b);
		void on_LabelArea_clicked(bool b);

	public:
		GlobalPainter	theGlobalPainter;
		QList<FeaturePainter> thePainters;
	private:
		void updatePaintList();
		void updatePagesIcons();

	signals:
		void stylesApplied(QList<FeaturePainter>* thePainters);
	private:
		bool FreezeUpdate;
};

#endif
