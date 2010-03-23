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

class ProjectionItem
{
    public:
        ProjectionItem ();
        ProjectionItem (QString aName, QString aProjection, bool aDeleted = false);
        QString name;
        QString projection;
        bool deleted;

    public:
        void toXml(QDomElement parent);
        static ProjectionItem fromXml(QDomElement parent);
};

typedef QMap <QString, ProjectionItem> ProjectionList;
typedef QMapIterator<QString, ProjectionItem> ProjectionListIterator;

class ProjectionsList
{
    public:
        void add(ProjectionsList aProjectionsList);
        void addProjection(ProjectionItem aProjItem);
        ProjectionItem getProjection(QString name) const;
        ProjectionList* getProjections();
        void toXml(QDomElement parent);
        static ProjectionsList fromXml(QDomElement parent);

    private:
        ProjectionList theProjections;
};

#endif // PROJECTIONS_LIST_H
