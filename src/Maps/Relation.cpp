#include "Maps/Relation.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Maps/Projection.h"
#include "MainWindow.h"
#include "Command/DocumentCommands.h"
#include "Command/RelationCommands.h"
#include "Maps/MapDocument.h"
#include "Utils/LineF.h"

#include "ExportOSM.h"

#include <QtCore/QAbstractTableModel>
#include <QProgressDialog>
#include <QPainter>

#include <algorithm>
#include <utility>
#include <QList>

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
		QList<QPair<QString, MapFeaturePtr> > Members;
		RelationMemberModel* theModel;
		int ModelReferences;
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
	for (int i=0; i<p->Members.size(); ++i)
		if (p->Members[i].second)
			p->Members[i].second->unsetParentFeature(this);
	delete p;
}

void Relation::setLayer(MapLayer* L)
{
	if (L)
		for (int i=0; i<p->Members.size(); ++i)
			if (p->Members[i].second)
				p->Members[i].second->setParentFeature(this);
	else
		for (int i=0; i<p->Members.size(); ++i)
			if (p->Members[i].second)
				p->Members[i].second->unsetParentFeature(this);
	MapFeature::setLayer(L);
}

void Relation::partChanged(MapFeature*, int ChangeId)
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
		CoordBox Clip;
		bool haveFirst = false;
		for (int i=0; i<p->Members.size(); ++i)
			if (p->Members[i].second && !p->Members[i].second->notEverythingDownloaded()) {
				if (!haveFirst) {
					Clip = p->Members[i].second->boundingBox();
					haveFirst = true;
				} else
					Clip.merge(p->Members[i].second->boundingBox());
			}
		return Clip;
	}
}

void Relation::draw(QPainter& P, const Projection& theProjection)
{
	Q_UNUSED(theProjection)

	if (!M_PREFS->getRelationsVisible())
		return;

	if (notEverythingDownloaded())
		P.setPen(QPen(Qt::red,M_PREFS->getRelationsWidth(),Qt::DashLine));
	else
		P.setPen(QPen(M_PREFS->getRelationsColor(),M_PREFS->getRelationsWidth(),Qt::DashLine));
	P.drawPath(p->thePath);
}

void Relation::drawFocus(QPainter& P, const Projection& theProjection, bool solid)
{
	QRegion clipRg = QRegion(P.clipRegion().boundingRect().adjusted(-20, -20, 20, 20));
	buildPath(theProjection, clipRg);
	if (!solid) {
		QPen thePen(M_PREFS->getFocusColor(),M_PREFS->getFocusWidth());
		thePen.setDashPattern(getParentDashes());
		P.setPen(thePen);
		P.drawPath(p->thePath);
	} else {
		P.setPen(QPen(M_PREFS->getFocusColor(),M_PREFS->getFocusWidth(),Qt::DashLine));
		P.drawPath(p->thePath);

		for (int i=0; i<p->Members.size(); ++i)
			if (p->Members[i].second && !p->Members[i].second->isDeleted())
				p->Members[i].second->drawFocus(P,theProjection, solid);

		if (M_PREFS->getShowParents()) {
			for (int i=0; i<sizeParents(); ++i)
				if (!getParent(i)->isDeleted())
					getParent(i)->drawFocus(P, theProjection, false);
		}
	}
}

void Relation::drawHover(QPainter& P, const Projection& theProjection, bool solid)
{
	QRegion clipRg = QRegion(P.clipRegion().boundingRect().adjusted(-20, -20, 20, 20));
	buildPath(theProjection, clipRg);
	if (!solid) {
		QPen thePen(M_PREFS->getHoverColor(),M_PREFS->getHoverWidth());
		thePen.setDashPattern(getParentDashes());
		P.setPen(thePen);
		P.drawPath(p->thePath);
	} else {
		P.setPen(QPen(M_PREFS->getHoverColor(),M_PREFS->getHoverWidth(),Qt::DashLine));
		P.drawPath(p->thePath);

		for (int i=0; i<p->Members.size(); ++i)
			if (p->Members[i].second && !p->Members[i].second->isDeleted())
				p->Members[i].second->drawHover(P,theProjection, solid);

		if (M_PREFS->getShowParents()) {
			for (int i=0; i<sizeParents(); ++i)
				if (!getParent(i)->isDeleted())
					getParent(i)->drawHover(P, theProjection, false);
		}
	}
}

double Relation::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	double Best = 1000000;
	for (int i=0; i<p->Members.size(); ++i)
	{
		if (p->Members[i].second) {
			double Dist = p->Members[i].second->pixelDistance(Target, ClearEndDistance, theProjection);
			if (Dist < Best)
				Best = Dist;
		}
	}

	double D;
	QRectF bb = QRectF(theProjection.project(boundingBox().topLeft()),theProjection.project(boundingBox().bottomRight()));
	bb.adjust(-10, -10, 10, 10);

	LineF F(bb.topLeft(), bb.topRight());
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(bb.topLeft(), bb.bottomLeft());
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(bb.bottomRight(), bb.bottomLeft());
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;
	F = LineF(bb.bottomRight(), bb.topRight());
	D = F.capDistance(Target);
	if ((D < ClearEndDistance) && (D<Best)) Best = D;

	return Best + 0.1; // Make sure we select simple elements first
}

void Relation::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives)
{
	for (int i=0; i<p->Members.size();) {
		if (p->Members[i].second && p->Members[i].second == aFeature)
		{
			if ( (p->Members.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				QString Role = p->Members[i].first;
				theList->add(new RelationRemoveFeatureCommand(this, i, theDocument->getDirtyOrOriginLayer(layer())));
				for (int j=0; j<Alternatives.size(); ++j)
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
	for (int i=0; i<p->Members.size(); ++i)
		if (p->Members[i].second)
			if (p->Members[i].second->notEverythingDownloaded())
			return true;
	return false;
}


void Relation::add(const QString& Role, MapFeature* F)
{
	p->Members.push_back(qMakePair(Role,F));
	F->setParentFeature(this);
}

void Relation::add(const QString& Role, MapFeature* F, int Idx)
{
	p->Members.push_back(qMakePair(Role,F));
	std::rotate(p->Members.begin()+Idx,p->Members.end()-1,p->Members.end());
	F->setParentFeature(this);
}

void Relation::remove(int Idx)
{
	if (p->Members[Idx].second) {
		MapFeature* F = p->Members[Idx].second;
		F->unsetParentFeature(this);
	}
	p->Members.erase(p->Members.begin()+Idx);
}

void Relation::remove(MapFeature* F)
{
	for (int i=p->Members.size(); i; --i)
		if (F == p->Members[i-1].second)
			remove(i-1);
}

int Relation::size() const
{
	return p->Members.size();
}

int Relation::find(MapFeature* Pt) const
{
	for (int i=0; i<p->Members.size(); ++i)
		if (Pt == p->Members[i].second)
			return i;
	return p->Members.size();
}

MapFeature* Relation::get(int idx)
{
	return p->Members[idx].second;
}

const MapFeature* Relation::get(int idx) const
{
	return p->Members[idx].second;
}

bool Relation::isNull() const
{
	return (p->Members.size() == 0);
}

const QString& Relation::getRole(int idx) const
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
	QRect bb = QRect(theProjection.project(boundingBox().topLeft()),theProjection.project(boundingBox().bottomRight()));
	bb.adjust(-10, -10, 10, 10);
	QList<QPoint> corners;

	corners << bb.bottomLeft() << bb.topLeft() << bb.topRight() << bb.bottomRight() << bb.bottomLeft();

	bool lastPointVisible = true;
	QPoint lastPoint = corners[0];
	QPoint aP = lastPoint;

	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*10+10;
	QRect clipRect = paintRegion.boundingRect().adjusted(int(-WW-20), int(-WW-20), int(WW+20), int(WW+20));


	if (M_PREFS->getDrawingHack()) {
		if (!clipRect.contains(aP)) {
			aP.setX(qMax(clipRect.left(), aP.x()));
			aP.setX(qMin(clipRect.right(), aP.x()));
			aP.setY(qMax(clipRect.top(), aP.y()));
			aP.setY(qMin(clipRect.bottom(), aP.y()));
			lastPointVisible = false;
		}
	}
	p->thePath.moveTo(aP);
	QPoint firstPoint = aP;

	for (int j=1; j<corners.size(); ++j) {
		aP = corners[j];
		if (M_PREFS->getDrawingHack()) {
			QLine l(lastPoint, aP);
			if (!clipRect.contains(aP)) {
				if (!lastPointVisible) {
					QPoint a, b;
					if (QRectInterstects(clipRect, l, a, b)) {
						p->thePath.lineTo(a);
						lastPoint = aP;
						aP = b;
					} else {
						lastPoint = aP;
						aP.setX(qMax(clipRect.left(), aP.x()));
						aP.setX(qMin(clipRect.right(), aP.x()));
						aP.setY(qMax(clipRect.top(), aP.y()));
						aP.setY(qMin(clipRect.bottom(), aP.y()));
					}
				} else {
					QPoint a, b;
					QRectInterstects(clipRect, l, a, b);
					lastPoint = aP;
					aP = a;
				}
				lastPointVisible = false;
			} else {
				if (!lastPointVisible) {
					QPoint a, b;
					QRectInterstects(clipRect, l, a, b);
					p->thePath.lineTo(a);
				}
				lastPoint = aP;
				lastPointVisible = true;
			}
		}
		p->thePath.lineTo(aP);
	}
}

QPainterPath Relation::getPath()
{
	return p->thePath;
}


QString Relation::toXML(int lvl, QProgressDialog * progress)
{
	if (progress)
		progress->setValue(progress->value()+1);

	QString S;
	S += QString(lvl*2, ' ') + QString("<relation id=\"%1\">\n").arg(stripToOSMId(id()));
	for (int i=0; i<size(); ++i)
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
	e.setAttribute("version", versionNumber());
	if (isDeleted())
		e.setAttribute("deleted","true");

	for (int i=0; i<size(); ++i) {
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
	QDateTime time = QDateTime::fromString(e.attribute("timestamp").left(19), Qt::ISODate);
	QString user = e.attribute("user");
	bool Deleted = (e.attribute("deleted") == "true");
	int Version = e.attribute("version").toInt();

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
	R->setDeleted(Deleted);
	R->setVersionNumber(Version);

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
				R->add(c.attribute("role"),F);
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
	quint8 Type = '\0';
	quint64 ref;

	theIndex["L" + QString::number(idToLong())] = ds.device()->pos();
	ds << (qint8)'L';
	ds << idToLong();
	ds << size();
	for (int i=0; i<size(); ++i) {
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

	if (!L) {
		for (int i=0; i < fSize; ++i) {
			ds >> Type;
			ds >> refId;
			ds >> Role;
		}
		return NULL;
	}

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
				F = d->getFeature(QString("node_%1").arg(refId));
				break;
			case 'R':
				F = d->getFeature(QString("way_%1").arg(refId));
				break;
			case 'L':
				F = d->getFeature(QString("rel_%1").arg(refId));
				break;
    	default: 
				F = NULL;
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
		if ( (index.column() == 0) )
			return Parent->Members[index.row()].first;
	}
	else if (role == Qt::UserRole)
	{
		QVariant v;
		v.setValue((MapFeature *)(Parent->Members[index.row()].second));
		return v;
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







