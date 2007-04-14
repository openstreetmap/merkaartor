#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include "Map/MapFeature.h"

class RoadPrivate;
class Way;

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

		void add(Way* W);
		void add(Way* W, unsigned int Idx);
		void erase(Way* W);
		unsigned int size() const;
		unsigned int find(Way* W) const;
		Way* get(unsigned int idx);
		const Way* get(unsigned int Idx) const;


		RoadPrivate* p;
};

MapFeature::TrafficDirectionType trafficDirection(const Road* R);


#endif


