#ifndef TAGSELECTORWIDGET_H
#define TAGSELECTORWIDGET_H

#include <QWidget>

class MainWindow;

namespace Ui {
    class TagSelectorWidget;
}

class TagSelectorWidget : public QWidget {
    Q_OBJECT
public:
    TagSelectorWidget(MainWindow* mw, QWidget *parent = 0);
    ~TagSelectorWidget();

protected:
    void changeEvent(QEvent *e);

signals:
    void sigOr();
    void sigAnd();
    void sigNot();

private slots:
    void on_cbKey_editTextChanged(const QString & text);

private:
    Ui::TagSelectorWidget *ui;

    MainWindow* main;
};

#endif // TAGSELECTORWIDGET_H
