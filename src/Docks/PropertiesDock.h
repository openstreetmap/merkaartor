#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include <ui_MinimumTrackPointProperties.h>
#include <ui_DefaultProperties.h>
#include <ui_MultiProperties.h>

#include <QList>

#include "MDockAncestor.h"
#include "ShortcutOverrideFilter.h"

class Feature;
class TagModel;
class EditCompleterDelegate;
class TagTemplates;
class TagTemplate;
class CommandList;

class PropertiesDock : public MDockAncestor
{
    Q_OBJECT

    public:
        PropertiesDock();
    public:
        ~PropertiesDock(void);

        Feature* selection(int idx);
        QList<Feature*> selection();
        bool isSelected(Feature *aFeature);
        int selectionSize() const;
        void resetValues();
        void checkMenuStatus();
        bool loadTemplates(const QString& filename = "");
        bool mergeTemplates(const QString& filename = "");
        bool saveTemplates(const QString& filename);

        Feature* highlighted(int idx);
        QList<Feature*> highlighted();
        int highlightedSize() const;

public slots:
        void setSelection(Feature* aFeature);
        void setMultiSelection(const QList<Feature*>& aFeatureList);
        void setHighlighted(const QList<Feature*>& aFeatureList);
        void toggleSelection(Feature* aFeature);
        void addSelection(Feature* aFeature);

        void on_TrackPointLat_editingFinished();
        void on_TrackPointLon_editingFinished();
        void on_RemoveTagButton_clicked();
        void on_SourceTagButton_clicked();
        void on_SelectionList_itemSelectionChanged();
        void on_SelectionList_itemDoubleClicked(QListWidgetItem* item);
        void executePendingSelectionChange();
        void on_SelectionList_customContextMenuRequested(const QPoint & pos);
        void on_centerAction_triggered();
        void on_centerZoomAction_triggered();
        void on_tag_selection_changed(const QItemSelection & selected, const QItemSelection & deselected);
        void on_tag_changed(QString k, QString v);
        void on_tag_cleared(QString k);
        void on_template_changed(TagTemplate* aNewTemplate);
        void adjustSelection();

    private:
        void cleanUpUi();
        void switchUi();
        void switchToNoUi();
        void switchToNodeUi();
        void switchToDefaultUi();
        void switchToMultiUi();
        void fillMultiUiSelectionBox();
        void changeEvent(QEvent*);
        void retranslateUi();

        QWidget* CurrentUi;
        QList<Feature*> Selection;
        QList<Feature*> FullSelection;
        Ui::TrackPointProperties TrackPointUi;
        Ui::DefaultProperties DefaultUi;
        Ui::MultiProperties MultiUi;
        TagModel* theModel;
        int PendingSelectionChange;
        EditCompleterDelegate* delegate;
        QAction* centerAction;
        QAction* centerZoomAction;
        ShortcutOverrideFilter* shortcutFilter;
        TagTemplates* theTemplates;

        QList<Feature*> Highlighted;

        QTableView *CurrentTagView;
        QTableView *CurrentMembersView;

        enum { NoUiShowing, TrackPointUiShowing, DefaultUiShowing, MultiShowing } NowShowing ;
};

#endif


