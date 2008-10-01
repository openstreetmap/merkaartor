#include <QTableWidget>
#include <QHeaderView>
#include <QAction>
#include <QLayout>
#include <QPushButton>

#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "ActionsDialog.h"

ActionsDialog::ActionsDialog(QList<QAction *>& actions, MainWindow *parent)
    : QDialog(parent), Main(parent)
{
    actionsTable = new QTableWidget(actions.count(), 2, this);
	QStringList hdr;
	hdr << tr("Description") << tr("Shortcut");
	actionsTable->setHorizontalHeaderLabels(hdr);
    actionsTable->verticalHeader()->hide();
	actionsTable->horizontalHeader()->setMinimumSectionSize(100);
	actionsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	actionsTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    //actionsTable->setColumnReadOnly(0, true);

    int row = 0;
    
	for (int i=0; i<actions.size(); ++i) {
	    QAction *action = static_cast<QAction*>(actions.at(i));
		QTableWidgetItem* it = new QTableWidgetItem(action->toolTip());
		it->setFlags(0);
        actionsTable->setItem(row, 0, it);
        actionsTable->setItem(row, 1, new QTableWidgetItem(action->shortcut().toString()));

        actionsList.append(action);
        ++row;
    }

    QPushButton *defaultButton = new QPushButton(tr("&Default"), this);
    QPushButton *okButton = new QPushButton(tr("&OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"), this);

    connect(actionsTable, SIGNAL(currentChanged(int, int)),
            this, SLOT(recordAction(int, int)));
    connect(actionsTable, SIGNAL(valueChanged(int, int)),
            this, SLOT(validateAction(int, int)));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(resetToDefault()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(defaultButton);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(8);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(actionsTable);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Shortcut Editor"));
	cancelButton->setFocus();
}

void ActionsDialog::resetToDefault()
{
    for (int row = 0; row < (int)actionsList.size(); ++row) {
		QAction *action = actionsList[row];
		actionsTable->item(row, 1)->setText(Main->shortcutsDefault[action->objectName()]);
	}
}

void ActionsDialog::accept()
{
	QStringList shortcuts;
    for (int row = 0; row < (int)actionsList.size(); ++row) {
        QAction *action = actionsList[row];
		action->setShortcut(QKeySequence(actionsTable->item(row, 1)->text()));
		shortcuts.append(action->objectName());
		shortcuts.append(action->shortcut().toString());
    }
	M_PREFS->setShortcuts(shortcuts);

    QDialog::accept();
}

void ActionsDialog::recordAction(int row, int column)
{
    oldAccelText = actionsTable->item(row, column)->text();
}

void ActionsDialog::validateAction(int row, int column)
{
    QTableWidgetItem *item = actionsTable->item(row, column);
    QString accelText = QString(QKeySequence(item->text()));

    if (accelText.isEmpty() && !item->text().isEmpty())
        item->setText(oldAccelText);
    else
        item->setText(accelText);
}
