#include "MDiscardableDialog.h"
#include "MerkaartorPreferences.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QLabel>
#include <QSettings>

MDiscardableDialog::MDiscardableDialog(QWidget *parent, QString title)
    : QDialog(parent), mainWidget(0), Title(title)
{
    QSettings* Sets = M_PREFS->getQSettings();
    Sets->beginGroup("DiscardableDialogs");
    DiscardableRole = Sets->value(title, -1).toInt();
    Sets->endGroup();

    setWindowTitle(title);
    setMinimumSize(300, 100);

    theLayout = new QVBoxLayout(this);
    theLayout->setSpacing(4);
    theLayout->setContentsMargins(4, 4, 4, 4);

    theDSA.setText(tr("Don't ask me this again"));
    theLayout->addWidget(&theDSA);
}

void MDiscardableDialog::setWidget ( QWidget * widget )
{
    if (mainWidget)
        theLayout->removeWidget(mainWidget);

    mainWidget = widget;
    mainWidget->setParent(this);
    theLayout->insertWidget(0, mainWidget);
}

QWidget* MDiscardableDialog::getWidget()
{
    mainWidget = new QWidget();
    mainWidget->setParent(this);

    theLayout->addWidget(mainWidget);

    return mainWidget;
}

int MDiscardableDialog::check()
{
    if (DiscardableRole != -1)
        return DiscardableRole;

    int tmpRet = exec();
    if (theDSA.isChecked()) {
        DiscardableRole = tmpRet;

        QSettings* Sets = M_PREFS->getQSettings();
        Sets->beginGroup("DiscardableDialogs");
        Sets->setValue(Title, DiscardableRole);
        Sets->endGroup();
    }

    return tmpRet;
}

/* MDiscardableMessage */

MDiscardableMessage::MDiscardableMessage(QWidget *parent, QString title, QString msg)
    : MDiscardableDialog(parent, title)
{
    theBB.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
    theLayout->addWidget(&theBB);

    connect(&theBB, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&theBB, SIGNAL(rejected()), this, SLOT(reject()));

    QLabel * txt = new QLabel();
    txt->setText(msg);
    setWidget(txt);
}

