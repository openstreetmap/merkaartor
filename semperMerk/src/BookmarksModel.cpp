#include "BookmarksModel.h"
#include "BookmarkItem.h"

BookmarksModel::BookmarksModel(QObject* parent)
    : QStandardItemModel(parent)
{
    setItemPrototype(new BookmarkItem);
}

BookmarksModel::BookmarksModel(int rows, int columns, QObject* parent)
    : QStandardItemModel(rows, columns, parent)
{
}

