#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Maps/MapFeature.h"
#include "Maps/MapDocument.h"

class MainWindow;
class RelationPrivate;
class QAbstractTableModel;
class QProgressDialog;
class OsbMapLayer;

class Relation : public MapFeature
{
	Q_OBJECT

	public:
		Relation(void);
		Relation(const Relation&);
		virtual ~Relation(void);

		virtual QString getClass() const {return "Relation";}

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection, const QTransform& theTransform);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, const QTransform& theTransform, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, const QTransform& theTransform, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual RenderPriority renderPriority();

		void add(const QString& Role, MapFeature* Pt);
		void add(const QString& Role, MapFeature* Pt, int Idx);
		virtual void remove(int Idx);
		virtual void remove(MapFeature* F);
		virtual int size() const;
		virtual int find(MapFeature* Pt) const;
		virtual MapFeature* get(int idx);
		virtual const MapFeature* get(int Idx) const;
		virtual bool isNull() const;

		const QString& getRole(int Idx) const;
		QAbstractTableModel* referenceMemberModel(MainWindow* aMain);
		void releaseMemberModel();
		QString description() const;

		virtual void setLayer(MapLayer* aLayer);
		virtual void partChanged(MapFeature* F, int ChangeId);

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QTransform& theTransform, const QRectF& clipRect);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		static Relation* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Relation* fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id);

	private:
		RelationPrivate* p;
};

Q_DECLARE_METATYPE( Relation * );

#endif
