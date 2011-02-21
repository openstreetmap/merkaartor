#ifndef DELEGATE_H
#define DELEGATE_H

#include <QItemDelegate>
#include <QTreeView>
#include <QPointer>

class BookmarksView;

class BookmarksDelegate : public QItemDelegate
{
    Q_OBJECT

    public:

        BookmarksDelegate(BookmarksView *aParent); // the delegate 'owner' has to be specified in the constructor - it's used to obtain visualRect of selected item/index
        ~BookmarksDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool isPointFromRect(const QPoint &aPoint, const QRect &aRect) const;

private:
        bool hasParent( const QModelIndex &index ) const;
        bool isFolder(const QModelIndex &index) const;
        bool isLast( const QModelIndex &index ) const;
        bool isExpanded( const QModelIndex &index ) const;

    private:
        QPointer<BookmarksView> mViewPtr;
};

#endif /* DELEGATE_H */

