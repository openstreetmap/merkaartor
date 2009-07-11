#ifndef MAPLAYER_H_
#define MAPLAYER_H_

#include "Maps/MapTypedef.h"
#include "Maps/Coord.h"

#include <QProgressDialog>

#include <ggl/index/rtree/rtree.hpp>

typedef ggl::index::rtree::rtree<CoordBox, Coord, MapFeaturePtr> MyRTree;

class QString;
class QprogressDialog;

class MapFeature;
class MapLayerPrivate;
class MapAdapter;
class Layer;
class LayerManager;
class LayerWidget;
class WMSMapAdapter;
class TileMapAdapter;
class TrackSegment;
class OsbMapLayerPrivate;
class IMapAdapter;
class MapDocument;

class MapLayer : public QObject
{
	Q_OBJECT

public:
	MapLayer();
	MapLayer(const QString& aName);

private:
	MapLayer(const MapLayer& aLayer);

public:
	typedef enum {
		UndefinedType,
		DeletedMapLayerType,
		DirtyMapLayerType,
		DrawingMapLayerType,
		ExtractedMapLayerType,
		ImageMapLayerType,
		OsbMapLayerType,
		TrackMapLayerType,
		UploadedMapLayerType
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
	virtual ~MapLayer();

	void setName(const QString& aName);
	const QString& name() const;
	void setDescription(const QString& aDesc);
	const QString& description() const;
	bool isVisible() const;
	bool isSelected() const;
	bool isEnabled() const;

	virtual void add(MapFeature* aFeature);
	virtual void add(MapFeature* aFeature, int Idx);
	virtual void remove(MapFeature* aFeature);
	virtual void deleteFeature(MapFeature* aFeature);
	virtual void clear();
	bool exists(MapFeature* aFeature) const;
	virtual int size() const;
	int get(MapFeature* aFeature);
	QList<MapFeature *> get();
	MapFeature* get(int i);
	const MapFeature* get(int i) const;
	MapFeature* get(const QString& id, bool exact=true);
	void notifyIdUpdate(const QString& id, MapFeature* aFeature);
	void sortRenderingPriority();

	virtual void invalidate(MapDocument*, CoordBox) {}
	void invalidateRenderPriority();

	void setDocument(MapDocument* aDocument);
	MapDocument* getDocument();

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

	CoordBox boundingBox();

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

	virtual MyRTree* getRTree();
	virtual void reIndex();


protected:
	MapLayerPrivate* p;
	LayerWidget* theWidget;
	mutable QString Id;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapLayer::LayerGroups)

class DrawingMapLayer : public MapLayer
{
	Q_OBJECT

public:
	DrawingMapLayer(const QString& aName);
	virtual ~DrawingMapLayer();

	virtual LayerWidget* newWidget(void);

	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static DrawingMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);
	static DrawingMapLayer* doFromXML(DrawingMapLayer* l, MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::DrawingMapLayerType;}
    virtual const LayerGroups classGroups() {return (MapLayer::OSM);}
};

class TrackMapLayer : public MapLayer
{
	Q_OBJECT
public:
	TrackMapLayer(const QString& aName="", const QString& filaname="");
	virtual ~TrackMapLayer();

	virtual LayerWidget* newWidget(void);

	virtual void extractLayer();
	virtual const QString getFilename();

	virtual QString toHtml();
	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static TrackMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::TrackMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::Tracks);}

	virtual bool isUploadable() {return true;}
	virtual bool isTrack() {return true;}

protected:
	QString Filename;
};

class DirtyMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	DirtyMapLayer(const QString& aName);
	virtual ~DirtyMapLayer();

	static DirtyMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::DirtyMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::Default|MapLayer::OSM);}

	virtual LayerWidget* newWidget(void);

	virtual bool canDelete() const { return false; }

};

class UploadedMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	UploadedMapLayer(const QString& aName);
	virtual ~UploadedMapLayer();

	static UploadedMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::UploadedMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::Default|MapLayer::OSM);}

	virtual LayerWidget* newWidget(void);

	virtual bool canDelete() const { return false; }
};

class DeletedMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	DeletedMapLayer(const QString& aName);
	virtual ~DeletedMapLayer();

	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static DeletedMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::DeletedMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::None);}
	virtual LayerWidget* newWidget(void);

	virtual bool isUploadable() {return false;}
	virtual bool canDelete() const { return false; }
};

class OsbMapLayer : public MapLayer
{
	Q_OBJECT
public:
	OsbMapLayer(const QString& aName);
	OsbMapLayer(const QString& aName, const QString& filename);
	virtual ~OsbMapLayer();

	virtual /* const */ LayerType classType() {return MapLayer::OsbMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::OSM);}
	virtual LayerWidget* newWidget(void);

	void setFilename(const QString& filename);

	virtual bool isUploadable() {return true;}
	virtual bool arePointsDrawable();

	virtual void invalidate(MapDocument* d, CoordBox vp);
	//MapFeature*  getFeatureByRef(MapDocument* d, quint64 ref);

	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static OsbMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);

	virtual QString toHtml();

public:
	QMap < MapFeature*, quint32 > featRefCount;

protected:
	OsbMapLayerPrivate* pp;

};

Q_DECLARE_METATYPE ( QUuid )

#endif


