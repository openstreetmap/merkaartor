//
// C++ Implementation: StyleDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "StyleDock.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Document.h"
#include "Feature.h"
#include "PaintStyle/EditPaintStyle.h"

#include <QAction>

StyleDock::StyleDock(MainWindow* aParent)
    : MDockAncestor(aParent), Main(aParent)
{
    setMinimumSize(220,100);
    setObjectName("StyleDock");

    ui.setupUi(getWidget());

    connect(ui.StyleList, SIGNAL(itemSelectionChanged()), this, SLOT(on_StyleList_itemSelectionChanged()));
    connect(ui.StyleList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_StyleList_itemDoubleClicked(QListWidgetItem*)));
    connect(ui.StyleList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_StyleList_customContextMenuRequested(const QPoint &)));

    retranslateUi();
}

StyleDock::~StyleDock()
{
}

void StyleDock::clearItems()
{
	ui.StyleList->blockSignals(true);
	ui.StyleList->clear();
	ui.StyleList->blockSignals(false);
}

void StyleDock::addItem(QAction* a)
{
	ui.StyleList->blockSignals(true);

	QListWidgetItem* it = new QListWidgetItem(a->text());
	it->setData(Qt::UserRole, qVariantFromValue((void *)a));
    ui.StyleList->addItem(it);
	if (a->data().toString() == M_PREFS->getDefaultStyle())
		ui.StyleList->setCurrentItem(it);

	ui.StyleList->blockSignals(false);
}

void StyleDock::setCurrent(QAction* a)
{
	ui.StyleList->blockSignals(true);

	for (int i=0; i < ui.StyleList->count(); i++) {
		if (ui.StyleList->item(i)->data(Qt::UserRole).value<void *>() == a) {
			ui.StyleList->setCurrentRow(i);
			break;
		}
	}

	ui.StyleList->blockSignals(false);
}

void StyleDock::on_StyleList_itemSelectionChanged()
{
	QListWidgetItem* item = ui.StyleList->currentItem();
	QAction * a = (QAction *)item->data(Qt::UserRole).value<void *>();
	a->trigger();
}

void StyleDock::on_StyleList_itemDoubleClicked(QListWidgetItem* item)
{
	Q_UNUSED(item)
}

void StyleDock::on_StyleList_customContextMenuRequested(const QPoint & pos)
{
	Q_UNUSED(pos)
}

void StyleDock::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    MDockAncestor::changeEvent(event);
}

void StyleDock::retranslateUi()
{
    setWindowTitle(tr("Styles"));
}

