//
// C++ Implementation: BookmarksList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>

#include "BookmarksList.h"

Bookmark::Bookmark()
{
	Bookmark(QApplication::translate("MerkaartorPreferences","New Bookmark"), CoordBox());
}

Bookmark::Bookmark(QString aName, CoordBox aCoord, bool Deleted)
	: Name(aName), Coordinates(aCoord), deleted(Deleted)
{
	if (Name == "") {
		Name = QApplication::translate("MerkaartorPreferences","New Bookmark");
	}
}

void Bookmark::toXml(QDomElement parent)
{
	QDomElement p = parent.ownerDocument().createElement("Bookmark");
	parent.appendChild(p);
	p.setAttribute("name", Name);
	if (deleted)
		p.setAttribute("deleted", "true");

	Coordinates.toXML("Coordinates", p);
}

Bookmark Bookmark::fromXml(QDomElement parent)
{
	Bookmark theBookmark;

	if (parent.tagName() == "Bookmark") {
		theBookmark.Name = parent.attribute("name");
		theBookmark.deleted = (parent.attribute("deleted") == "true" ? true : false);

		theBookmark.Coordinates = CoordBox::fromXML(parent.firstChildElement("Coordinates"));
	}

	return theBookmark;
}

/** **/

void BookmarksList::add(BookmarksList aBookmarksList)
{
	QMapIterator <QString, Bookmark> it(*(aBookmarksList.getBookmarks()));
	while (it.hasNext()) {
		it.next();

		Bookmark anItem = it.value();
		theBookmarks.insert(anItem.Name, anItem);
	}
}

BookmarkList* BookmarksList::getBookmarks()
{
	return &theBookmarks;
}

void BookmarksList::addBookmark(Bookmark aBookmark)
{
	theBookmarks.insert(aBookmark.Name, aBookmark);
}

bool BookmarksList::contains(QString name) const
{
	if (theBookmarks.contains(name))
		return true;
	else {
		BookmarkListIterator it(theBookmarks);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return true;
		}
	}
	return false;
}

Bookmark BookmarksList::getBookmark(QString name) const
{
	if (theBookmarks.contains(name))
		return theBookmarks.value(name);
	else {
		BookmarkListIterator it(theBookmarks);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return it.value();
		}
	}
	return Bookmark();
}

void BookmarksList::toXml(QDomElement parent)
{
	QDomElement rt = parent.ownerDocument().createElement("Bookmarks");
	parent.appendChild(rt);
	rt.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

	BookmarkListIterator it(theBookmarks);
	while (it.hasNext()) {
		it.next();

		Bookmark i = it.value();
		i.toXml(rt);
	}
}

BookmarksList BookmarksList::fromXml(QDomElement parent)
{
	BookmarksList theBookmarksList;

	if (parent.nodeName() == "Bookmarks") {
		QDomElement c = parent.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "Bookmark") {
				theBookmarksList.addBookmark(Bookmark::fromXml(c));
			} 

			c = c.nextSiblingElement();
		}
	}

	return theBookmarksList;
}
