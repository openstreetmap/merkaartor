#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include "GeneratedFiles/ui_TrackPointProperties.h"
#include "GeneratedFiles/ui_RelationProperties.h"
#include "GeneratedFiles/ui_RoadProperties.h"
#include "GeneratedFiles/ui_MultiProperties.h"

#include <QtGui/QDockWidget>

#include <vector>

class MainWindow;
class MapFeature;
class TagModel;

class PropertiesDock : public QDockWidget
{
	Q_OBJECT

	public:
		PropertiesDock(MainWindow* aParent);
	public:
		~PropertiesDock(void);

		void setSelection(MapFeature* aFeature);
		template<class T>
		void setSelection(const std::vector<T*>& aFeatureList);
		void toggleSelection(MapFeature* aFeature);
		MapFeature* selection(unsigned int idx);
		unsigned int size() const;
		void resetValues();
		void checkMenuStatus();

	public slots:
		void on_TrackPointLat_textChanged(const QString& s);
		void on_TrackPointLon_textChanged(const QString& s);
		void on_RoadName_textChanged(const QString& s);
		void on_TrafficDirection_activated(int idx);
		void on_Highway_activated(int idx);
		void on_Amenity_activated(int idx);
		void on_RemoveTagButton_clicked();
		void on_SelectionList_itemSelectionChanged();
		void on_SelectionList_itemDoubleClicked(QListWidgetItem* item);
		void executePendingSelectionChange();

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


