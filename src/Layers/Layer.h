#ifndef LAYER_H_
#define LAYER_H_

#include "Maps/MapTypedef.h"
#include "Maps/Coord.h"
#include "Feature.h"

#include <QProgressDialog>

#include <deque>
#include <ggl/extensions/index/rtree/rtree.hpp>

typedef ggl::index::rtree<CoordBox, MapFeaturePtr> MyRTree;

class QString;
class QprogressDialog;

class Feature;
class LayerPrivate;
class MapAdapter;
class Layer;
class LayerManager;
class LayerWidget;
class WMSMapAdapter;
class TileMapAdapter;
class TrackSegment;
class OsbLayerPrivate;
class IMapAdapter;
class Document;

class Layer : public QObject
{
    Q_OBJECT

public:
    Layer();
    Layer(const QString& aName);

private:
    Layer(const Layer& aLayer);

public:
    typedef enum {
        UndefinedType,
        DeletedLayerType,
        DirtyLayerType,
        DrawingLayerType,
        ExtractedLayerType,
        ImageLayerType,
        OsbLayerType,
        TrackLayerType,
        UploadedLayerType
    } LayerType;

    enum LayerGroup {
        None				= 0x00000000,
        Default				= 0x00000001,
        OSM					= 0x00000002,
        Tracks				= 0x00000004,
        All					= 0xffffffff
    };

    Q_DECLARE_FLAGS(LayerGroups, LayerGroup)

public:
    virtual ~Layer();

    void setName(const QString& aName);
    const QString& name() const;
    void setDescription(const QString& aDesc);
    const QString& description() const;
    bool isVisible() const;
    bool isSelected() const;
    bool isEnabled() const;

    virtual void add(Feature* aFeature);
    virtual void add(Feature* aFeature, int Idx);
    virtual void remove(Feature* aFeature);
    virtual void deleteFeature(Feature* aFeature);
    virtual void clear();
    bool exists(Feature* aFeature) const;
    virtual int size() const;
    int get(Feature* aFeature);
    QList<Feature *> get();
    Feature* get(int i);
    const Feature* get(int i) const;
    Feature* get(const QString& id, bool exact=true);
    void notifyIdUpdate(const QString& id, Feature* aFeature);

    virtual void get(const CoordBox& hz, QList<Feature*>& theFeatures);
    void getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, QSet<Way*>& theCoastlines, Document* theDocument,
                       QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform);

    void setDocument(Document* aDocument);
    Document* getDocument();

    LayerWidget* getWidget(void);
    void deleteWidget(void);
    virtual void updateWidget() {}

    virtual void setVisible(bool b);
    virtual void setSelected(bool b);
    virtual void setEnabled(bool b);
    virtual void setReadonly(bool b);
    virtual void setUploadable(bool b);
    virtual LayerWidget* newWidget(void) = 0;

    virtual void setAlpha(const qreal alpha);
    virtual qreal getAlpha() const;

    void setId(const QString& id);
    const QString& id() const;

    virtual QString toMainHtml();
    virtual QString toHtml();
    virtual bool toXML(QDomElement& xParent, QProgressDialog & progress) = 0;

    virtual CoordBox boundingBox();

    virtual /* const */ LayerType classType() = 0;
    virtual const LayerGroups classGroups() = 0;

    int incDirtyLevel(int inc=1);
    int decDirtyLevel(int inc=1);
    int getDirtyLevel();
    int setDirtyLevel(int newLevel);
    int getDirtySize();

    virtual bool canDelete() const;
    virtual bool isUploadable() const;
    virtual bool isReadonly() const;
    virtual bool isTrack() {return false;}
    virtual bool arePointsDrawable() {return true;}

    void blockVirtualUpdates(bool val);
    bool isVirtualUpdatesBlocked() const;

    virtual void blockIndexing(bool val);
    virtual void indexAdd(const CoordBox& bb, const MapFeaturePtr aFeat);
    virtual void indexRemove(const CoordBox& bb, const MapFeaturePtr aFeat);
    virtual std::deque<Feature*> indexFind(const CoordBox& vp);
    virtual void reIndex();


protected:
    LayerPrivate* p;
    LayerWidget* theWidget;
    mutable QString Id;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Layer::LayerGroups)

class DrawingLayer : public Layer
{
    Q_OBJECT

public:
    DrawingLayer(const QString& aName);
    virtual ~DrawingLayer();

    virtual LayerWidget* newWidget(void);

    virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
    static DrawingLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog & progress);
    static DrawingLayer* doFromXML(DrawingLayer* l, Document* d, const QDomElement e, QProgressDialog & progress);

    virtual /* const */ LayerType classType() {return Layer::DrawingLayerType;}
    virtual const LayerGroups classGroups() {return (Layer::OSM);}
};

class TrackLayer : public Layer
{
    Q_OBJECT
public:
    TrackLayer(const QString& aName="", const QString& filaname="");
    virtual ~TrackLayer();

    virtual LayerWidget* newWidget(void);

    virtual void extractLayer();
    virtual const QString getFilename();

    virtual QString toHtml();
    virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
    static TrackLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog & progress);

    virtual /* const */ LayerType classType() {return Layer::TrackLayerType;}
    virtual const LayerGroups classGroups() {return(Layer::Tracks);}

    virtual bool isUploadable() {return true;}
    virtual bool isTrack() {return true;}

protected:
    QString Filename;
};

class DirtyLayer : public DrawingLayer
{
    Q_OBJECT
public:
    DirtyLayer(const QString& aName);
    virtual ~DirtyLayer();

    static DirtyLayer* fromXML(Document* d, const QDomElement e, QProgressDialog & progress);

    virtual /* const */ LayerType classType() {return Layer::DirtyLayerType;}
    virtual const LayerGroups classGroups() {return(Layer::Default|Layer::OSM);}

    virtual LayerWidget* newWidget(void);

    virtual bool canDelete() const { return false; }

};

class UploadedLayer : public DrawingLayer
{
    Q_OBJECT
public:
    UploadedLayer(const QString& aName);
    virtual ~UploadedLayer();

    static UploadedLayer* fromXML(Document* d, const QDomElement e, QProgressDialog & progress);

    virtual /* const */ LayerType classType() {return Layer::UploadedLayerType;}
    virtual const LayerGroups classGroups() {return(Layer::Default|Layer::OSM);}

    virtual LayerWidget* newWidget(void);

    virtual bool canDelete() const { return false; }
};

class DeletedLayer : public DrawingLayer
{
    Q_OBJECT
public:
    DeletedLayer(const QString& aName);
    virtual ~DeletedLayer();

    virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
    static DeletedLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog & progress);

    virtual /* const */ LayerType classType() {return Layer::DeletedLayerType;}
    virtual const LayerGroups classGroups() {return(Layer::None);}
    virtual LayerWidget* newWidget(void);

    virtual bool isUploadable() {return false;}
    virtual bool canDelete() const { return false; }
};

class OsbLayer : public Layer
{
    Q_OBJECT

    friend class OsbFeatureIterator;

public:
    OsbLayer(const QString& aName);
    OsbLayer(const QString& aName, const QString& filename, bool isWorld = false);
    virtual ~OsbLayer();

    virtual /* const */ LayerType classType() {return Layer::OsbLayerType;}
    virtual const LayerGroups classGroups() {return(Layer::OSM);}
    virtual LayerWidget* newWidget(void);

    void setFilename(const QString& filename);

    virtual bool isUploadable() {return true;}
    virtual bool arePointsDrawable();

    virtual void preload();
    virtual void get(const CoordBox& hz, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, QSet<Way*>& theCoastlines, Document* theDocument,
                               QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform);

    virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
    static OsbLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog & progress);

    virtual QString toHtml();

protected:
    OsbLayerPrivate* pp;

};

Q_DECLARE_METATYPE ( QUuid )

#endif


