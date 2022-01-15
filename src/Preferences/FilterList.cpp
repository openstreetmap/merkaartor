//
// C++ Implementation: FilterList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "FilterList.h"
#include "build-metadata.hpp"

FilterItem::FilterItem ()
    : name(""), filter(""), deleted(false)
{
    id = QUuid::createUuid();
}

FilterItem::FilterItem (QUuid aId, QString aName, QString aFilter, bool aDeleted)
    : id(aId), name(aName), filter(aFilter), deleted(aDeleted)
{
}

void FilterItem::toXml(QDomElement parent)
{
    QDomElement p = parent.ownerDocument().createElement("Filter");
    parent.appendChild(p);
    p.setAttribute("xml:id", id.toString());
    p.setAttribute("name", name);
    if (deleted)
        p.setAttribute("deleted", "true");

    QDomText t = parent.ownerDocument().createTextNode(filter);
    p.appendChild(t);
}

FilterItem FilterItem::fromXml(QDomElement parent)
{
    FilterItem theFilter;

    if (parent.tagName() == "Filter") {
        theFilter.filter = parent.text().trimmed();
        if (parent.hasAttribute("xml:id"))
            theFilter.id = QUuid(parent.attribute("xml:id"));
        else
            theFilter.id = QUuid::createUuid();
        theFilter.name = parent.attribute("name");
        theFilter.deleted = (parent.attribute("deleted") == "true" ? true : false);
    }

    return theFilter;
}

void FiltersList::add(FiltersList aFilterList)
{
    FilterListIterator it(*(aFilterList.getFilters()));
    while (it.hasNext()) {
        it.next();

        FilterItem anItem = it.value();
        if (!theFilters.contains(anItem.name))
            theFilters.insert(anItem.name, anItem);
    }
}

void FiltersList::addFilter(FilterItem aFilter)
{
    theFilters.insert(aFilter.name, aFilter);
}

FilterItem FiltersList::getFilter(QString name) const
{
    if (theFilters.contains(name))
        return theFilters.value(name);
    else {
        QMapIterator <QString, FilterItem> it(theFilters);
        while (it.hasNext()) {
            it.next();

            if (it.key().contains(name, Qt::CaseInsensitive))
                return it.value();
        }
    }
    return FilterItem();
}

QMap <QString, FilterItem>* FiltersList::getFilters()
{
    return &theFilters;
}

void FiltersList::toXml(QDomElement parent)
{
    QDomElement rt = parent.ownerDocument().createElement("Filters");
    parent.appendChild(rt);
    rt.setAttribute("creator", QString("%1 v%2%3").arg(BuildMetadata::PRODUCT).arg(BuildMetadata::VERSION).arg(BuildMetadata::REVISION));

    QMapIterator <QString, FilterItem> it(theFilters);
    while (it.hasNext()) {
        it.next();

        FilterItem i = it.value();
        i.toXml(rt);
    }
}

FiltersList FiltersList::fromXml(QDomElement parent)
{
    FiltersList theFilters;

    if (parent.nodeName() == "Filters") {
        QDomElement c = parent.firstChildElement();
        while(!c.isNull()) {
            if (c.tagName() == "Filter") {
                theFilters.addFilter(FilterItem::fromXml(c));
            }

            c = c.nextSiblingElement();
        }
    }

    return theFilters;
}
