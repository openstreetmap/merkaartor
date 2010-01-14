#ifndef MERKATOR_MAPFEATURE_H_
#define MERKATOR_MAPFEATURE_H_

#include "Maps/Coord.h"
#include "PaintStyle/PaintStyle.h"

#include <QtCore/QString>

#include <QList>

#define CAST_NODE(x) (dynamic_cast<TrackPoint*>(x))
#define CAST_WAY(x) (dynamic_cast<Road*>(x))
#define CAST_RELATION(x) (dynamic_cast<Relation*>(x))
#define CAST_SEGMENT(x) (dynamic_cast<TrackSegment*>(x))

class CommandList;
class MapDocument;
class MapLayer;
class Projection;

class QPointF;
class QPainter;
class QProgressDialog;

class MapFeaturePrivate;

class RenderPriority
{
	public:
		typedef enum { IsArea, IsLinear, IsSingular } Class;
		RenderPriority()
			: theClass(IsLinear), InClassPriority(0.0) { }
		RenderPriority(Class C, double IC)
			: theClass(C), InClassPriority(IC) { }
		RenderPriority(const RenderPriority& other)
			: theClass(other.theClass), InClassPriority(other.InClassPriority) { }
		bool operator<(const RenderPriority& R) const
		{
			return (theClass < R.theClass) ||
				( (theClass == R.theClass) && (InClassPriority < R.InClassPriority) );
		}
		bool operator==(const RenderPriority& R) const
		{
			return ((theClass == R.theClass) && (InClassPriority == R.InClassPriority));
		}
		RenderPriority &operator=(const RenderPriority &other)
		{
			if (this != &other) {
				theClass = other.theClass;
				InClassPriority = other.InClassPriority;
			}
			return *this;
		}

	private:
		Class theClass;
		double InClassPriority;
};

/// Used to store objects of the map
class MapFeature : public QObject
{
	Q_OBJECT

	public:
		typedef enum { User, UserResolved, OSMServer, OSMServerConflict, NotYetDownloaded, Log } ActorType;
		typedef enum { UnknownDirection, BothWays, OneWay, OtherWay } TrafficDirectionType;
		typedef enum {
			Relations			= 0x00000000,
			Roads				= 0x00000001,
			Nodes				= 0x00000002,
			All					= 0xffffffff
		} FeatureType;
	public:
		/// Constructor for an empty map feature
		MapFeature();
		/// Copy constructor
		/// @param other the MapFeature
		MapFeature(const MapFeature& other);
		/// Destructor
		virtual ~MapFeature() = 0;

		/** Return the smalest box contening all the MapFeature
		 * @return A coord box
		 */
		virtual CoordBox boundingBox() const = 0;

		/** Draw the feature using the given QPainter an Projection
		 * @param P The QPainter used to draw
		 * @param theProjection the Projection used to convert real coordinates to screen coordinates
		 */
		virtual void draw(QPainter& P, MapView* theView) = 0;

		/** Draw the feature using the given QPainter an Projection and with the focused draw
		 * @param P The QPainter used to draw
		 * @param theProjection the Projection used to convert real coordinates to screen coordinates
		 */
		virtual void drawFocus(QPainter& P, MapView* theView, bool solid=true) = 0;

		/** Draw the feature using the given QPainter an Projection and with the hover draw
		 * @param P The QPainter used to draw
		 * @param theProjection the Projection used to convert real coordinates to screen coordinates
		 */
		virtual void drawHover(QPainter& P, MapView* theView, bool solid=true) = 0;
		virtual void drawHighlight(QPainter& P, MapView* theView, bool solid=true) = 0;


		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const = 0;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives) = 0;
		virtual bool notEverythingDownloaded() = 0;

		/** Set the id for the current feature.
		 */
		void setId(const QString& id);

		/** Reset the id for the current feature to a random one.
		 */
		const QString& resetId();

		/** Give the id of the feature.
		 *  If the feature has no id, a random id is generated
		 * @return the id of the current feature
		 */
		const QString& id() const;
		/** Give the id of the feature in a form suitable for xml integration.
		 *  If the feature has no id, a random id is generated
		 * @return the id of the current feature
		 */
		qint64 idToLong() const;

		QString xmlId() const;
		bool hasOSMId() const;
		ActorType lastUpdated() const;
		void setLastUpdated(ActorType A);
		const QDateTime& time() const;
		void setTime(const QDateTime& aTime);
		const QString& user() const;
		void setUser(const QString& aUser);
		virtual void setLayer(MapLayer* aLayer);
		virtual MapLayer* layer();
		int versionNumber() const;
		void setVersionNumber(int vn);
		virtual QString description() const = 0;
		virtual const RenderPriority& renderPriority();
		virtual void setRenderPriority(const RenderPriority& aPriority);

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

		/** @return the number of tags for the current object
		 */
		virtual int tagSize() const;

		/** if a tag with the key "k" exists, return its index.
		 * if the key doesn't exist, return the number of tags
		 * @return index of tag
		 */
		virtual int findKey(const QString& k) const;

		/** return the value of the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 * @return the value
		 */
		virtual QString tagValue(int i) const;

		/** return the value of the tag with the key "k".
		 * if such a tag doesn't exists, return Default.
		 * @return value or Default
		 */
		virtual QString tagValue(const QString& k, const QString& Default) const;

		/** return the value of the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 * @return the value
		*/
		virtual QString tagKey(int i) const;

		/** remove the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 */
		virtual void removeTag(int i);

		/** check if the feature is on the dirty layer
		 * @return true if on the dirty layer
		 */
		virtual bool isDirty() const;

		/** check if the feature is on an uploadable layer
		 * @return true if on an uploadable layer
		 */
		virtual bool isUploadable() const;

		/** set the logical delete state of the feature
		 */
		virtual void setDeleted(bool delState);

		/** check if the feature is logically deleted
		 * @return true if logically deleted
		 */
		virtual bool isDeleted() const;

		/** check if the feature has been uploaded
		 * @return true if uploaded
		 */
		virtual bool isUploaded() const;
		virtual void setUploaded(bool state);

		/** check if the feature is virtual
		 * @return true if virtual
		 */
		virtual bool isVirtual() const;
		virtual void setVirtual(bool val);

		virtual bool isInteresting() const {return true;}

		const FeaturePainter* getEditPainter(double PixelPerM) const;
		const FeaturePainter* getCurrentEditPainter() const;
		bool hasEditPainter() const;
		void invalidatePainter();
		QVector<qreal> getParentDashes() const;

		virtual void remove(int Idx) = 0;
		virtual void remove(MapFeature* F) = 0;
		virtual int size() const = 0;
		virtual int find(MapFeature* Pt) const = 0;
		virtual MapFeature* get(int idx) = 0;
		virtual const MapFeature* get(int Idx) const = 0;
		virtual bool isNull() const = 0;

		void setParentFeature(MapFeature* F);
		void unsetParentFeature(MapFeature* F);
		int sizeParents() const;
		MapFeature* getParent(int i);
		const MapFeature* getParent(int i) const;
		virtual void partChanged(MapFeature* F, int ChangeId) = 0;
		void notifyChanges();
		void notifyParents(int Id);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL) = 0;
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress) = 0;

		virtual QString toMainHtml(QString type, QString systemtype);
		virtual QString toHtml() = 0;

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex) = 0;

		virtual QString getClass() const = 0;
		virtual void updateMeta() = 0;
		virtual void invalidateMeta();

		virtual bool deleteChildren(MapDocument* , CommandList* ) { return true; }

		static Relation * GetSingleParentRelation(MapFeature * mapFeature);
		static TrackPoint* getTrackPointOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id);
		static Road* getWayOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id);
		static Relation* getRelationOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id);
		static void mergeTags(MapDocument* theDocument, CommandList* L, MapFeature* Dest, MapFeature* Src);
		static bool QRectInterstects(const QRect& r, const QLine& l, QPoint& a, QPoint& b);


	private:
		MapFeaturePrivate* p;

	protected:
		bool MetaUpToDate;

		QString tagsToXML(int lvl=0);
		bool tagsToXML(QDomElement xParent);
		static void tagsFromXML(MapDocument* d, MapFeature* f, QDomElement e);
		static QString stripToOSMId(const QString& id);

};

Q_DECLARE_METATYPE( MapFeature * );

void copyTags(MapFeature* Dest, MapFeature* Src);
bool hasOSMId(const MapFeature* aFeature);

#endif


