#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include <ui_TrackPointProperties.h>
#include <ui_RelationProperties.h>
#include <ui_RoadProperties.h>
#include <ui_MultiProperties.h>

#include <QtGui/QDockWidget>

#include <vector>

class MainWindow;
class MapFeature;
class TagModel;
class EditCompleterDelegate;

class PropertiesDock : public QDockWidget
{
	Q_OBJECT

	public:
		PropertiesDock(MainWindow* aParent);
	public:
		~PropertiesDock(void);

		void setSelection(MapFeature* aFeature);
		void setMultiSelection(MapFeature* aFeature);
		template<class T>
				void setSelection(const std::vector<T*>& aFeatureList);
		void setMultiSelection(const std::vector<MapFeature*>& aFeatureList);
		void toggleSelection(MapFeature* aFeature);
		void addSelection(MapFeature* aFeature);
		MapFeature* selection(unsigned int idx);
		QVector<MapFeature*> selection();
		unsigned int size() const;
		void resetValues();
		void checkMenuStatus();

	public slots:
		void on_TrackPointLat_editingFinished();
		void on_TrackPointLon_editingFinished();
		void on_RoadName_editingFinished();
		void on_TrafficDirection_activated(int idx);
		void on_Highway_activated(int idx);
		void on_Amenity_activated(int idx);
		void on_LandUse_activated(int idx);
		void on_RemoveTagButton_clicked();
		void on_SelectionList_itemSelectionChanged();
		void on_SelectionList_itemDoubleClicked(QListWidgetItem* item);
		void executePendingSelectionChange();
		void on_SelectionList_customContextMenuRequested(const QPoint & pos);
		void on_centerAction_triggered();
		void on_centerZoomAction_triggered();

	private:
		void cleanUpUi();
		void switchUi();
		void switchToNoUi();
		void switchToTrackPointUi();
		void switchToRoadUi();
		void switchToMultiUi();
		void switchToRelationUi();
		void fillMultiUiSelectionBox();

		MainWindow* Main;
		QWidget* CurrentUi;
		std::vector<MapFeature*> Selection;
		std::vector<MapFeature*> FullSelection;
		Ui::TrackPointProperties TrackPointUi;
		Ui::RoadProperties RoadUi;
		Ui::MultiProperties MultiUi;
		Ui::RelationProperties RelationUi;
		TagModel* theModel;
		unsigned int PendingSelectionChange;
		EditCompleterDelegate* delegate;
		QAction* centerAction;
		QAction* centerZoomAction;

		enum { NoUiShowing, TrackPointUiShowing, RoadUiShowing, RelationUiShowing, MultiShowing } NowShowing ;
};

template<class T>
		void PropertiesDock::setSelection(const std::vector<T*>& aFeatureList)
{
	cleanUpUi();
	Selection.clear();
	for (unsigned int i=0; i<aFeatureList.size(); ++i)
		Selection.push_back(aFeatureList[i]);
	FullSelection = Selection;
	switchUi();
	fillMultiUiSelectionBox();
}


#endif


