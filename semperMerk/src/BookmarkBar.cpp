#include "BookmarkBar.h"

#include <QtCore>
#include <QtGui>

BookmarkBar::BookmarkBar(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.m_updButton, SIGNAL(clicked()), SLOT(processBookmark()));
    connect(ui.m_delButton, SIGNAL(clicked()), SLOT(delBookmark()));
    connect(ui.m_fldrButton, SIGNAL(clicked()), SLOT(processFolder()));
}

void BookmarkBar::processBookmark()
{
    if (!ui.m_nameEdit->text().isEmpty() && !ui.m_urlEdit->text().isEmpty())
        emit bookmarkEntered(ui.m_nameEdit->text(), ui.m_urlEdit->text());
}

void BookmarkBar::processFolder()
{
    if (!ui.m_nameEdit->text().isEmpty())
        emit bookmarkEntered(ui.m_nameEdit->text(), QString());
}

void BookmarkBar::delBookmark()
{
    if (!ui.m_nameEdit->text().isEmpty())
        emit bookmarkDeleted(ui.m_nameEdit->text());
}

void BookmarkBar::slotBookmarkSelected(const QString& name, const QString& url)
{
    ui.m_nameEdit->setText(name);
    ui.m_urlEdit->setText(url);
}
