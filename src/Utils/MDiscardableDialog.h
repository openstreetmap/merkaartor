#ifndef MDISCARDABLEDIALOG_H
#define MDISCARDABLEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>

class QVBoxLayout;

class MDiscardableDialog : public QDialog
{
Q_OBJECT

public:
    MDiscardableDialog(QWidget *parent = 0, QString title = QString());
    void setWidget ( QWidget * widget );
    QWidget* getWidget();
    virtual int check();

protected:
    QVBoxLayout* theLayout;
    QWidget* mainWidget;
    QCheckBox theDSA;
    QString Title;
    int DiscardableRole;
};

class MDiscardableMessage : public MDiscardableDialog
{
public:
    MDiscardableMessage(QWidget *parent = 0, QString title = QString(), QString msg = QString());

protected:
    QDialogButtonBox theBB;
};

#endif //MDISCARDABLEDIALOG_H
