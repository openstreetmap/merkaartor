#include "BookmarksDelegate.h"
#include "BookmarkItem.h"
#include "BookmarksView.h"

#include <QDebug>
#include <QPainter>


#define RADIUS 10
#define SPACER 10

#define scaleFactor1 0.4
#define scaleFactor2 0.8

BookmarksDelegate::BookmarksDelegate(BookmarksView *aParent)
    : QItemDelegate(aParent)
    , mViewPtr(aParent)
{
}

BookmarksDelegate::~BookmarksDelegate()
{
}

void BookmarksDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(!mViewPtr)
        return;

    painter->save();
    QColor bkgrColor = QColor(0xAA,0xAA,0xAA);
    QColor folderColor = QColor(0x33, 0x33, 0x33);

    QPen borderPen(bkgrColor.darker());
    if(!isFolder(index))
    {
        QRadialGradient itemGradient(0.3*(qreal)option.rect.width()+option.rect.left(), -2*(qreal)option.rect.height()+option.rect.top(), 0.8*(qreal)option.rect.width(), 0.3*(qreal)option.rect.width()+option.rect.left(), -2*(qreal)option.rect.height()+option.rect.top());
        itemGradient.setColorAt(0.0, Qt::white);
        itemGradient.setColorAt(1.0, bkgrColor);

        if(isLast(index))
        {
            QPainterPath endPath;
            endPath.moveTo(option.rect.topLeft());
            endPath.lineTo(option.rect.bottomLeft()-QPoint(0, RADIUS));
            endPath.arcTo(option.rect.left(), option.rect.bottom()-2*RADIUS, 2*RADIUS, 2*RADIUS, 180, 90);
            endPath.lineTo(option.rect.bottomRight()-QPoint(RADIUS, 0));
            endPath.arcTo(option.rect.right()-2*RADIUS, option.rect.bottom()-2*RADIUS, 2*RADIUS, 2*RADIUS, 270, 90);
            endPath.lineTo(option.rect.topRight());

            //painter->setBrush( bkgrColor );
            painter->setBrush(itemGradient);
            painter->setPen(borderPen);
            painter->drawPath(endPath);
        }
        else // middle elements
        {
            //painter->setBrush( bkgrColor );
            painter->setBrush(itemGradient);
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);

            painter->setPen(borderPen);
            // vertical lines
            painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
            painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
            // horizontal lines
            painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
        }

        QRectF brect = option.rect;
        brect.setLeft(brect.left()+SPACER);
        painter->setFont(option.font);
        painter->drawText(brect, Qt::AlignVCenter, qVariantValue<QString>(index.data()));
    }
    else // doesn't have parent
    {
        QRadialGradient itemGradient(0.3*(qreal)option.rect.width()+option.rect.left(), -2*(qreal)option.rect.height()+option.rect.top(), 0.8*(qreal)option.rect.width(), 0.3*(qreal)option.rect.width()+option.rect.left(), -2*(qreal)option.rect.height()+option.rect.top());
        itemGradient.setColorAt(0.0, Qt::white);
        itemGradient.setColorAt(1.0, folderColor);

        QPainterPath titlePath;
        if(isExpanded(index))
        {
            titlePath.moveTo(option.rect.bottomLeft());
            titlePath.lineTo(option.rect.topLeft()+QPoint(0, RADIUS));
            titlePath.arcTo(option.rect.left(), option.rect.top(), 2*RADIUS, 2*RADIUS, 180, -90);
            titlePath.lineTo(option.rect.topRight()-QPoint(RADIUS, 0));
            titlePath.arcTo(option.rect.right()-2*RADIUS, option.rect.top(), 2*RADIUS, 2*RADIUS, 90, -90);
            titlePath.lineTo(option.rect.bottomRight());
            titlePath.closeSubpath();
        }
        else
        {
            titlePath.lineTo(option.rect.topLeft()+QPoint(0, RADIUS));
            titlePath.arcTo(option.rect.left(), option.rect.top(), 2*RADIUS, 2*RADIUS, 180, -90);
            titlePath.lineTo(option.rect.topRight()-QPoint(RADIUS, 0));
            titlePath.arcTo(option.rect.right()-2*RADIUS, option.rect.top(), 2*RADIUS, 2*RADIUS, 90, -90);
            titlePath.lineTo(option.rect.bottomRight()-QPoint(0, RADIUS));
            titlePath.arcTo(option.rect.right()-2*RADIUS, option.rect.bottom()-2*RADIUS, 2*RADIUS, 2*RADIUS, 0, -90);
            titlePath.lineTo(option.rect.bottomLeft()+QPoint(RADIUS, 0));
            titlePath.arcTo(option.rect.left(), option.rect.bottom()-2*RADIUS, 2*RADIUS, 2*RADIUS, 270, -90);
            titlePath.closeSubpath();
        }

        painter->setBrush(itemGradient);
        painter->setPen(borderPen);
        painter->drawPath(titlePath);

        QRectF brect = option.rect;
        brect.setLeft(brect.left()+SPACER);
        painter->setFont(option.font);
        painter->drawText(brect, Qt::AlignVCenter, qVariantValue<QString>(index.data()));
    }

    //// HIGHLIGHTING SELECTED ITEM
    //if (option.state & QStyle::State_Selected)
        //painter->fillRect(option.rect, option.palette.highlight());

    painter->restore();
}

QSize BookmarksDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    return QSize(100,40);
}

bool BookmarksDelegate::hasParent( const QModelIndex &index ) const
{
    if( index.parent().isValid() )
        return true;
    else
        return false;
}

bool BookmarksDelegate::isFolder( const QModelIndex &index ) const
{
    BookmarkItem *item=dynamic_cast<BookmarkItem*>(mViewPtr->model()->itemFromIndex(index));
    Q_ASSERT(item);
    return item->bookmark().uri.isEmpty();
}

bool BookmarksDelegate::isLast( const QModelIndex &index ) const
{
    if(!hasParent(index))
        return false; // what should be returned here?

    if(index.row() >= (mViewPtr->model()->rowCount(index.parent())-1))
        return true;
    else
        return false;
}

bool BookmarksDelegate::isExpanded( const QModelIndex &index ) const
{
    if( !mViewPtr )
        return false;
    else
        return mViewPtr->view()->isExpanded( index );
}

bool BookmarksDelegate::isPointFromRect(const QPoint &aPoint, const QRect &aRect) const
{
    if( (aPoint.x()>=aRect.left() && aPoint.x()<=aRect.right()) && (aPoint.y()>=aRect.top() && aPoint.y()<=aRect.bottom()) )
        return true;

    return false;
}

