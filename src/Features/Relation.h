#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Document.h"
#include "Feature.h"

class MainWindow;
class RelationPrivate;
class QAbstractTableModel;
class QProgressDialog;
class OsbLayer;

class Relation : public Feature
{
	Q_OBJECT

	public:
		Relation(void);
		Relation(const Relation&);
		virtual ~Relation(void);

		virtual QString getClass() const {return "Relation";}
		virtual Feature::FeatureType getType() const {return Feature::Relations;}
		virtual void updateMeta();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, MapView* theView);
		virtual void drawFocus(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHover(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHighlight(QPainter& P, MapView* theView, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const;
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

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QTransform& theTransform, const QRectF& clipRect);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static Relation* fromXML(Document* d, Layer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Relation* fromBinary(Document* d, OsbLayer* L, QDataStream& ds, qint8 c, qint64 id);

	private:
		RelationPrivate* p;
};

Q_DECLARE_METATYPE( Relation * );

#endif
