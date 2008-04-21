#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "Map/Coord.h"
#include "Map/MapFeature.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"

#include <QtCore/QDateTime>
#include <QtXml>

class TrackPoint : public MapFeature
{
	public:
		TrackPoint(const Coord& aCoord);
		TrackPoint(const TrackPoint& other);
		virtual ~TrackPoint();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual void drawHover(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		const Coord& position() const;
		void setPosition(const Coord& aCoord);

		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		virtual QString toXML(unsigned int lvl=0);
		virtual bool toXML(QDomElement xParent);
		virtual bool toTrackXML(QDomElement xParent);
		static TrackPoint* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();

	private:
		Coord Position;
};

#endif


