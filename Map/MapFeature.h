#ifndef MERKATOR_MAPFEATURE_H_
#define MERKATOR_MAPFEATURE_H_

#include "Map/Coord.h"
#include "PaintStyle/PaintStyle.h"

#include <QtCore/QString>

#include <vector>

class CommandList;
class MapDocument;
class MapLayer;
class Projection;

class QPointF;
class QPainter;

class MapFeaturePrivate;

class RenderPriority
{
	public:
		typedef enum { IsArea, IsLinear, IsSingular } Class;
		RenderPriority(Class C, double IC)
			: theClass(C), InClassPriority(IC) { }
		bool operator<(const RenderPriority& R) const
		{
			return (theClass < R.theClass) ||
				( (theClass == R.theClass) && (InClassPriority > R.InClassPriority) );
		}
	private:
		Class theClass;
		double InClassPriority;
};

class MapFeature
{
	public:
		typedef enum { User, UserResolved, OSMServer, OSMServerConflict, NotYetDownloaded } ActorType;
		typedef enum { UnknownDirection, BothWays, OneWay, OtherWay } TrafficDirectionType;
	public:
		MapFeature();
		MapFeature(const MapFeature& other);
		virtual ~MapFeature() = 0;

		virtual CoordBox boundingBox() const = 0;
		virtual void draw(QPainter& P, const Projection& theProjection) = 0;
		virtual void drawFocus(QPainter& P, const Projection& theProjection) = 0;
		virtual void drawHover(QPainter& P, const Projection& theProjection) = 0;
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const = 0;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives) = 0;
		virtual bool notEverythingDownloaded() const = 0;

		void setId(const QString& id);
		const QString& id() const;
		QString xmlId() const;
		ActorType lastUpdated() const;
		void setLastUpdated(ActorType A);
		const QDateTime& time() const;
		void setTime(const QDateTime& aTime);
		const QString& user() const;
		void setUser(const QString& aUser);
		virtual void setLayer(MapLayer* aLayer);
		virtual MapLayer* layer();
		int versionNumber() const;
		void setVersionNumber(int vn);
		virtual QString description() const = 0;
		virtual RenderPriority renderPriority(double aPixelPerM) const = 0;

		void setTag(const QString& k, const QString& v);
		void setTag(unsigned int idx, const QString& k, const QString& v);
		void clearTags();
		void clearTag(const QString& k);
		unsigned int tagSize() const;
		unsigned int findKey(const QString& k) const;
		QString tagValue(unsigned int i) const;
		QString tagValue(const QString& k, const QString& Default) const;
		QString tagKey(unsigned int i) const;
		void removeTag(unsigned int i);

		FeaturePainter* getEditPainter(double PixelPerM) const;
		bool hasEditPainter() const;
		void invalidatePainter();

		void setParent(MapFeature* F);
		void unsetParent(MapFeature* F);
		unsigned int sizeParents() const;
		MapFeature* getParent(unsigned int i);
		const MapFeature* getParent(unsigned int i) const;
		virtual void partChanged(MapFeature* F, unsigned int ChangeId) = 0;
		void notifyChanges();
		void notifyParents(unsigned int Id);

		virtual QString toXML(unsigned int lvl=0) = 0;
		virtual bool toXML(QDomElement xParent) = 0;

		virtual QString toMainHtml(QString type, QString systemtype);
		virtual QString toHtml() = 0;

		static TrackPoint* getTrackPointOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id);
		static Road* getWayOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id);
		static Relation* getRelationOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id);

	private:
		MapFeaturePrivate* p;

	protected:
		QString tagsToXML(unsigned int lvl=0);
		bool tagsToXML(QDomElement xParent);
		static void tagsFromXML(MapDocument* d, MapFeature* f, QDomElement e);
		static QString stripToOSMId(const QString& id);

};

void copyTags(MapFeature* Dest, MapFeature* Src);
bool hasOSMId(const MapFeature* aFeature);

#endif


