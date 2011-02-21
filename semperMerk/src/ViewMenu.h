#ifndef VIEWMENU_H
#define VIEWMENU_H

#include <QDialog>

namespace Ui {
    class ViewMenu;
}

class ViewMenu : public QDialog
{
    Q_OBJECT

public:
    explicit ViewMenu(QWidget *parent = 0);
    ~ViewMenu();

private slots:
    void on_btMap_clicked();
    void on_btGps_clicked();

signals:
    void mapRequested();
    void gpsRequested();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ViewMenu *ui;
};

#endif // VIEWMENU_H
