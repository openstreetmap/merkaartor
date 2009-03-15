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

void ProjectionsList::addProjection(QString name, QString projCommand)
{
	theProjections.insert(name, projCommand);
}

QString ProjectionsList::getProjection(QString name) const
{

	QString theProj = theProjections.value(name, "__NULL__");
	if (theProj != "__NULL__")
		return theProj;
	else {
		QMapIterator <QString, QString> it(theProjections);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return it.value();
		}
	}
	return QString("+init=%1").arg(name);
}

QMap <QString, QString> ProjectionsList::getProjections() const
{
	return theProjections;
}

void ProjectionsList::toXml(QDomElement parent)
{
	QDomElement rt = parent.ownerDocument().createElement("Projections");
	parent.appendChild(rt);

	QMapIterator <QString, QString> it(theProjections);
	while (it.hasNext()) {
		it.next();

		QDomElement p = parent.ownerDocument().createElement("Projection");
		rt.appendChild(p);
		p.setAttribute("name", it.key());

		QDomText t = parent.ownerDocument().createTextNode(it.value());
		p.appendChild(t);
	}
}

ProjectionsList ProjectionsList::fromXml(QDomElement parent)
{
	ProjectionsList theProjections;

	if (parent.nodeName() == "Projections") {
		QDomElement c = parent.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "Projection") {
				QString theProj = c.text().trimmed();
				//QString theProj = QString("%1 +a=%2 +b=%3").arg(c.text().trimmed()).arg(double(INT_MAX)/M_PI).arg(double(INT_MAX)/M_PI);
				theProjections.addProjection(c.attribute("name"), theProj);
			} 

			c = c.nextSiblingElement();
		}
	}

	return theProjections;
}
