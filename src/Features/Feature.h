#ifndef MERKATOR_MAPFEATURE_H_
#define MERKATOR_MAPFEATURE_H_

#include "Maps/Coord.h"
#include "PaintStyle/PaintStyle.h"

#include <QtCore/QString>
#include <QList>

#include <boost/intrusive_ptr.hpp>

#define CAST_NODE(x) (dynamic_cast<Node*>(x))
#define CAST_WAY(x) (dynamic_cast<Way*>(x))
#define CAST_RELATION(x) (dynamic_cast<Relation*>(x))
#define CAST_SEGMENT(x) (dynamic_cast<TrackSegment*>(x))

class CommandList;
class Document;
class Layer;
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
			: theClass(IsLinear), InClassPriority(0.0), theLayer(0) { }
		RenderPriority(Class C, double IC, int L)
			: theClass(C), InClassPriority(IC), theLayer(L) { }
		RenderPriority(const RenderPriority& other)
			: theClass(other.theClass), InClassPriority(other.InClassPriority), theLayer(other.theLayer) { }
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
				theLayer = other.theLayer;
			}
			return *this;
		}
		int layer() const
		{
			return theLayer;
		}

	private:
		Class theClass;
		double InClassPriority;
		int theLayer;
};

namespace boost
{
	void intrusive_ptr_add_ref(Feature * p);
	void intrusive_ptr_release(Feature * p);
}

/// Used to store objects of the map
class Feature : public QObject
{
	Q_OBJECT

	public:
		typedef enum { User, UserResolved, OSMServer, OSMServerConflict, NotYetDownloaded, Log } ActorType;
		typedef enum { UnknownDirection, BothWays, OneWay, OtherWay } TrafficDirectionType;
		typedef enum {
			Relations			= 0x00000000,
			Ways				= 0x00000001,
			Nodes				= 0x00000002,
			All					= 0xffffffff
		} FeatureType;
	public:
		/// Constructor for an empty map feature
		Feature();
		/// Copy constructor
		/// @param other the MapFeature
		Feature(const Feature& other);
		/// Destructor
		virtual ~Feature() = 0;

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
		virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives) = 0;
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
		virtual void setLayer(Layer* aLayer);
		virtual Layer* layer();
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

		/** set the visibility state of the feature
		 */
		virtual void setVisible(bool val);

		/** check if the feature is visible
		 * @return true if visible
		 */
		virtual bool isVisible() const;

		/** check if the feature is hidden
		 * @return true if hidden
		 */
		virtual bool isHidden() const;

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
		void blockVirtualUpdates(bool val);
		bool isVirtualUpdatesBlocked() const;

		virtual bool isInteresting() const {return true;}

		const FeaturePainter* getEditPainter(double PixelPerM) const;
		const FeaturePainter* getCurrentEditPainter() const;
		bool hasEditPainter() const;
		void invalidatePainter();
		QVector<qreal> getParentDashes() const;

		virtual void remove(int Idx) = 0;
		virtual void remove(Feature* F) = 0;
		virtual int size() const = 0;
		virtual int find(Feature* Pt) const = 0;
		virtual Feature* get(int idx) = 0;
		virtual const Feature* get(int Idx) const = 0;
		virtual bool isNull() const = 0;

		void setParentFeature(Feature* F);
		void unsetParentFeature(Feature* F);
		int sizeParents() const;
		Feature* getParent(int i);
		const Feature* getParent(int i) const;
		virtual void partChanged(Feature* F, int ChangeId) = 0;
		void notifyChanges();
		void notifyParents(int Id);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL) = 0;
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress) = 0;

		virtual QString toMainHtml(QString type, QString systemtype);
		virtual QString toHtml() = 0;

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex) = 0;

		virtual QString getClass() const = 0;
		virtual void updateMeta() = 0;
		virtual void updateIndex();
		virtual void invalidateMeta();

		virtual bool deleteChildren(Document* , CommandList* ) { return true; }

		static Relation * GetSingleParentRelation(Feature * mapFeature);
		static Node* getTrackPointOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id);
		static Way* getWayOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id);
		static Relation* getRelationOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id);
		static void mergeTags(Document* theDocument, CommandList* L, Feature* Dest, Feature* Src);
		static bool QRectInterstects(const QRect& r, const QLine& l, QPoint& a, QPoint& b);


	private:
		MapFeaturePrivate* p;

	protected:
		mutable CoordBox BBox;
		bool MetaUpToDate;

		QString tagsToXML(int lvl=0);
		bool tagsToXML(QDomElement xParent);
		static void tagsFromXML(Document* d, Feature* f, QDomElement e);
		static QString stripToOSMId(const QString& id);

		long    m_references;
		friend void ::boost::intrusive_ptr_add_ref(Feature * p);
		friend void ::boost::intrusive_ptr_release(Feature * p);
};

Q_DECLARE_METATYPE( Feature * );

void copyTags(Feature* Dest, Feature* Src);
bool hasOSMId(const Feature* aFeature);

#endif


