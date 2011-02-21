#ifndef BOOKMARKITEM_H
#define BOOKMARKITEM_H

#include "BookmarksModel.h"

struct MobileBookmark
{
    QString title;
    QString uri;
};

class BookmarkItem : public QObject, public QStandardItem
{
    Q_OBJECT
    Q_PROPERTY(bool isFolder READ isFolder)

    friend class BookmarksModel;

public:
    BookmarkItem();
    BookmarkItem(const QString & text);
    BookmarkItem(const BookmarkItem &other);
    ~BookmarkItem();

    int type () const { return QStandardItem::UserType + 1; }
    BookmarkItem * clone () const;

public:
    MobileBookmark& bookmark();

    BookmarkItem &operator=(const BookmarkItem &other);
    bool isFolder();

    void read(QDataStream &in);
    void write(QDataStream &out) const;
private:
    MobileBookmark m_bookmark;
};

#endif // BOOKMARKITEM_H
