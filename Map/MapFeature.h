#ifndef MERKATOR_MAPFEATURE_H_
#define MERKATOR_MAPFEATURE_H_

#include "Map/Coord.h"
#include "PaintStyle/PaintStyle.h"

#include <QtCore/QString>

#include <vector>

#define CAST_NODE(x) (dynamic_cast<TrackPoint*>(x))
#define CAST_WAY(x) (dynamic_cast<Road*>(x))
#define CAST_RELATION(x) (dynamic_cast<Relation*>(x))

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
		RenderPriority(Class C, double IC)
			: theClass(C), InClassPriority(IC) { }
		bool operator<(const RenderPriority& R) const
		{
			return (theClass < R.theClass) ||
				( (theClass == R.theClass) && (InClassPriority > R.InClassPriority) );
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
		virtual void draw(QPainter& P, const Projection& theProjection) = 0;

		/** Draw the feature using the given QPainter an Projection and with the focused draw
		 * @param P The QPainter used to draw
		 * @param theProjection the Projection used to convert real coordinates to screen coordinates
		 */
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true) = 0;

		/** Draw the feature using the given QPainter an Projection and with the hover draw
		 * @param P The QPainter used to draw
		 * @param theProjection the Projection used to convert real coordinates to screen coordinates
		 */
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true) = 0;
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const = 0;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives) = 0;
		virtual bool notEverythingDownloaded() const = 0;

		/** Set the id for the current feature.
		 */
		void setId(const QString& id);

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
		virtual RenderPriority renderPriority(double aPixelPerM) const = 0;

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

		/** @return the number of tags for the current object
		 */
		unsigned int tagSize() const;

		/** if a tag with the key "k" exists, return its index.
		 * if the key doesn't exist, return the number of tags
		 * @return index of tag
		 */
		unsigned int findKey(const QString& k) const;

		/** return the value of the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 * @return the value
		 */
		QString tagValue(unsigned int i) const;

		/** return the value of the tag with the key "k".
		 * if such a tag doesn't exists, return Default.
		 * @return value or Default
		 */
		QString tagValue(const QString& k, const QString& Default) const;

		/** return the value of the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 * @return the value
		*/
		QString tagKey(unsigned int i) const;

		/** remove the tag at the position "i".
		 * position start at 0.
		 * Be carefull: no verification is made on i.
		 */
		void removeTag(unsigned int i);

		/** check if the feature is on the dirty layer
		 * @return true if on the dirty layer
		 */
		bool isDirty() const;

		/** check if the feature is on an uploadable layer
		 * @return true if on an uploadable layer
		 */
		bool isUploadable() const;

		/** set the logical delete state of the feature
		 */
		void setDeleted(bool delState);

		/** check if the feature is logically deleted
		 * @return true if logically deleted
		 */
		bool isDeleted() const;

		/** check if the feature has been uploaded
		 * @return true if uploaded
		 */
		bool isUploaded() const;

		const FeaturePainter* getEditPainter(double PixelPerM) const;
		const FeaturePainter* getCurrentEditPainter() const;
		bool hasEditPainter() const;
		void invalidatePainter();
		QVector<qreal> getParentDashes() const;

		virtual void remove(unsigned int Idx) = 0;
		virtual void remove(MapFeature* F) = 0;
		virtual unsigned int size() const = 0;
		virtual unsigned int find(MapFeature* Pt) const = 0;
		virtual MapFeature* get(unsigned int idx) = 0;
		virtual const MapFeature* get(unsigned int Idx) const = 0; 
		virtual bool isNull() const = 0;

		void setParent(MapFeature* F);
		void unsetParent(MapFeature* F);
		unsigned int sizeParents() const;
		MapFeature* getParent(unsigned int i);
		const MapFeature* getParent(unsigned int i) const;
		virtual void partChanged(MapFeature* F, unsigned int ChangeId) = 0;
		void notifyChanges();
		void notifyParents(unsigned int Id);

		virtual QString toXML(unsigned int lvl=0, QProgressDialog * progress=NULL) = 0;
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress) = 0;

		virtual QString toMainHtml(QString type, QString systemtype);
		virtual QString toHtml() = 0;

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex) = 0;

		virtual QString getClass() const = 0;

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
		QString tagsToXML(unsigned int lvl=0);
		bool tagsToXML(QDomElement xParent);
		static void tagsFromXML(MapDocument* d, MapFeature* f, QDomElement e);
		static QString stripToOSMId(const QString& id);

};

Q_DECLARE_METATYPE( MapFeature * );

void copyTags(MapFeature* Dest, MapFeature* Src);
bool hasOSMId(const MapFeature* aFeature);

#endif


