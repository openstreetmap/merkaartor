#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "Preferences/MerkaartorPreferences.h"

#include "Map/Coord.h"
#include "Map/MapFeature.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"

#include <QtCore/QDateTime>
#include <QtXml>

class QProgressDialog;

class TrackPoint : public MapFeature
{
	Q_OBJECT

	public:
		TrackPoint(const Coord& aCoord);
		TrackPoint(const TrackPoint& other);
		virtual ~TrackPoint();

		virtual QString getClass() const {return "TrackPoint";}

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		virtual unsigned int find(MapFeature* Pt) const;
		virtual void remove(unsigned int idx);
		virtual void remove(MapFeature* F);
		virtual unsigned int size() const;
		virtual MapFeature* get(unsigned int idx);
		virtual const MapFeature* get(unsigned int Idx) const;
		virtual bool isNull() const;
		virtual bool isInteresting() const;

		const Coord& position() const;
		void setPosition(const Coord& aCoord);
		const QPoint& projection() const;
		void setProjection(const QPoint& aProjection);
		ProjectionType projectionType() const;
		void setProjectionType(const ProjectionType aProjectionType);

		double speed() const;
		void setSpeed(double aSpeed);

		double elevation() const;
		void setElevation(double aElevation);

		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		virtual QString toXML(unsigned int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress);
		static TrackPoint* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);
		static TrackPoint* fromGPX(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static TrackPoint* fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id);

private:
		Coord Position;

		double Elevation;
		double Speed;
		QPoint Projected;
		ProjectionType ProjectedType;
};

Q_DECLARE_METATYPE( TrackPoint * );

#endif


