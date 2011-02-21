
#include "BookmarksView.h"
#include "BookmarksDelegate.h"
#include "BookmarkItem.h"

BookmarksView::BookmarksView(QWidget *parent)
    : QWidget(parent)
{
    QString sData;
    QList<QStandardItem *> items;

    fromXml();

#if 1
    int rowCount;
    if (!m_model.rowCount()) {
        qDebug() << "Bookmarks: " << QDir::homePath() + "/Bookmarks.bin";
        QFile f(QDir::homePath() + "/Bookmarks.bin");
        f.open(QIODevice::ReadOnly);
        if (f.isOpen()) {
            QDataStream d(&f);
            d >> rowCount;
            for (int i=0; i<rowCount; i++) {
                items.clear();
                d >> sData;
                BookmarkItem* it = new BookmarkItem(sData);
                it->setEditable(false);
                it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
                d >> sData;
                it->bookmark().uri = sData;
                m_model.appendRow(it);
            }
            f.close();;
        }
    }
#endif

    if (!m_model.rowCount()) {
        BookmarkItem* bk1 = new BookmarkItem("SemperPax");
        bk1->bookmark().uri = "http://redmine.semperpax.com";
        bk1->setEditable(false);
        bk1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        m_model.appendRow(bk1);

        bk1 = new BookmarkItem("Twitter");
        bk1->bookmark().uri = "http://mobile.twitter.com";
        bk1->setEditable(false);
        bk1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        m_model.appendRow(bk1);

        bk1 = new BookmarkItem("Wikipedia (Mobile)");
        bk1->bookmark().uri = "http://mobile.wikipedia.org";
        bk1->setEditable(false);
        bk1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        m_model.appendRow(bk1);

        BookmarkItem* fld1 = new BookmarkItem("Google");
        fld1->setEditable(false);
        fld1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
        m_model.appendRow(fld1);

        bk1 = new BookmarkItem("Google Search");
        bk1->bookmark().uri = "http://www.google.com/m";
        bk1->setEditable(false);
        bk1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        fld1->setChild(fld1->rowCount(), bk1);

        bk1 = new BookmarkItem("GMail");
        bk1->bookmark().uri = "http://m.gmail.com";
        bk1->setEditable(false);
        bk1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        fld1->setChild(fld1->rowCount(), bk1);
    }

    m_iconView = new QTreeView(this);
    m_iconView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_iconView->setDragEnabled(true);
    m_iconView->setAcceptDrops(true);
    m_iconView->setDropIndicatorShown(true);
    m_iconView->setDefaultDropAction(Qt::MoveAction);
    m_iconView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_iconView->setDragDropOverwriteMode(false);
    m_iconView->setExpandsOnDoubleClick(false);
    m_iconView->setAutoScroll(true);

    m_iconView->setModel(&m_model);
    connect(m_iconView, SIGNAL(clicked(const QModelIndex &)), SLOT(select(const QModelIndex &)));
    connect(m_iconView, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(activate(const QModelIndex &)));

    m_iconView->setHeaderHidden(true);
    m_iconView->setRootIsDecorated(false);
    m_iconView->setIndentation(10);
    m_iconView->setAnimated(true);
    m_iconView->setUniformRowHeights(true);
    m_iconView->setItemDelegate(new BookmarksDelegate(this));

//    charm = new MouseMachine(m_iconView, MouseMachine::AutoScroll);
//	connect(charm, SIGNAL(singleTap(QPoint)), SLOT(slotClicked(QPoint)));
//    connect(charm, SIGNAL(doubleTap(QPoint)), SLOT(slotDoubleClicked(QPoint)));
//	connect(charm, SIGNAL(tapAndHold(QPoint)), SLOT(slotTapAndHold(QPoint)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->addWidget(m_iconView);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

BookmarksView::~BookmarksView()
{
    toXml();
}

void BookmarksView::select(const QModelIndex &index)
{
    BookmarkItem *item=static_cast<BookmarkItem*>(m_model.itemFromIndex(index));

    QString name = item->bookmark().title;
    QString url = item->bookmark().uri;

    if (url.isEmpty())
        m_iconView->setExpanded(index, !m_iconView->isExpanded(index));
    emit bookmarkSelected(name, url);
}

void BookmarksView::activate(const QModelIndex &index)
{
    BookmarkItem *item=static_cast<BookmarkItem*>(m_model.itemFromIndex(index));
    if (!item)
        return;

    if (item->bookmark().uri.isEmpty())
        m_iconView->setExpanded(index, !m_iconView->isExpanded(index));
    else
        emit urlSelected(QUrl(item->bookmark().uri));
}

void BookmarksView::bookmarkAdd(const QString& name, const QString &url, int seq)
{
    QList<QStandardItem *> items = m_model.findItems(name, Qt::MatchRecursive | Qt::MatchExactly);
    if (items.size() == 0) {
        BookmarkItem *ibk;

        ibk = new BookmarkItem(name);
        ibk->bookmark().uri = url;
        ibk->setEditable(false);
        ibk->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
        if (url.isEmpty())
            ibk->setFlags(ibk->flags() | Qt::ItemIsDropEnabled);
        items.append(ibk);
        m_model.appendRow(items);
    } else {
        BookmarkItem *item=static_cast<BookmarkItem*>(items[0]);
        bool isfolder = url.isEmpty();
        if (item->isFolder() != isfolder) {
            bookmarkAdd(QString("%1#%2").arg(name).arg(seq, 1), url, ++seq);
        } else {
            item->bookmark().uri = url;
        }
    }
}

void BookmarksView::bookmarkDel(const QString& name)
{
    QList<QStandardItem *> items = m_model.findItems(name, Qt::MatchRecursive | Qt::MatchExactly);
    if (items.size() != 0) {
        QModelIndex ix = m_model.indexFromItem(items[0]);
        m_model.removeRow(ix.row(), ix.parent());
    }
}

void BookmarksView::slotDoubleClicked(QPoint p)
{
    QModelIndex index = m_iconView->indexAt(p);
    activate(index);
}

void BookmarksView::fromXmlFolder(QDomElement& e, BookmarkItem* fit)
{
    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "Bookmark") {
            BookmarkItem* bk = new BookmarkItem(c.attribute("title"));
            bk->setEditable(false);
            bk->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
            bk->bookmark().uri = c.attribute("uri");

            fit->setChild(fit->rowCount(), bk);
        } else if (c.tagName() == "Folder") {
            BookmarkItem* bk = new BookmarkItem(c.attribute("title"));
            bk->setEditable(false);
            bk->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
            fit->setChild(fit->rowCount(), bk);

            fromXmlFolder(c, bk);
        }

        c = c.nextSiblingElement();
    }
}

void BookmarksView::fromXml()
{
    qDebug() << "Bookmarks: " << QDir::homePath() + "/semperWebBookmarks.xml";
    QFile f(QDir::homePath() + "/semperWebBookmarks.xml");
    f.open(QIODevice::ReadOnly);
    if (f.isOpen()) {
        QDomDocument doc;
        doc.setContent(&f);
        f.close();

        QDomElement bks = doc.documentElement();
        while (!bks.isNull()) {
            if (bks.nodeName() == "BookmarkList") {
                QDomElement c = bks.firstChildElement();
                while(!c.isNull()) {
                    if (c.tagName() == "Bookmark") {
                        BookmarkItem* bk = new BookmarkItem(c.attribute("title"));
                        bk->setEditable(false);
                        bk->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
                        bk->bookmark().uri = c.attribute("uri");

                        m_model.appendRow(bk);
                    } else if (c.tagName() == "Folder") {
                        BookmarkItem* bk = new BookmarkItem(c.attribute("title"));
                        bk->setEditable(false);
                        bk->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
                        m_model.appendRow(bk);

                        fromXmlFolder(c, bk);
                    }

                    c = c.nextSiblingElement();
                }
            }
            bks = bks.nextSiblingElement();
        }
    }
}

void BookmarksView::toXmlFolder(QDomElement& e, QModelIndex& pindex)
{
    BookmarkItem* pit = static_cast<BookmarkItem*>(m_model.itemFromIndex(pindex));

    for (int i=0; i<pit->rowCount(); ++i) {
        QModelIndex ix = m_model.index(i, 0, pindex);
        BookmarkItem* it = static_cast<BookmarkItem*>(m_model.itemFromIndex(ix));

        if (it->bookmark().uri.isEmpty()) {
            QDomElement fld = e.ownerDocument().createElement("Folder");
            e.appendChild(fld);

            fld.setAttribute("title", it->bookmark().title);
            toXmlFolder(fld, ix);
        } else {
            QDomElement bk = e.ownerDocument().createElement("Bookmark");
            e.appendChild(bk);

            bk.setAttribute("title", it->bookmark().title);
            bk.setAttribute("uri", it->bookmark().uri);
        }
    }
}

void BookmarksView::toXml()
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement rt = doc.createElement("BookmarkList");
    doc.appendChild(rt);

    for (int i=0; i<m_model.rowCount(); ++i) {
        QModelIndex ix = m_model.index(i, 0);
        BookmarkItem* it = static_cast<BookmarkItem*>(m_model.itemFromIndex(ix));

        if (it->bookmark().uri.isEmpty()) {
            QDomElement fld = doc.createElement("Folder");
            rt.appendChild(fld);

            fld.setAttribute("title", it->bookmark().title);
            toXmlFolder(fld, ix);
        } else {
            QDomElement bk = doc.createElement("Bookmark");
            rt.appendChild(bk);

            bk.setAttribute("title", it->bookmark().title);
            bk.setAttribute("uri", it->bookmark().uri);
        }
    }
    QByteArray ba = doc.toByteArray();

    QFile f(QDir::homePath() + "/semperWebBookmarks.xml");
    f.open(QIODevice::WriteOnly);
    if (f.isOpen()) {
        f.write(ba);
        f.close();
    }
}

