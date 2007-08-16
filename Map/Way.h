#ifndef MERKATOR_WAY_H_
#define MERKATOR_WAY_H_

#include "Map/MapFeature.h"

class Road;
class TrackPoint;

class Way : public MapFeature
{
	public:
		Way(TrackPoint* aFrom, TrackPoint* aTo);
		Way(TrackPoint* aFrom, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* aTo);
		~Way(void);

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual bool notEverythingDownloaded() const;

		MapFeature::TrafficDirectionType trafficDirection() const;
		TrackPoint* from();
		TrackPoint* to();
		const TrackPoint* from() const;
		const TrackPoint* to() const;
		TrackPoint* controlFrom();
		TrackPoint* controlTo();
		void setFromTo(TrackPoint* From, TrackPoint* To);
		void setFromTo(TrackPoint* From, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* To);
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		bool isPartOf(Road* R);
		const std::vector<Road*>& containers() const;

	private:
		void addAsPartOf(Road* R);
		void removeAsPartOf(Road* R);

		TrackPoint* From, *To;
		TrackPoint* ControlFrom, *ControlTo;
		std::vector<Road*> PartOf;

		// So it can do addAsPartOf and removeAsPartOf
		friend class Road;
};

QString cascadedTagValue(const Way* W, const QString& Tag, const QString& Default);
QString highwayTypeOf(const Way* W);
double widthOf(const Way* W);

#endif


