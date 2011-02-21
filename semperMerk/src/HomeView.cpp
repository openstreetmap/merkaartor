
#include "HomeView.h"

#include <QtCore>
#include <QtGui>

#include "AddressBar.h"
#include "BookmarkBar.h"
#include "BookmarksView.h"

#include "MouseMachine/MouseMachine.h"

HomeView::HomeView(QWidget *parent)
    : QWidget(parent)
    , m_addressBar(0)
    , m_bookmarkBar(0)
    , charm(0)
{
    m_addressBar = new AddressBar(parent);
    connect(m_addressBar, SIGNAL(addressEntered(QString)), SLOT(gotoAddress(QString)));
    connect(m_addressBar, SIGNAL(back()), SIGNAL(back()));

    m_bookmarkBar = new BookmarkBar(parent);
    m_bookmarks = new BookmarksView(parent);

    connect(m_bookmarks, SIGNAL(urlSelected(QUrl)), SIGNAL(urlActivated(QUrl)));
    connect(m_bookmarks, SIGNAL(bookmarkSelected(QString, QString)), m_bookmarkBar, SLOT(slotBookmarkSelected(QString, QString)));
    connect(m_bookmarks, SIGNAL(bookmarkSelected(QString, QString)), this, SLOT(slotBookmarkSelected(QString, QString)));
    connect(m_bookmarkBar, SIGNAL(bookmarkEntered(QString, QString)), m_bookmarks, SLOT(bookmarkAdd(QString, QString)));
    connect(m_bookmarkBar, SIGNAL(bookmarkDeleted(QString)), m_bookmarks, SLOT(bookmarkDel(QString)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(2);
    layout->setSpacing(4);
    layout->addWidget(m_addressBar);
    layout->addWidget(m_bookmarks);
    layout->addWidget(m_bookmarkBar);
}

void HomeView::gotoAddress(const QString &address)
{
    emit addressEntered(address);
}

void HomeView::setCurrentAdress(const QString & name, const QString& adress)
{
    m_bookmarkBar->slotBookmarkSelected(name, adress);
}

void HomeView::slotBookmarkSelected(const QString& name, const QString& url)
{
    m_addressBar->setUrl(url);
}

void HomeView::toggleEdit()
{
    if (charm) {
        delete charm;
        charm = NULL;

        m_addressBar->show();
        m_bookmarkBar->show();

        m_addressBar->activateWindow();
        m_addressBar->setFocus();
    } else {
        switchToBrowse();
    }
}

void HomeView::switchToBrowse()
{
    m_addressBar->hide();
    m_bookmarkBar->hide();

    if (!charm) {
        charm = new MouseMachine(m_bookmarks->view(), MouseMachine::AutoScroll | MouseMachine::SignalTap);
        charm->setObjectName("Bookmark_charm");
        connect(charm, SIGNAL(singleTap(QPoint)), m_bookmarks, SLOT(slotDoubleClicked(QPoint)));
    }
    m_bookmarks->toXml();
}
