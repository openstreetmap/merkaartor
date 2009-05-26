//
// C++ Implementation: ProjectionsList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "ProjectionsList.h"

#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

ProjectionItem::ProjectionItem ()
	: name(""), projection(""), deleted(false)
{
}

ProjectionItem::ProjectionItem (QString aName, QString aProjection, bool aDeleted)
	: name(aName), projection(aProjection), deleted(aDeleted)
{
}

void ProjectionItem::toXml(QDomElement parent)
{
	QDomElement p = parent.ownerDocument().createElement("Projection");
	parent.appendChild(p);
	p.setAttribute("name", name);
	if (deleted)
		p.setAttribute("deleted", "true");

	QDomText t = parent.ownerDocument().createTextNode(projection);
	p.appendChild(t);
}

ProjectionItem ProjectionItem::fromXml(QDomElement parent)
{
	ProjectionItem theProjection;

	if (parent.tagName() == "Projection") {
		theProjection.projection = parent.text().trimmed();
		theProjection.name = parent.attribute("name");
		theProjection.deleted = (parent.attribute("deleted") == "true" ? true : false);
	}

	return theProjection;
}

void ProjectionsList::add(ProjectionsList aProjectionsList)
{
	QMapIterator <QString, ProjectionItem> it(aProjectionsList.getProjections());
	while (it.hasNext()) {
		it.next();

		ProjectionItem anItem = it.value();
		theProjections.insert(anItem.name, anItem);
	}
}

void ProjectionsList::addProjection(ProjectionItem aProjection)
{
	theProjections.insert(aProjection.name, aProjection);
}

QString searchEPSG(QString name, QString filename)
{
	QString ret;
	QString espgNum = name.remove("epsg:", Qt::CaseInsensitive);
	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	if (!f.isOpen())
		return ret;

	QString theEpsgNum("<" + espgNum + ">");
	while (!f.atEnd()) {
		QByteArray epsg = f.readLine();

		int idx = epsg.indexOf(theEpsgNum);
		if (idx != -1) {
			idx += theEpsgNum.length();
			int idx2 = epsg.indexOf("<>", idx);
			ret = epsg.mid(idx, idx2 - idx);
			break;
		}
	}
	f.close();

	return ret;
}

ProjectionItem ProjectionsList::getProjection(QString name) const
{
	if (name.contains("+proj")) {
		return ProjectionItem(name, name);
	}
	if (theProjections.contains(name))
		return theProjections.value(name);
	else {
		QMapIterator <QString, ProjectionItem> it(theProjections);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return it.value();
		}
	}
	QString theProj;
	if (name.startsWith("epsg:", Qt::CaseInsensitive)) {
		theProj = searchEPSG(name, ":/proj/epsg");
		if (!theProj.isEmpty())
			return ProjectionItem(name, theProj);
		theProj = searchEPSG(name, QString(STRINGIFY(SHARE_DIR)) + "/proj/epsg");
		if (!theProj.isEmpty())
			return ProjectionItem(name, theProj);
	}
	return ProjectionItem(name, QString("+init=%1").arg(name));
}

QMap <QString, ProjectionItem> ProjectionsList::getProjections() const
{
	return theProjections;
}

void ProjectionsList::toXml(QDomElement parent)
{
	QDomElement rt = parent.ownerDocument().createElement("Projections");
	parent.appendChild(rt);
	rt.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

	QMapIterator <QString, ProjectionItem> it(theProjections);
	while (it.hasNext()) {
		it.next();

		ProjectionItem i = it.value();
		i.toXml(rt);
	}
}

ProjectionsList ProjectionsList::fromXml(QDomElement parent)
{
	ProjectionsList theProjections;

	if (parent.nodeName() == "Projections") {
		QDomElement c = parent.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "Projection") {
				theProjections.addProjection(ProjectionItem::fromXml(c));
			} 

			c = c.nextSiblingElement();
		}
	}

	return theProjections;
}
