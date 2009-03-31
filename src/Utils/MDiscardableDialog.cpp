#include "Utils/MDiscardableDialog.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QLabel>
#include <QSettings>

MDiscardableDialog::MDiscardableDialog(QWidget *parent, QString title)
	: QDialog(parent), Title(title), mainWidget(0)
{
	QSettings Sets;
	Sets.beginGroup("DiscardableDialogs");
	DiscardableRole = Sets.value(title, -1).toInt();

	setWindowTitle(title);
	setMinimumSize(300, 100);

	theLayout = new QVBoxLayout(this);
	theLayout->setSpacing(4);
	theLayout->setMargin(4);
	
	theBB.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
	theLayout->addWidget(&theBB);

	theDSA.setText(tr("Don't ask me this again"));
	theLayout->addWidget(&theDSA);

	connect(&theBB, SIGNAL(accepted()), this, SLOT(accept()));
	connect(&theBB, SIGNAL(rejected()), this, SLOT(reject()));

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

		QSettings Sets;
		Sets.beginGroup("DiscardableDialogs");
		Sets.setValue(Title, DiscardableRole);
	}

	return tmpRet;
}

/* MDiscardableMessage */

MDiscardableMessage::MDiscardableMessage(QWidget *parent, QString title, QString msg)
	: MDiscardableDialog(parent, title)
{
	QLabel * txt = new QLabel();
	txt->setText(msg);
	setWidget(txt);
}

