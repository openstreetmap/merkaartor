#ifndef BOOKMARKBAR_H
#define BOOKMARKBAR_H

#include <QWidget>

#include "ui_BookmarkBar.h"

class QLineEdit;
class QToolButton;

class BookmarkBar : public QWidget
{
    Q_OBJECT

public:
    BookmarkBar(QWidget *parent = 0);

protected:

signals:
    void bookmarkEntered(const QString &name, const QString &url);
    void bookmarkDeleted(const QString &name);

public slots:
    void slotBookmarkSelected(const QString& name, const QString& url);

private slots:
    void processBookmark();
    void processFolder();
    void delBookmark();

private:
    Ui::BookmarkBar ui;

};

#endif // BOOKMARKBAR_H
