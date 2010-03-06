#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include <QList>

#include "Document.h"
#include "Feature.h"
#include "Layer.h"

#ifndef _MOBILE
#include <ggl/ggl.hpp>
#endif

class WayPrivate;
class Node;
class QProgressDialog;

class Way : public Feature
{
	Q_OBJECT

	public:
		Way(void);
		Way(const Way& other);
		virtual ~Way();

	public:
		virtual QString getClass() const {return "Road";}
		virtual Feature::FeatureType getType() const {return Feature::Ways;}
		virtual void updateMeta();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, MapView* theView);
		virtual void drawFocus(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHover(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHighlight(QPainter& P, MapView* theView, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, bool selectNodes, const Projection& theProjection, const QTransform& thensform) const;
		virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives);
		virtual bool notEverythingDownloaded();
		virtual QString description() const;

		virtual void add(Node* Pt);
		virtual void add(Node* Pt, int Idx);
		virtual void remove(int Idx);
		virtual void remove(Feature* F);
		virtual int size() const;
		virtual int find(Feature* Pt) const;
		virtual int findVirtual(Feature* Pt) const;
		virtual Feature* get(int idx);
		virtual const Feature* get(int Idx) const;
		virtual bool isNull() const;
		virtual void setDeleted(bool delState);
		void removeVirtuals();
		void addVirtuals();
		void updateVirtuals();

		const QList<Coord>& smoothed() const;

		Node* getNode(int idx);
		const Node* getNode(int idx) const;
		const std::vector<NodePtr>& getNodes() const;

		bool isNodeAtEnd(Node* node);

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

		virtual void partChanged(Feature* F, int ChangeId);
		virtual void setLayer(Layer* aLayer);

		bool isCoastline();
		double area();
		bool isClosed() const;
		double distance();
		double widthOf();

		virtual bool deleteChildren(Document* theDocument, CommandList* theList);

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QTransform& thensform, const QRectF& clipRect);

		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport=false);
		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static Way* fromXML(Document* d, Layer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Way* fromBinary(Document* d, OsbLayer* L, QDataStream& ds, qint8 c, qint64 id);

		bool isExtrimity(Node* node);
		static Way * GetSingleParentRoad(Feature * mapFeature);
		static Way * GetSingleParentRoadInner(Feature * mapFeature);

		static int createJunction(Document* theDocument, CommandList* theList, Way* R1, Way* R2, bool doIt);

	protected:
		WayPrivate* p;
};

Q_DECLARE_METATYPE( Way * );

Feature::TrafficDirectionType trafficDirection(const Way* R);
int findSnapPointIndex(const Way* R, Coord& P);

#endif


