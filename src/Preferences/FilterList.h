//
// C++ Interface: FilterList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FILTER_LIST_H
#define FILTER_LIST_H

#include <QString>
#include <QMap>
#include <QtXml>

class FilterItem
{
    public:
        FilterItem ();
        FilterItem (QUuid aId, QString aName, QString aProjection, bool aDeleted = false);
        QUuid id;
        QString name;
        QString filter;
        bool deleted;

    public:
        void toXml(QDomElement parent);
        static FilterItem fromXml(QDomElement parent);
};

typedef QMap <QString, FilterItem> FilterList;
typedef QMapIterator<QString, FilterItem> FilterListIterator;

class FiltersList
{
    public:
        void add(FiltersList aFilterList);
        void addFilter(FilterItem aProjItem);
        FilterItem getFilter(QString name) const;
        FilterList* getFilters();
        void toXml(QDomElement parent);
        static FiltersList fromXml(QDomElement parent);

    private:
        FilterList theFilters;
};

#endif // FILTER_LIST_H
