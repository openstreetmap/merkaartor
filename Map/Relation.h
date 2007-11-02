#ifndef MERKAARTOR_RELATIONS_H_
#define MERKAARTOR_RELATIONS_H_

#include "Map/MapFeature.h"

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
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;

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

	private:
		Relation(const Relation&);
		RelationPrivate* p;
};

#endif