#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include <ui_MinimumTrackPointProperties.h>
#include <ui_MinimumRelationProperties.h>
#include <ui_MinimumRoadProperties.h>
#include <ui_MultiProperties.h>

#include <QList>

#include "Utils/MDockAncestor.h"
#include "Utils/ShortcutOverrideFilter.h"

class MainWindow;
class MapFeature;
class TagModel;
class EditCompleterDelegate;
class TagTemplates;
class TagTemplate;
class CommandList;

class PropertiesDock : public MDockAncestor
{
	Q_OBJECT

	public:
		PropertiesDock(MainWindow* aParent);
	public:
		~PropertiesDock(void);

		void setSelection(MapFeature* aFeature);
		void setMultiSelection(MapFeature* aFeature);
		template<class T>
				void setSelection(const QList<T*>& aFeatureList);
		void setMultiSelection(const QList<MapFeature*>& aFeatureList);
		void toggleSelection(MapFeature* aFeature);
		void addSelection(MapFeature* aFeature);
		void adjustSelection();
		MapFeature* selection(int idx);
		QList<MapFeature*> selection();
		bool isSelected(MapFeature *aFeature);
		int size() const;
		void resetValues();
		void checkMenuStatus();
		bool loadTemplates(const QString& filename = "");
		bool mergeTemplates(const QString& filename = "");
		bool saveTemplates(const QString& filename);

	public slots:
		void on_TrackPointLat_editingFinished();
		void on_TrackPointLon_editingFinished();
		void on_Member_customContextMenuRequested(const QPoint & pos);
		void on_RemoveMemberButton_clicked();
		void on_RemoveTagButton_clicked();
		void on_SelectionList_itemSelectionChanged();
		void on_SelectionList_itemDoubleClicked(QListWidgetItem* item);
		void executePendingSelectionChange();
		void on_SelectionList_customContextMenuRequested(const QPoint & pos);
		void on_centerAction_triggered();
		void on_centerZoomAction_triggered();
		void on_tag_changed(QString k, QString v);
		void on_tag_cleared(QString k);
		void on_template_changed(TagTemplate* aNewTemplate);

	private:
		void cleanUpUi();
		void switchUi();
		void switchToNoUi();
		void switchToTrackPointUi();
		void switchToRoadUi();
		void switchToMultiUi();
		void switchToRelationUi();
		void fillMultiUiSelectionBox();
		void changeEvent(QEvent*);
		void retranslateUi();

		MainWindow* Main;
		QWidget* CurrentUi;
		QList<MapFeature*> Selection;
		QList<MapFeature*> FullSelection;
		Ui::TrackPointProperties TrackPointUi;
		Ui::RoadProperties RoadUi;
		Ui::MultiProperties MultiUi;
		Ui::RelationProperties RelationUi;
		TagModel* theModel;
		int PendingSelectionChange;
		EditCompleterDelegate* delegate;
		QAction* centerAction;
		QAction* centerZoomAction;
		ShortcutOverrideFilter* shortcutFilter;
		TagTemplates* theTemplates;

		QTableView *CurrentTagView;
		QTableView *CurrentMembersView;

		enum { NoUiShowing, TrackPointUiShowing, RoadUiShowing, RelationUiShowing, MultiShowing } NowShowing ;
};

template<class T>
		void PropertiesDock::setSelection(const QList<T*>& aFeatureList)
{
	cleanUpUi();
	Selection.clear();
	for (int i=0; i<aFeatureList.size(); ++i)
		Selection.push_back(aFeatureList[i]);
	FullSelection = Selection;
	switchUi();
	fillMultiUiSelectionBox();
}


#endif


