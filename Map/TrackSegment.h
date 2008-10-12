#ifndef MERKATOR_TRACKSEGMENT_H_
#define MERKATOR_TRACKSEGMENT_H_

#include "Map/MapFeature.h"

class TrackSegmentPrivate;
class TrackPoint;

class QProgressDialog;

class TrackSegment : public MapFeature
{
	public:
		TrackSegment(void);
		~TrackSegment(void);
	private:
		TrackSegment(const TrackSegment& other);

		void drawDirectionMarkers(QPainter & P, QPen & pen, const QPointF & FromF, const QPointF & ToF);

	public:
		virtual QString getClass() const {return "TrackSegment";};

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		void add(TrackPoint* aPoint);
		void add(TrackPoint* Pt, unsigned int Idx);
		virtual unsigned int find(MapFeature* Pt) const;
		virtual void remove(unsigned int idx);
		virtual void remove(MapFeature* F);
		virtual MapFeature* get(unsigned int idx);
		virtual unsigned int size() const;
		TrackPoint* getNode(unsigned int idx);
		virtual const MapFeature* get(unsigned int Idx) const;

		void sortByTime();
		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		virtual QString toXML(unsigned int, QProgressDialog *) {return QString("");};
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static TrackSegment* fromXML(MapDocument* d, MapLayer* L, const QDomElement e, QProgressDialog & progress);

		virtual QString toHtml() {return "";};

		virtual void toBinary(QDataStream& /* ds */, QHash <QString, quint64>& /*theIndex*/) { return; };

private:
		TrackSegmentPrivate* p;
};

#endif


