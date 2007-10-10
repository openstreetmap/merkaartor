#ifndef MERKATOR_PROPERTIESDOCK_H_
#define MERKATOR_PROPERTIESDOCK_H_

#include "GeneratedFiles/ui_TrackPointProperties.h"
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
		void on_RemoveTagButton_clicked();

	private:
		void switchUi();
		void switchToNoUi();
		void switchToTrackPointUi();
		void switchToRoadUi();
		void switchToMultiUi();

		MainWindow* Main;
		QWidget* CurrentUi;
		std::vector<MapFeature*> Selection;
		Ui::TrackPointProperties TrackPointUi;
		Ui::RoadProperties RoadUi;
		Ui::MultiProperties MultiUi;
		TagModel* theModel;

		enum { NoUiShowing, TrackPointUiShowing, RoadUiShowing, MultiShowing } NowShowing ;
};

#endif


