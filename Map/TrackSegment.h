#ifndef MERKATOR_TRACKSEGMENT_H_
#define MERKATOR_TRACKSEGMENT_H_

#include "Map/MapFeature.h"

class TrackSegmentPrivate;
class TrackPoint;

class TrackSegment : public MapFeature
{
	public:
		TrackSegment(void);
		~TrackSegment(void);
	private:
		TrackSegment(const TrackSegment& other);

		bool visibleLine(const CoordBox & viewport, const Coord & last, const Coord & here);
		void drawDirectionMarkers(QPainter & P, QPen & pen, const QPointF & FromF, const QPointF & ToF);

	public:
		virtual QString getClass() const {return "TrackSegment";};

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual void drawHover(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		void add(TrackPoint* aPoint);
		void add(TrackPoint* Pt, unsigned int Idx);
		unsigned int find(TrackPoint* Pt) const;
		void remove(unsigned int idx);
		unsigned int size() const;
		TrackPoint* get(int i);
		void sortByTime();
		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		virtual QString toXML(unsigned int) {return QString("");};
		virtual bool toXML(QDomElement xParent);
		static TrackSegment* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml() {return "";};

		virtual void toBinary(QDataStream& /* ds */) { return; };

private:
		TrackSegmentPrivate* p;
};

#endif


