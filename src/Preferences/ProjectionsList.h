//
// C++ Interface: ProjectionsList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PROJECTIONS_LIST_H
#define PROJECTIONS_LIST_H

#include <QString>
#include <QMap>
#include <QtXml>

class ProjectionsList
{
	public:
		void addProjection(QString name, QString projCommand);
		QString getProjection(QString name) const;
		QMap <QString, QString> getProjections() const;
		void toXml(QDomElement parent);
		static ProjectionsList fromXml(QDomElement parent);

	private:
		QMap <QString, QString> theProjections;
};

#endif // PROJECTIONS_LIST_H
