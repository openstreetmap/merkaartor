#ifndef MERKAARTOR_SATELLITEVIEW_H_
#define MERKAARTOR_SATELLITEVIEW_H_

#include <QtGui/QWidget>

#include <vector>

class Satellite
{
	public:
		int Id;
		int Azimuth;
		int Elevation;
		int SignalStrength;
};

void sortSatellitesById(std::vector<Satellite>& List);

class SatelliteStrengthView : public QWidget
{
	Q_OBJECT

	public:
		SatelliteStrengthView(QWidget* aParent);

		void setSatellites(const std::vector<Satellite>& List);

	protected:
		void paintEvent(QPaintEvent* ev);

		std::vector<Satellite> List;
};

#endif
