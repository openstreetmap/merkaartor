//
// C++ Interface: BookmarksList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef BOOKMARKS_LIST_H
#define BOOKMARKS_LIST_H

#include <QString>
#include <QMap>
#include <QtXml>

#include "Maps/Coord.h"

class Bookmark
{
	public:
		Bookmark();
		Bookmark(QString aName, CoordBox aCoord, bool Deleted=false);

		void toXml(QDomElement parent);
		static Bookmark fromXml(QDomElement parent);

	public:
		QString Name;
		CoordBox Coordinates;
		bool deleted;
};
typedef QMap<QString, Bookmark> BookmarkList;
typedef QMapIterator<QString, Bookmark> BookmarkListIterator;

class BookmarksList
{
	public:
		void add(BookmarksList aBookmarksList);
		void addBookmark(Bookmark aBookmark);
		bool contains(QString name) const;
		BookmarkList* getBookmarks();
		Bookmark getBookmark(QString name) const;
		void toXml(QDomElement parent);
		static BookmarksList fromXml(QDomElement parent);

	private:
		BookmarkList theBookmarks;
};

#endif // BOOKMARKS_LIST_H
