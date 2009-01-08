#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include <vector>

#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"

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
		virtual QString getClass() const {return "Road";};

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		virtual void add(TrackPoint* Pt);
		virtual void add(TrackPoint* Pt, unsigned int Idx);
		virtual void remove(unsigned int Idx);
		virtual void remove(MapFeature* F);
		virtual unsigned int size() const;
		virtual unsigned int find(MapFeature* Pt) const;
		virtual MapFeature* get(unsigned int idx);
		virtual const MapFeature* get(unsigned int Idx) const;
		virtual bool isNull() const;

		const std::vector<Coord>& smoothed() const;

		TrackPoint* getNode(unsigned int idx);
		const TrackPoint* getNode(unsigned int idx) const;

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
		virtual void setTag(unsigned int index, const QString& key, const QString& value, bool addToTagList=true);

		/** remove all the tags for the curent feature
		 */
		virtual void clearTags();

		/** remove the tag with the key "k".
		 * if no corresponding tag, don't do anything
		 */
		virtual void clearTag(const QString& k);

		virtual void partChanged(MapFeature* F, unsigned int ChangeId);
		virtual void setLayer(MapLayer* aLayer);

		double area() const;
		double distance() const;
		double widthOf();

		virtual bool deleteChildren(MapDocument* theDocument, CommandList* theList);

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QRegion& paintRegion);

		virtual QString toXML(unsigned int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static Road* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();
	
		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Road* fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id);

		bool isExtrimity(TrackPoint* node);
		static Road * GetSingleParentRoad(MapFeature * mapFeature);

	protected:
		RoadPrivate* p;
};

MapFeature::TrafficDirectionType trafficDirection(const Road* R);
unsigned int findSnapPointIndex(const Road* R, Coord& P);

#endif


