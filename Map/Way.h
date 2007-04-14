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

		double width() const;
		void setWidth(double w);
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
		void addAsPartOf(Road* R);
		void removeAsPartOf(Road* R);
		bool isPartOf(Road* R);

	private:
		TrackPoint* From, *To;
		TrackPoint* ControlFrom, *ControlTo;
		std::vector<Road*> PartOf;
};

#endif


