#ifndef MERKATOR_TRACKSEGMENT_H_
#define MERKATOR_TRACKSEGMENT_H_

#include "Maps/MapFeature.h"

class TrackSegmentPrivate;
class TrackPoint;

class QProgressDialog;

class TrackSegment : public MapFeature
{
	Q_OBJECT

	public:
		TrackSegment(void);
		~TrackSegment(void);
	private:
		TrackSegment(const TrackSegment& other);

		void drawDirectionMarkers(QPainter & P, QPen & pen, const QPointF & FromF, const QPointF & ToF);

	public:
		virtual QString getClass() const {return "TrackSegment";}

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection, const QTransform& theTransform);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, const QTransform& theTransform, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, const QTransform& theTransform, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const;
		void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority();

		void add(TrackPoint* aPoint);
		void add(TrackPoint* Pt, int Idx);
		virtual int find(MapFeature* Pt) const;
		virtual void remove(int idx);
		virtual void remove(MapFeature* F);
		virtual MapFeature* get(int idx);
		virtual int size() const;
		TrackPoint* getNode(int idx);
		virtual const MapFeature* get(int Idx) const;
		virtual bool isNull() const;

		void sortByTime();
		virtual void partChanged(MapFeature* F, int ChangeId);

		virtual QString toXML(int, QProgressDialog *) {return QString("");}
		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport=false);
		static TrackSegment* fromGPX(MapDocument* d, MapLayer* L, const QDomElement e, QProgressDialog & progress);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static TrackSegment* fromXML(MapDocument* d, MapLayer* L, const QDomElement e, QProgressDialog & progress);

		virtual QString toHtml() {return "";}

		virtual void toBinary(QDataStream& /* ds */, QHash <QString, quint64>& /*theIndex*/) { return; }

private:
		TrackSegmentPrivate* p;
};

#endif


