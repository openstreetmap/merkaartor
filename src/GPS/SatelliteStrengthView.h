#ifndef MERKAARTOR_SATELLITEVIEW_H_
#define MERKAARTOR_SATELLITEVIEW_H_

#include <QtGui/QWidget>
#include <QList>

class Satellite
{
	public:
		int Id;
		int Azimuth;
		int Elevation;
		int SignalStrength;
};

void sortSatellitesById(QList<Satellite>& List);

class SatelliteStrengthView : public QWidget
{
	Q_OBJECT

	public:
		SatelliteStrengthView(QWidget* aParent);

		void setSatellites(const QList<Satellite>& List);

	protected:
		void paintEvent(QPaintEvent* ev);

		QList<Satellite> List;
};

#endif
