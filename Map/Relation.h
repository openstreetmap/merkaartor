#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Map/MapFeature.h"
#include "Map/MapDocument.h"

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
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		void add(const QString& Role, MapFeature* Pt);
		void add(const QString& Role, MapFeature* Pt, unsigned int Idx);
		virtual void remove(unsigned int Idx);
		virtual void remove(MapFeature* F);
		virtual unsigned int size() const;
		virtual unsigned int find(MapFeature* Pt) const;
		virtual MapFeature* get(unsigned int idx);
		virtual const MapFeature* get(unsigned int Idx) const;
		virtual bool isNull() const;

		const QString& getRole(unsigned int Idx) const;
		QAbstractTableModel* referenceMemberModel(MainWindow* aMain);
		void releaseMemberModel();
		QString description() const;

		virtual void setLayer(MapLayer* aLayer);
		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		QPainterPath getPath();
		void buildPath(Projection const &theProjection, const QRegion& paintRegion);

		virtual QString toXML(unsigned int lvl=0, QProgressDialog * progress=NULL);
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
