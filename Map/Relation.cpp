#include "Map/Relation.h"
#include "Map/Road.h"
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
	return QString("relationship %1").arg(id());
}

RenderPriority Relation::renderPriority(double aPixelPerM) const
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
	for (unsigned int i=0; i<p->Members.size(); ++i)
		if (p->Members[i].second == aFeature)
		{
			if ( (p->Members.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				QString Role = p->Members[i].first;
				theList->add(new RelationRemoveFeatureCommand(this, i));
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					theList->add(new RelationAddFeatureCommand(this, Role, Alternatives[j], i+j));
			}
			return;
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

QString Relation::exportOSM()
{
	QString S;
	S += QString("<relation id=\"%1\">").arg(stripToOSMId(id()));
	for (unsigned int i=0; i<size(); ++i)
	{
		QString Type("node");
		if (dynamic_cast<const Road*>(get(i)))
			Type="way";
		else if (dynamic_cast<const Relation*>(get(i)))
			Type="relation";
		S+=QString("<member type=\"%1\" ref=\"%2\" role=\"%3\"/>").arg(Type).arg(stripToOSMId(get(i)->id())).arg(getRole(i));
	}
	S += tagOSM();
	S += "</relation>";
	return S;
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
	if (index.row() >= Parent->Members.size())
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
		if ( (index.column() == 0) && (index.row() < Parent->Members.size()) )
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
			return "Role";
		else
			return "Member";
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
		CommandList* L = new CommandList;
		L->add(new RelationRemoveFeatureCommand(Parent->theRelation, index.row()));
		L->add(new RelationAddFeatureCommand(Parent->theRelation,value.toString(),Tmp,index.row()));
		Main->document()->history().add(L);
		emit dataChanged(index, index);
		return true;
	}
	return false;
}






