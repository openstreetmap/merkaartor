#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Map/MapFeature.h"
#include "Map/MapDocument.h"

class MainWindow;
class RelationPrivate;
class QAbstractTableModel;

class Relation : public MapFeature
{
	public:
		Relation(void);
		virtual ~Relation(void);

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual void drawHover(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual RenderPriority renderPriority(double aPixelPerM) const;

		void add(const QString& Role, MapFeature* Pt);
		void add(const QString& Role, MapFeature* Pt, unsigned int Idx);
		void remove(unsigned int Idx);
		unsigned int size() const;
		unsigned int find(MapFeature* Pt) const;
		MapFeature* get(unsigned int idx);
		const MapFeature* get(unsigned int Idx) const;
		const QString& getRole(unsigned int Idx) const;
		QAbstractTableModel* referenceMemberModel(MainWindow* aMain);
		void releaseMemberModel();
		QString description() const;

		virtual void setLayer(MapLayer* aLayer);
		virtual void partChanged(MapFeature* F, unsigned int ChangeId);

		virtual QString toXML(unsigned int lvl=0);
		virtual bool toXML(QDomElement xParent);
		static Relation* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();

	private:
		Relation(const Relation&);
		RelationPrivate* p;
};

#endif
