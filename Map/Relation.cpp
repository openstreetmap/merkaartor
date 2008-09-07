#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Projection.h"
#include "MainWindow.h"
#include "Command/DocumentCommands.h"
#include "Command/RelationCommands.h"
#include "Map/MapDocument.h"
#include "Utils/LineF.h"

#include "ExportOSM.h"

#include <QtCore/QAbstractTableModel>
#include <QProgressDialog>
#include <QPainter>

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
		QPainterPath thePath;
};

Relation::Relation()
{
	p = new RelationPrivate(this);
}

Relation::Relation(const Relation& other)
: MapFeature(other)
{
	p = new RelationPrivate(this);
}

Relation::~Relation()
{
	for (unsigned int i=0; i<p->Members.size(); ++i)
		p->Members[i].second->unsetParent(this);
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

void Relation::draw(QPainter& P, const Projection& theProjection)
{
	if (!M_PREFS->getRelationsVisible())
		return;

	P.setPen(QPen(M_PREFS->getRelationsColor(),2,Qt::DashLine));
	P.drawRect(QRectF(theProjection.project(boundingBox().bottomLeft()),theProjection.project(boundingBox().topRight())));
}

void Relation::drawFocus(QPainter& P, const Projection& theProjection)
{
	P.setPen(QPen(M_PREFS->getFocusColor(),2,Qt::DashLine));
	P.drawRect(QRectF(theProjection.project(boundingBox().bottomLeft()),theProjection.project(boundingBox().topRight())));

	for (unsigned int i=0; i<p->Members.size(); ++i)
		p->Members[i].second->drawFocus(P,theProjection);
}

void Relation::drawHover(QPainter& P, const Projection& theProjection)
{
	P.setPen(QPen(M_PREFS->getHoverColor(),2,Qt::DashLine));
	P.drawRect(QRectF(theProjection.project(boundingBox().bottomLeft()),theProjection.project(boundingBox().topRight())));

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

	double D;
	LineF F(theProjection.project(boundingBox().topLeft()),theProjection.project(boundingBox().topRight()));
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(theProjection.project(boundingBox().topLeft()),theProjection.project(boundingBox().bottomLeft()));
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(theProjection.project(boundingBox().bottomRight()),theProjection.project(boundingBox().bottomLeft()));
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(theProjection.project(boundingBox().bottomRight()),theProjection.project(boundingBox().topRight()));
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;

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
				theList->add(new RelationRemoveFeatureCommand(this, i, theDocument->getDirtyOrOriginLayer(layer())));
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					if (i < p->Members.size())
						if (p->Members[i+j].second != Alternatives[j])
							if (p->Members[i+j-1].second != Alternatives[j])
								theList->add(new RelationAddFeatureCommand(this, Role, Alternatives[j], i+j, theDocument->getDirtyOrOriginLayer(Alternatives[j]->layer())));
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


void Relation::add(const QString& Role, MapFeature* F)
{
	p->Members.push_back(std::make_pair(Role,F));
	F->setParent(this);
}

void Relation::add(const QString& Role, MapFeature* F, unsigned int Idx)
{
	p->Members.push_back(std::make_pair(Role,F));
	std::rotate(p->Members.begin()+Idx,p->Members.end()-1,p->Members.end());
	F->setParent(this);
}

void Relation::remove(unsigned int Idx)
{
	MapFeature* F = p->Members[Idx].second;
	F->unsetParent(this);
	p->Members.erase(p->Members.begin()+Idx);
}

void Relation::remove(MapFeature* F)
{
	for (unsigned int i=p->Members.size(); i; --i)
		if (F == p->Members[i-1].second)
			remove(i-1);
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

void Relation::buildPath(Projection const &theProjection, const QRegion& paintRegion)
{
	p->thePath = QPainterPath();
	for (unsigned int i=0; i<size(); ++i)
		if (Road* M = dynamic_cast<Road*>(p->Members[i].second)) {
			M->buildPath(theProjection, paintRegion);
			p->thePath.addPath(M->getPath());
		}
}

QPainterPath Relation::getPath()
{
	return p->thePath;
}


QString Relation::toXML(unsigned int lvl, QProgressDialog * progress)
{
	if (progress)
		progress->setValue(progress->value()+1);

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

bool Relation::toXML(QDomElement xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("relation");
	xParent.appendChild(e);

	e.setAttribute("id", xmlId());
	e.setAttribute("timestamp", time().toString(Qt::ISODate)+"Z");
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

	progress.setValue(progress.value()+1);
	return OK;
}

Relation * Relation::fromXML(MapDocument * d, MapLayer * L, const QDomElement e)
{
	QString id = e.attribute("id");
	if (!id.startsWith('{'))
		id = "rel_" + id;
	QDateTime time = QDateTime::fromString(e.attribute("timestamp").left(19), "yyyy-MM-ddTHH:mm:ss");
	QString user = e.attribute("user");

	Relation* R = dynamic_cast<Relation*>(d->getFeature(id));
	if (!R) {
		R = new Relation;
		R->setId(id);
		R->setLastUpdated(MapFeature::OSMServer);
	} else {
		R->layer()->remove(R);
	}
	R->setTime(time);
	R->setUser(user);

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "member") {
			QString Type = c.attribute("type");
			MapFeature* F = 0;
			if (Type == "node") {
				QString nId = c.attribute("ref");
				if (!nId.startsWith('{'))
					nId = "node_" + nId;
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
				QString rId = c.attribute("ref");
				if (!rId.startsWith('{'))
					rId = "way_" + rId;
				Road* Part = dynamic_cast<Road*>(d->getFeature(rId));
				if (!Part)
				{
					Part = new Road;
					Part->setId(rId);
					Part->setLastUpdated(MapFeature::NotYetDownloaded);
					L->add(Part);
				}
				F = Part;
			} else if (Type == "relation") {
				QString RId = c.attribute("ref");
				if (!RId.startsWith('{'))
					RId = "rel_" + RId;
				Relation* Part = dynamic_cast<Relation*>(d->getFeature(RId));
				if (!Part)
				{
					Part = new Relation;
					Part->setId(RId);
					Part->setLastUpdated(MapFeature::NotYetDownloaded);
					L->add(Part);
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
	D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + QString::number(intToAng(bb.topLeft().lat()), 'f', 4) + " / " + QString::number(intToAng(bb.topLeft().lon()), 'f', 4);
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + QString::number(intToAng(bb.bottomRight().lat()), 'f', 4) + " / " + QString::number(intToAng(bb.bottomRight().lon()), 'f', 4);

	return MapFeature::toMainHtml(QApplication::translate("MapFeature", "Relation"),"relation").arg(D);
}

void Relation::toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex)
{
	quint8 Type;
	quint64 ref;

	theIndex["L" + QString::number(idToLong())] = ds.device()->pos();
	ds << (qint8)'L';
	ds << idToLong();
	ds << size();
	for (unsigned int i=0; i<size(); ++i) {
		if (dynamic_cast<const TrackPoint*>(get(i))) {
			Type='N';
			ref = get(i)->idToLong();
		}
		else if (dynamic_cast<const Road*>(get(i))) {
			Type='R';
			ref = get(i)->idToLong();
		}
		else if (dynamic_cast<const Relation*>(get(i))) {
			Type='L';
			ref = get(i)->idToLong();
		}
//		ds << (qint8) Type << ref << getRole(i);
		ds << Type << get(i)->idToLong() << getRole(i);
	}
}

Relation* Relation::fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id)
{
	Q_UNUSED(c);

	qint32	fSize;
	QString strId;
	quint8 Type;
	qint64 refId;
	QString Role;

	ds >> fSize;

	if (id < 1)
		strId = QString::number(id);
	else
		strId = QString("rel_%1").arg(QString::number(id));

	Relation* R = dynamic_cast<Relation*>(d->getFeature(strId));
	if (!R) {
		R = new Relation();
		R->setId(strId);
		R->setLastUpdated(MapFeature::OSMServer);
		L->add(R);
	} else {
		if (R->lastUpdated() == MapFeature::NotYetDownloaded)
			R->setLastUpdated(MapFeature::OSMServer);
		else  {
			for (int i=0; i < fSize; ++i) {
				ds >> Type;
				ds >> refId;
				ds >> Role;
			}
			return R;
		}
	}

	for (int i=0; i < fSize; ++i) {
		ds >> Type;
		ds >> refId;
		ds >> Role;

		//MapFeature* F = d->getFeature(QString::number(refId), false);
		MapFeature* F;
		switch (Type) {
			case 'N':
				F = L->get(QString("node_%1").arg(refId));
				break;
			case 'R':
				F = L->get(QString("way_%1").arg(refId));
				break;
			case 'L':
				F = L->get(QString("rel_%1").arg(refId));
				break;
		}
		if (F)
			R->add(Role, F);
	}

	return R;
}

/* RELATIONMODEL */

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
		L->add(new RelationRemoveFeatureCommand(Parent->theRelation, index.row(), Main->document()->getDirtyOrOriginLayer(Parent->theRelation->layer())));
		L->add(new RelationAddFeatureCommand(Parent->theRelation,value.toString(),Tmp,index.row(), Main->document()->getDirtyOrOriginLayer(Parent->theRelation->layer())));
		Main->document()->addHistory(L);
		emit dataChanged(index, index);
		return true;
	}
	return false;
}







