#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include "Map/MapFeature.h"

class RoadPrivate;
class TrackPoint;

class Road : public MapFeature
{
	public:
		Road(void);
		virtual ~Road();
	private:
		Road(const Road& other);

	public:
		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);		
		virtual bool notEverythingDownloaded() const;

		void add(TrackPoint* Pt);
		void add(TrackPoint* Pt, unsigned int Idx);
		void remove(unsigned int Idx);
		unsigned int size() const;
		unsigned int find(TrackPoint* Pt) const;
		TrackPoint* get(unsigned int idx);
		const TrackPoint* get(unsigned int Idx) const;

		RoadPrivate* p;
};

MapFeature::TrafficDirectionType trafficDirection(const Road* R);
double widthOf(const Road* R);
unsigned int findSnapPointIndex(const Road* R, Coord& P);
bool isClosed(Road* R);

#endif


