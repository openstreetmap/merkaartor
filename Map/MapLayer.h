#ifndef MAPLAYER_H_
#define MAPLAYER_H_

#include "Map/MapTypedef.h"
#include "Map/MapDocument.h"

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

class MapLayer : public QObject
{
	Q_OBJECT

public:
	MapLayer();
	MapLayer(const QString& aName);

private:
	MapLayer(const MapLayer& aLayer);

public:
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

	void add(MapFeature* aFeature);
	void add(MapFeature* aFeature, unsigned int Idx);
	virtual void remove(MapFeature* aFeature);
	virtual void clear();
	bool exists(MapFeature* aFeature) const;
	unsigned int size() const;
	int get(MapFeature* aFeature);
	QVector<MapFeature *> get();
	MapFeature* get(unsigned int i);
	const MapFeature* get(unsigned int i) const;
	MapFeature* get(const QString& id, bool exact=true);
	void notifyIdUpdate(const QString& id, MapFeature* aFeature);
	void sortRenderingPriority(double PixelPerM);

	virtual void invalidate(MapDocument*, CoordBox) {};
	void invalidateRenderPriority();

	void setDocument(MapDocument* aDocument);
	MapDocument* getDocument();

	LayerWidget* getWidget(void);
	void deleteWidget(void);
	virtual void updateWidget() {};

	virtual void setVisible(bool b) = 0;
	virtual void setSelected(bool b);
	virtual void setEnabled(bool b);
	virtual LayerWidget* newWidget(void) = 0;

	virtual void setAlpha(const qreal alpha);
	virtual qreal getAlpha() const;

	void setId(const QString& id);
	const QString& id() const;

	virtual QString toMainHtml();
	virtual QString toHtml();
	virtual bool toXML(QDomElement xParent, QProgressDialog & progress) = 0;

	static CoordBox boundingBox(const MapLayer* theLayer);

	virtual const QString className() = 0;
	virtual const LayerGroups classGroups() = 0;

	unsigned int incDirtyLevel(unsigned int inc=1);
	unsigned int decDirtyLevel(unsigned int inc=1);
	unsigned int getDirtyLevel();
	unsigned int setDirtyLevel(unsigned int newLevel);

	virtual bool canDelete();
	virtual bool isUploadable() {return true;};
	virtual bool isTrack() {return false;};

protected:
	MapLayerPrivate* p;
	mutable QString Id;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapLayer::LayerGroups)

class DrawingMapLayer : public MapLayer
{
	Q_OBJECT

public:
	DrawingMapLayer(const QString& aName);
	virtual ~DrawingMapLayer();

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);

	virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
	static DrawingMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);
	static DrawingMapLayer* doFromXML(DrawingMapLayer* l, MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "DrawingMapLayer";};
	virtual const LayerGroups classGroups() {return (MapLayer::OSM);};
};

class ImageMapLayer : public MapLayer
{
	Q_OBJECT
public:
	ImageMapLayer() : layermanager(0) {};
	ImageMapLayer(const QString& aName, LayerManager* aLayerMgr=NULL);
	virtual ~ImageMapLayer();

	Layer* imageLayer();
	void setMapAdapter(ImageBackgroundType typ);
	LayerManager* layermanager;

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);
	virtual void updateWidget();

	virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
	static ImageMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "ImageMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::Default);};

private:
	WMSMapAdapter* wmsa;
	TileMapAdapter* tmsa;
};

class TrackMapLayer : public MapLayer
{
	Q_OBJECT
public:
	TrackMapLayer(const QString& aName="", const QString& filaname="");
	virtual ~TrackMapLayer();

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);

	virtual void extractLayer();
	virtual const QString getFilename();

	virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
	static TrackMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "TrackMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::Tracks);};

	virtual bool isUploadable() {return true;};
	virtual bool isTrack() {return true;};

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

	virtual const QString className() {return "DirtyMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::Default|MapLayer::OSM);};

	virtual LayerWidget* newWidget(void);
};

class UploadedMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	UploadedMapLayer(const QString& aName);
	virtual ~UploadedMapLayer();

	static UploadedMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "UploadedMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::Default|MapLayer::OSM);};
	virtual LayerWidget* newWidget(void);
};

class ExtractedMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	ExtractedMapLayer(const QString& aName);
	virtual ~ExtractedMapLayer();

	static ExtractedMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "ExtractedMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::OSM);};
	virtual LayerWidget* newWidget(void);

	virtual bool isUploadable() {return false;};
};

class DeletedMapLayer : public DrawingMapLayer
{
	Q_OBJECT
public:
	DeletedMapLayer(const QString& aName);
	virtual ~DeletedMapLayer();

	static DeletedMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual const QString className() {return "DeletedMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::None);};
	virtual LayerWidget* newWidget(void);

	virtual bool isUploadable() {return false;};
};

class OsbMapLayer : public MapLayer
{
	Q_OBJECT
public:
	OsbMapLayer(const QString& aName);
	virtual ~OsbMapLayer();

	virtual const QString className() {return "OsbMapLayer";};
	virtual const LayerGroups classGroups() {return(MapLayer::OSM);};
	virtual LayerWidget* newWidget(void);

	virtual void setVisible(bool b);
	virtual bool isUploadable() {return true;};

	virtual void invalidate(MapDocument* d, CoordBox vp);
	//MapFeature*  getFeatureByRef(MapDocument* d, quint64 ref);

	virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
	static OsbMapLayer* fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress);

	virtual QString toHtml();

public:
	QMap < MapFeature*, quint32 > featRefCount;

protected:
	OsbMapLayerPrivate* pp;

};

#endif


