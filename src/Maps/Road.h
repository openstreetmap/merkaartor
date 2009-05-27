#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include <QList>

#include "Maps/MapDocument.h"
#include "Maps/MapFeature.h"
#include "Maps/MapLayer.h"

#ifndef _MOBILE
#include <geometry/geometry.hpp>
#endif

class RoadPrivate;
class TrackPoint;
class QProgressDialog;

class Road : public MapFeature
{
	Q_OBJECT

	public:
		Road(void);
		Road(const Road& other);
		virtual ~Road();
	private:
		void updateMeta() const;

	public:
		virtual QString getClass() const {return "Road";}

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM);

		virtual void add(TrackPoint* Pt);
		virtual void add(TrackPoint* Pt, int Idx);
		virtual void remove(int Idx);
		virtual void remove(MapFeature* F);
		virtual int size() const;
		virtual int find(MapFeature* Pt) const;
		virtual MapFeature* get(int idx);
		virtual const MapFeature* get(int Idx) const;
		virtual bool isNull() const;

		const QList<Coord>& smoothed() const;

		TrackPoint* getNode(int idx);
		const TrackPoint* getNode(int idx) const;
		
		bool isNodeAtEnd(TrackPoint* node);

		/** Set the tag "key=value" to the current object
		 * If a tag with the same key exist, it is replaced
		 * Otherwise the tag is added at the end
		 * @param key the key of the tag
		 * @param value the value corresponding to the key
		 */
		virtual void setTag(const QString& key, const QString& value, bool addToTagList=true);

		/** Set the tag "key=value" at the position index
		 * If a tag with the same key exist, it is replaced
		 * Otherwise the tag is added at the index position
		 * @param index the place for the given tag. Start at 0.
		 * @param key the key of the tag
		 * @param value the value corresponding to the key
		*/
		virtual void setTag(int index, const QString& key, const QString& value, bool addToTagList=true);

		/** remove all the tags for the curent feature
		 */
		virtual void clearTags();

		/** remove the tag with the key "k".
		 * if no corresponding tag, don't do anything
		 */
		virtual void clearTag(const QString& k);

		virtual void partChanged(MapFeature* F, int ChangeId);
		virtual void setLayer(MapLayer* aLayer);

		bool isCoastline() const;
		double area() const;
		bool isClosed() const;
		double distance() const;
		double widthOf();

		virtual bool deleteChildren(MapDocument* theDocument, CommandList* theList);

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QRect& clipRect);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static Road* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();
	
		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Road* fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id);

		bool isExtrimity(TrackPoint* node);
		static Road * GetSingleParentRoad(MapFeature * mapFeature);
		static Road * GetSingleParentRoadInner(MapFeature * mapFeature);

	protected:
		RoadPrivate* p;
};

Q_DECLARE_METATYPE( Road * );

MapFeature::TrafficDirectionType trafficDirection(const Road* R);
int findSnapPointIndex(const Road* R, Coord& P);

#endif


