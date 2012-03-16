#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Document.h"
#include "Feature.h"

class MainWindow;
class RelationPrivate;
class QAbstractTableModel;
class QProgressDialog;

class Relation : public Feature
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

protected:
    Relation(void);
    Relation(const Relation&);
    virtual ~Relation(void);

public:
    virtual QString getClass() const {return "Relation";}
    virtual char getType() const {return IFeature::OsmRelation;}
    virtual void updateMeta();

    virtual const CoordBox& boundingBox(bool update=true) const;
    virtual void draw(QPainter& P, MapView* theView);
    virtual void drawSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawParentsSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawChildrenSpecial(QPainter& P, QPen& Pen, MapView* theView, int depth);

    virtual qreal pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const;
    virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives);
    virtual bool notEverythingDownloaded();
    virtual const RenderPriority& renderPriority();

    void add(const QString& Role, Feature* Pt);
    void add(const QString& Role, Feature* Pt, int Idx);
    virtual void remove(int Idx);
    virtual void remove(Feature* F);
    virtual int size() const;
    virtual int find(Feature* Pt) const;
    virtual Feature* get(int idx);
    virtual const Feature* get(int Idx) const;
    virtual bool isNull() const;

    const QString& getRole(int Idx) const;
    QAbstractTableModel* referenceMemberModel(MainWindow* aMain);
    void releaseMemberModel();
    QString description() const;

    virtual void setLayer(Layer* aLayer);
    virtual void partChanged(Feature* F, int ChangeId);

    const QPainterPath& getPath() const;
    void buildPath(Projection const &theProjection);

    virtual bool toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict=false, QString changetsetid="");
    static Relation* fromXML(Document* d, Layer* L, QXmlStreamReader& stream);

    virtual QString toHtml();

    qreal widthOf();

private:
    RelationPrivate* p;
};

Q_DECLARE_METATYPE( Relation * );

#endif
