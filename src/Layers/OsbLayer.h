#ifndef OSBLAYER_H_
#define OSBLAYER_H_

#include "Layer.h"

class OsbLayer : public Layer
{
    Q_OBJECT

    friend class OsbFeatureIterator;

public:
    OsbLayer(const QString& aName);
    OsbLayer(const QString& aName, const QString& filename, bool isWorld = false);
    virtual ~OsbLayer();

    virtual /* const */ LayerType classType() const {return Layer::OsbLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Draw);}
    virtual LayerWidget* newWidget(void);

    void setFilename(const QString& filename);

    virtual bool isUploadable() {return true;}
    virtual bool arePointsDrawable();

    virtual void preload();
    virtual void get(const CoordBox& hz, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, Document* theDocument,
                               QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform);

    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static OsbLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog * progress);

    virtual QString toHtml();

protected:
    OsbLayerPrivate* pp;

};

#endif
