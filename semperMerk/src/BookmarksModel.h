#ifndef BOOKMARKSMODEL_H
#define BOOKMARKSMODEL_H

#include <QStandardItemModel>

class BookmarkItem;

class BookmarksModel : public QStandardItemModel
{
    Q_OBJECT

public:
    BookmarksModel(QObject * parent = 0);
    BookmarksModel(int rows, int columns, QObject * parent = 0);

public:
};

#endif // BOOKMARKSMODEL_H
