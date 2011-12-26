#ifndef MERKATOR_MAPFEATURE_H_
#define MERKATOR_MAPFEATURE_H_

#include "IFeature.h"
#include "Coord.h"
#include "MapView.h"
#include "FeaturePainter.h"

#include <QtCore/QString>
#include <QList>

#define CAST_FEATURE(x) (dynamic_cast<Feature*>(x))
#define CAST_NODE(x) (dynamic_cast<Node*>(x))
#define CAST_TRACKNODE(x) (dynamic_cast<TrackNode*>(x))
#define CAST_WAY(x) (dynamic_cast<Way*>(x))
#define CAST_RELATION(x) (dynamic_cast<Relation*>(x))
#define CAST_SEGMENT(x) (dynamic_cast<TrackSegment*>(x))

#define STATIC_CAST_FEATURE(x) (static_cast<Feature*>(x))
#define STATIC_CAST_NODE(x) (static_cast<Node*>(x))
#define STATIC_CAST_TRACKNODE(x) (static_cast<TrackNode*>(x))
#define STATIC_CAST_WAY(x) (static_cast<Way*>(x))
#define STATIC_CAST_RELATION(x) (static_cast<Relation*>(x))
#define STATIC_CAST_SEGMENT(x) (static_cast<TrackSegment*>(x))

#define CHECK_NODE(x) ((x)->getType() & IFeature::Point)
#define CHECK_WAY(x) ((x)->getType() & IFeature::LineString)
#define CHECK_RELATION(x) ((x)->getType() & IFeature::OsmRelation)
#define CHECK_SEGMENT(x) ((x)->getType() & IFeature::GpxSegment)

class CommandList;
class Document;
class Layer;
class Projection;
class TrackNode;

class QPointF;
class QPainter;
class QPen;
class QProgressDialog;

class FeaturePrivate;

class RenderPriority
{
public:
    typedef enum { IsArea, IsLinear, IsSingular } Class;
    RenderPriority()
        : theClass(IsLinear), InClassPriority(0.0), theLayer(0) { }
    RenderPriority(Class C, qreal IC, int L)
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
    qreal InClassPriority;
    int theLayer;
};

namespace boost
{
void intrusive_ptr_add_ref(Feature * p);
void intrusive_ptr_release(Feature * p);
}

/// Used to store objects of the map
class Feature : public IFeature
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

public:
    typedef enum { User, UserResolved, OSMServer, OSMServerConflict, NotYetDownloaded, Log } ActorType;
    typedef enum { UnknownDirection, BothWays, OneWay, OtherWay } TrafficDirectionType;
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
    virtual const CoordBox& boundingBox(bool update=true) const = 0;

    /** Draw the feature using the given QPainter an Projection
         * @param P The QPainter used to draw
         * @param theProjection the Projection used to convert real coordinates to screen coordinates
         */
    virtual void draw(QPainter& P, MapView* theView) = 0;

    virtual void drawSpecial(QPainter& P, QPen& Pen, MapView* theView) = 0;
    virtual void drawParentsSpecial(QPainter& P, QPen& Pen, MapView* theView) = 0;
    virtual void drawChildrenSpecial(QPainter & P, QPen& Pen, MapView *theView, int depth) = 0;

    virtual void drawHover(QPainter& P, MapView* theView);
    virtual void drawHighlight(QPainter& P, MapView* theView);
    virtual void drawFocus(QPainter& P, MapView* theView);

    virtual qreal pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const = 0;
    virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives) = 0;
    virtual bool notEverythingDownloaded() = 0;

    /** Set the id for the current feature.
         */
    void setId(const IFeature::FId& id);

    /** Reset the id for the current feature to a random one.
         */
    const IFeature::FId& resetId() const;

    /** Give the id of the feature.
         *  If the feature has no id, a random id is generated
         * @return the id of the current feature
         */
    const IFeature::FId& id() const;

    QString xmlId() const;
    bool hasOSMId() const;
    ActorType lastUpdated() const;
    void setLastUpdated(ActorType A);
#ifndef FRISIUS_BUILD
    const QDateTime time() const;
    void setTime(const QDateTime& aTime);
    void setTime(uint epoch);
    const QString& user() const;
    void setUser(const QString& aUser);
    int versionNumber() const;
    void setVersionNumber(int vn);
#endif
    virtual void setLayer(Layer* aLayer);
    virtual Layer* layer() const;
    virtual QString description() const = 0;

    /** Set the tag "key=value" to the current object
         * If a tag with the same key exist, it is replaced
         * Otherwise the tag is added at the end
         * @param key the key of the tag
         * @param value the value corresponding to the key
         */
    virtual void setTag(const QString& key, const QString& value);

    /** Set the tag "key=value" at the position index
         * If a tag with the same key exist, it is replaced
         * Otherwise the tag is added at the index position
         * @param index the place for the given tag. Start at 0.
         * @param key the key of the tag
         * @param value the value corresponding to the key
        */
    virtual void setTag(int index, const QString& key, const QString& value);

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

    /** check if the dirty status of the feature
         * @return true if the feature is dirty
         */
    virtual bool isDirty() const;

    virtual int incDirtyLevel(int inc=1);
    virtual int decDirtyLevel(int inc=1);
    virtual int getDirtyLevel() const;
    virtual int setDirtyLevel(int newLevel);

    /** check if the feature is on an uploadable layer
         * @return true if on an uploadable layer
         */
    virtual bool isUploadable() const;

    /** check if the feature is read-only
         * @return true if is read-only
         */
    virtual bool isReadonly();
    virtual void setReadonly(bool val);

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
    virtual bool isVisible();

    /** check if the feature is hidden
         * @return true if hidden
         */
    virtual bool isHidden();

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

    /** check if the feature is a special one (cannot be uploaded)
         * @return true if special
         */
    virtual bool isSpecial() const;
    virtual void setSpecial(bool val);

    virtual const QPainterPath& getPath() const;

    const FeaturePainter* getPainter(qreal PixelPerM) const;
    const FeaturePainter* getCurrentPainter() const;
    bool hasPainter() const;
    bool hasPainter(qreal PixelPerM) const;
    void invalidatePainter();
    QVector<qreal> getParentDashes() const;

    virtual qreal getAlpha();

    virtual void remove(int Idx) = 0;
    virtual void remove(Feature* F) = 0;
    virtual int size() const = 0;
    virtual int find(Feature* Pt) const = 0;
    virtual Feature* get(int idx) = 0;
    virtual const Feature* get(int Idx) const = 0;
    virtual bool isNull() const = 0;

    int sizeParents() const;
    IFeature* getParent(int i);
    const IFeature* getParent(int i) const;

    void setParentFeature(Feature* F);
    void unsetParentFeature(Feature* F);
    virtual void partChanged(Feature* F, int ChangeId) = 0;
    void notifyChanges();
    void notifyParents(int Id);

    static void fromXML(QXmlStreamReader& stream, Feature* F);
    virtual void toXML(QXmlStreamWriter& stream, bool strict, QString changetsetid="");

    virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
    virtual bool toXML(QXmlStreamWriter& stream, QProgressDialog * progress=NULL, bool strict=false, QString changetsetid="") = 0;

    QString toMainHtml(QString type, QString systemtype);
    virtual QString toHtml() { return ""; }

    virtual QString getClass() const = 0;
    virtual char getType() const = 0;
    virtual void updateMeta();
    virtual void updateFilters();
    virtual void invalidateMeta();

    virtual bool deleteChildren(Document* , CommandList* ) { return true; }

    static Relation * GetSingleParentRelation(Feature * mapFeature);
    static Node* getNodeOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id);
    static TrackNode* getTrackNodeOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id);
    static Way* getWayOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id);
    static Relation* getRelationOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const IFeature::FId& Id);
    static void mergeTags(Document* theDocument, CommandList* L, Feature* Dest, Feature* Src);

    static QString stripToOSMId(const IFeature::FId& id);

public:
    quint32 internal_id;

private:
    FeaturePrivate* p;

protected:
    mutable CoordBox BBox;
    bool MetaUpToDate;
    IFeature::FId newId(IFeature::FeatureType type) const;

    bool tagsToXML(QXmlStreamWriter& stream, bool strict);
    static void tagsFromXML(Document* d, Feature* f, QXmlStreamReader& stream);

    long    m_references;
    friend void ::boost::intrusive_ptr_add_ref(Feature * p);
    friend void ::boost::intrusive_ptr_release(Feature * p);

    QPainterPath thePath;
};

Q_DECLARE_METATYPE( Feature * );

extern qint64 g_feat_rndId;

void copyTags(Feature* Dest, Feature* Src);
bool hasOSMId(const Feature* aFeature);

#endif


