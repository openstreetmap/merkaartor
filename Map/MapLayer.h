#ifndef MAPLAYER_H_
#define MAPLAYER_H_

#include "Map/MapDocument.h"

class QString;
class MapFeature;
class MapLayerPrivate;
class MapAdapter;
class Layer;
class LayerManager;
class LayerWidget;

class MapLayer
{
public:
	MapLayer(const QString& aName);

private:
	MapLayer(const MapLayer& aLayer);

public:
	virtual ~MapLayer();

	void setName(const QString& aName);
	const QString& name() const;
	bool isVisible() const;
	bool isSelected() const;

	void add(MapFeature* aFeature);
	void add(MapFeature* aFeature, unsigned int Idx);
	void remove(MapFeature* aFeature);
	bool exists(MapFeature* aFeature) const;
	unsigned int size() const;
	MapFeature* get(unsigned int i);
	const MapFeature* get(unsigned int i) const;
	MapFeature* get(const QString& id);
	void notifyIdUpdate(const QString& id, MapFeature* aFeature);
	void sortRenderingPriority(double PixelPerM);
	void invalidateRenderPriority();

	void setDocument(MapDocument* aDocument);
	MapDocument* getDocument();

	LayerWidget* getWidget(void);
	virtual void updateWidget() {};

	virtual void setVisible(bool b) = 0;
	virtual void setSelected(bool b);
	virtual LayerWidget* newWidget(void) = 0;

protected:
	MapLayerPrivate* p;
};

class DrawingMapLayer : public MapLayer
{
public:
	DrawingMapLayer(const QString& aName);
	virtual ~DrawingMapLayer();

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);
};

class ImageMapLayer : public MapLayer, public QObject
{
public:
	ImageMapLayer(const QString& aName);
	virtual ~ImageMapLayer();

	Layer* imageLayer();
	void setMapAdapter(ImageBackgroundType typ);
	LayerManager* layermanager;

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);
	virtual void updateWidget();
};

class TrackMapLayer : public MapLayer
{
public:
	TrackMapLayer(const QString& aName);
	virtual ~TrackMapLayer();

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);

	void extractLayer();
};

#endif


