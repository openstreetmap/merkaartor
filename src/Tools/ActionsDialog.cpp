#include <QTableWidget>
#include <QHeaderView>
#include <QAction>
#include <QLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "ActionsDialog.h"

ActionsDialog::ActionsDialog(QList<QAction *>& actions, MainWindow *parent)
    : QDialog(parent), Main(parent)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    QPushButton *importButton = new QPushButton(tr("&Import"), this);
    QPushButton *exportButton = new QPushButton(tr("&Export"), this);
    QPushButton *defaultButton = new QPushButton(tr("&Default"), this);
    QPushButton *okButton = new QPushButton(tr("&OK"), this);
    QPushButton *cancelButton = new QPushButton(tr("&Cancel"), this);

    connect(actionsTable, SIGNAL(currentChanged(int, int)),
            this, SLOT(recordAction(int, int)));
    connect(actionsTable, SIGNAL(valueChanged(int, int)),
            this, SLOT(validateAction(int, int)));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importShortcuts()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportShortcuts()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(resetToDefault()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(exportButton);
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

void ActionsDialog::importShortcuts()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Shortcut scheme"), QString(), tr("Merkaartor shortcut scheme (*.mss)"));
	if (!fileName.isNull()) {
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(this, tr("Unable to open file"), tr("%1 could not be opened.").arg(fileName));
			return;
		}

		QTextStream ts(&file);
		while (!ts.atEnd()) {
			QString s = ts.readLine();
			if (!s.isNull()) {
				QStringList t = s.split(":");
				for (int row = 0; row < (int)actionsList.size(); ++row) {
					if (actionsTable->item(row, 0)->text() == t[0]) {
						actionsTable->item(row, 1)->setText(t[1]);
						break;
					}
				}
			}
		}
	}
}

void ActionsDialog::exportShortcuts()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Shortcut scheme"), QString("%1/%2.mss").arg(MerkaartorPreferences::instance()->getWorkingDir()).arg(tr("untitled")), tr("Merkaartor shortcut scheme (*.mss)"));

	if (!fileName.isNull()) {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			QMessageBox::critical(this, tr("Unable to open save file"), tr("%1 could not be opened for writing.").arg(fileName));
			return;
		}

		QTextStream ts(&file);
		QStringList shortcuts;
		for (int row = 0; row < (int)actionsList.size(); ++row) {
			ts << actionsTable->item(row, 0)->text() << ":" << actionsTable->item(row, 1)->text() << endl;
		}

		file.close();
	}
}

