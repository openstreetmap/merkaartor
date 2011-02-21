#include "BookmarkItem.h"

BookmarkItem::BookmarkItem()
    : QStandardItem()
{
    m_bookmark.title = QString();
    m_bookmark.uri = QString();
}

BookmarkItem::BookmarkItem(const QString & text)
    : QStandardItem(text)
{
    m_bookmark.title = text;
    m_bookmark.uri = QString();
}

BookmarkItem::BookmarkItem(const BookmarkItem &other)
    : QStandardItem(other)
{
    m_bookmark = other.m_bookmark;
}

BookmarkItem & BookmarkItem::operator=(const BookmarkItem &other)
{
    if (this != &other) {
        QStandardItem::operator =(other);
        m_bookmark = other.m_bookmark;
    }
    return *this;
}

BookmarkItem::~BookmarkItem()
{
}

MobileBookmark& BookmarkItem::bookmark()
{
    return m_bookmark;
}

BookmarkItem * BookmarkItem::BookmarkItem::clone() const
{
    BookmarkItem* i = new BookmarkItem(m_bookmark.title);
    i->m_bookmark = m_bookmark;

    return i;
}

void BookmarkItem::read(QDataStream &in)
{
    QStandardItem::read(in);
    in >> m_bookmark.title;
    in >> m_bookmark.uri;
}

void BookmarkItem::write(QDataStream &out) const
{
    QStandardItem::write(out);
    out << m_bookmark.title;
    out << m_bookmark.uri;
}

bool BookmarkItem::isFolder()
{
    return m_bookmark.uri.isEmpty();
}
