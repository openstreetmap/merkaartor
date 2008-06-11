#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "MainWindow.h"
#include "Command/DocumentCommands.h"
#include "Command/RelationCommands.h"
#include "Map/MapDocument.h"

#include "ExportOSM.h"

#include <QtCore/QAbstractTableModel>

#include <algorithm>
#include <utility>
#include <vector>

class RelationMemberModel : public QAbstractTableModel
{
	public:
		RelationMemberModel(RelationPrivate* aParent, MainWindow* aMain);
		int rowCount(const QModelIndex &parent = QModelIndex()) const;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

		RelationPrivate* Parent;
		MainWindow* Main;
};

class RelationPrivate
{
	public:
		RelationPrivate(Relation* R) : theRelation(R), theModel(0), ModelReferences(0)
		{
		}
		~RelationPrivate()
		{
			delete theModel;
		}
		Relation* theRelation;
		std::vector<std::pair<QString, MapFeature*> > Members;
		RelationMemberModel* theModel;
		unsigned int ModelReferences;
};

Relation::Relation()
{
	p = new RelationPrivate(this);
}

Relation::Relation(const Relation& other)
: MapFeature(other), p(0)
{
}

Relation::~Relation()
{
	delete p;
}

void Relation::setLayer(MapLayer* L)
{
	if (L)
		for (unsigned int i=0; i<p->Members.size(); ++i)
			p->Members[i].second->setParent(this);
	else
		for (unsigned int i=0; i<p->Members.size(); ++i)
			p->Members[i].second->unsetParent(this);
	MapFeature::setLayer(L);
}

void Relation::partChanged(MapFeature*, unsigned int ChangeId)
{
	notifyParents(ChangeId);
}

QString Relation::description() const
{
	return QString(QApplication::translate("MapFeature", "relationship %1")).arg(id());
}

RenderPriority Relation::renderPriority(double /* aPixelPerM */) const
{
	return RenderPriority(RenderPriority::IsLinear,0);
}

CoordBox Relation::boundingBox() const
{
	if (p->Members.size() == 0)
		return CoordBox(Coord(0,0),Coord(0,0));
	else
	{
		CoordBox Clip(p->Members[0].second->boundingBox());
		for (unsigned int i=1; i<p->Members.size(); ++i)
			Clip.merge(p->Members[i].second->boundingBox());
		return Clip;
	}
}

void Relation::draw(QPainter&, const Projection&)
{
}

void Relation::drawFocus(QPainter& P, const Projection& theProjection)
{
	for (unsigned int i=0; i<p->Members.size(); ++i)
		p->Members[i].second->drawFocus(P,theProjection);
}

void Relation::drawHover(QPainter& P, const Projection& theProjection)
{
	for (unsigned int i=0; i<p->Members.size(); ++i)
		p->Members[i].second->drawHover(P,theProjection);
}

double Relation::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	double Best = 1000000;
	for (unsigned int i=0; i<p->Members.size(); ++i)
	{
		double Dist = p->Members[i].second->pixelDistance(Target, ClearEndDistance, theProjection);
		if (Dist < Best)
			Best = Dist;
	}
	return Best + 0.1; // Make sure we select simple elements first
}

void Relation::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives)
{
	for (unsigned int i=0; i<p->Members.size();) {
		if (p->Members[i].second == aFeature)
		{
			if ( (p->Members.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				QString Role = p->Members[i].first;
				theList->add(new RelationRemoveFeatureCommand(this, i, theDocument->getDirtyLayer()));
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					if (i < p->Members.size())
						if (p->Members[i+j].second != Alternatives[j])
							if (p->Members[i+j-1].second != Alternatives[j])
								theList->add(new RelationAddFeatureCommand(this, Role, Alternatives[j], i+j, theDocument->getDirtyLayer()));
				continue;
			}
		}
		++i;
	}
}

bool Relation::notEverythingDownloaded() const
{
	if (lastUpdated() == MapFeature::NotYetDownloaded)
		return true;
	for (unsigned int i=0; i<p->Members.size(); ++i)
		if (p->Members[i].second->notEverythingDownloaded())
			return true;
	return false;
}


void Relation::add(const QString& Role, MapFeature* Pt)
{
	p->Members.push_back(std::make_pair(Role,Pt));
}

void Relation::add(const QString& Role, MapFeature* Pt, unsigned int Idx)
{
	p->Members.push_back(std::make_pair(Role,Pt));
	std::rotate(p->Members.begin()+Idx,p->Members.end()-1,p->Members.end());
}

void Relation::remove(unsigned int Idx)
{
	p->Members.erase(p->Members.begin()+Idx);
}

unsigned int Relation::size() const
{
	return p->Members.size();
}

unsigned int Relation::find(MapFeature* Pt) const
{
	for (unsigned int i=0; i<p->Members.size(); ++i)
		if (Pt == p->Members[i].second)
			return i;
	return p->Members.size();
}

MapFeature* Relation::get(unsigned int idx)
{
	return p->Members[idx].second;
}

const MapFeature* Relation::get(unsigned int idx) const
{
	return p->Members[idx].second;
}

const QString& Relation::getRole(unsigned int idx) const
{
	return p->Members[idx].first;
}

QAbstractTableModel* Relation::referenceMemberModel(MainWindow* aMain)
{
	++p->ModelReferences;
	if (!p->theModel)
		p->theModel = new RelationMemberModel(p, aMain);
	return p->theModel;
}

void Relation::releaseMemberModel()
{
	--p->ModelReferences;
	if (p->ModelReferences == 0)
	{
		delete p->theModel;
		p->theModel = 0;
	}
}

QString Relation::toXML(unsigned int lvl)
{
	QString S;
	S += QString(lvl*2, ' ') + QString("<relation id=\"%1\">\n").arg(stripToOSMId(id()));
	for (unsigned int i=0; i<size(); ++i)
	{
		QString Type("node");
		if (dynamic_cast<const Road*>(get(i)))
			Type="way";
		else if (dynamic_cast<const Relation*>(get(i)))
			Type="relation";
		S += QString((lvl+1)*2, ' ') + QString("<member type=\"%1\" ref=\"%2\" role=\"%3\"/>").arg(Type).arg(stripToOSMId(get(i)->id())).arg(getRole(i));
	}
	S += tagsToXML(lvl+1);
	S += QString(lvl*2, ' ') + "</relation>\n";
	return S;
}

bool Relation::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("relation");
	xParent.appendChild(e);

	e.setAttribute("id", xmlId());
	e.setAttribute("timestamp", time().toString(Qt::ISODate));
	e.setAttribute("user", user());

	for (unsigned int i=0; i<size(); ++i) {
		QString Type("node");
		if (dynamic_cast<const Road*>(get(i)))
			Type="way";
		else if (dynamic_cast<const Relation*>(get(i)))
			Type="relation";

		QDomElement n = xParent.ownerDocument().createElement("member");
		e.appendChild(n);

		n.setAttribute("type", Type);
		n.setAttribute("ref", get(i)->xmlId());
		n.setAttribute("role", getRole(i));
	}

	tagsToXML(e);

	return OK;
}

Relation * Relation::fromXML(MapDocument * d, MapLayer * L, const QDomElement e)
{
	QString id = "rel_"+e.attribute("id");
	QDateTime time = QDateTime::fromString(e.attribute("timestamp"), Qt::ISODate);
	QString user = e.attribute("user");

	Relation* R = dynamic_cast<Relation*>(d->getFeature(id));
	if (!R) {
		R = new Relation;
		R->setId(id);
		R->setLastUpdated(MapFeature::OSMServer);
	} else {
		if (R->layer() != L) {
			R->layer()->remove(R);
		}
	}
	R->setTime(time);
	R->setUser(user);

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "member") {
			QString Type = c.attribute("type");
			MapFeature* F = 0;
			if (Type == "node") {
				QString nId = "node_"+c.attribute("ref");
				TrackPoint* Part = dynamic_cast<TrackPoint*>(d->getFeature(nId));
				if (!Part)
				{
					Part = new TrackPoint(Coord(0,0));
					Part->setId(nId);
					Part->setLastUpdated(MapFeature::NotYetDownloaded);
					L->add(Part);
				}
				F = Part;
			} else if (Type == "way") {
				QString rId = "way_"+c.attribute("ref");
				Road* Part = dynamic_cast<Road*>(d->getFeature(rId));
				if (!Part)
				{
					Part = new Road;
					Part->setId(rId);
					Part->setLastUpdated(MapFeature::NotYetDownloaded);
				}
				F = Part;
			} else if (Type == "relation") {
				QString RId = "rel_"+c.attribute("ref");
				Relation* Part = dynamic_cast<Relation*>(d->getFeature(RId));
				if (!Part)
				{
					Part = new Relation;
					Part->setId(RId);
					Part->setLastUpdated(MapFeature::NotYetDownloaded);
				}
				F = Part;
			}
			if (F)
			{
				R->add(e.attribute("role"),F);
			}
		}
		c = c.nextSiblingElement();
	}

	L->add(R);
	MapFeature::tagsFromXML(d, R, e);

	return R;
}

QString Relation::toHtml()
{
	QString D;

	D += "<i>"+QApplication::translate("MapFeature", "size")+": </i>" + QString::number(size()) + " nodes";
	CoordBox bb = boundingBox();
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + QString::number(radToAng(bb.topLeft().lat()), 'f', 4) + " / " + QString::number(radToAng(bb.topLeft().lon()), 'f', 4);
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + QString::number(radToAng(bb.bottomRight().lat()), 'f', 4) + " / " + QString::number(radToAng(bb.bottomRight().lon()), 'f', 4);

	return MapFeature::toMainHtml(QApplication::translate("MapFeature", "Relation"),"relation").arg(D);
}

RelationMemberModel::RelationMemberModel(RelationPrivate *aParent, MainWindow* aMain)
: Parent(aParent), Main(aMain)
{
}

int RelationMemberModel::rowCount(const QModelIndex &) const
{
	return Parent->Members.size();
}

int RelationMemberModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QVariant RelationMemberModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (index.row() >= (int)Parent->Members.size())
		return QVariant();
	if (role == Qt::DisplayRole)
	{
		if (index.column() == 0)
			return Parent->Members[index.row()].first;
		else
			return Parent->Members[index.row()].second->description();
	}
	else if (role == Qt::EditRole)
	{
		if ( (index.column() == 0) && (index.row() < (int)Parent->Members.size()) )
			return Parent->Members[index.row()].first;
	}
	return QVariant();
}

QVariant RelationMemberModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	if (orientation == Qt::Horizontal)
	{
		if (section == 0)
			return QApplication::translate("MapFeature", "Role");
		else
			return QApplication::translate("MapFeature", "Member");
	}
	return QVariant();
}

Qt::ItemFlags RelationMemberModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;
	if (index.column() == 0)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable  | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	return QAbstractTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool RelationMemberModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		MapFeature* Tmp = Parent->Members[index.row()].second;
		CommandList* L = new CommandList(MainWindow::tr("Relation Modified %1").arg(Parent->theRelation->id()), Parent->theRelation);
		L->add(new RelationRemoveFeatureCommand(Parent->theRelation, index.row(), Main->document()->getDirtyLayer()));
		L->add(new RelationAddFeatureCommand(Parent->theRelation,value.toString(),Tmp,index.row(), Main->document()->getDirtyLayer()));
		Main->document()->addHistory(L);
		emit dataChanged(index, index);
		return true;
	}
	return false;
}







