//
// C++ Implementation: FilterPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/FilterPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

FilterPreferencesDialog::FilterPreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadPrefs();
}

FilterPreferencesDialog::~FilterPreferencesDialog()
{
}

void FilterPreferencesDialog::addFilter(const FilterItem & srv)
{
    theItems.push_back(srv);
    if (!srv.deleted) {
        QListWidgetItem* item = new QListWidgetItem(srv.name);
        item->setData(Qt::UserRole, (int) theItems.size()-1);
        lvFilters->addItem(item);
    }
}

void FilterPreferencesDialog::on_btApply_clicked(void)
{
    int idx = static_cast<int>(lvFilters->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theItems.size())
        return;

    FilterItem& item(theItems[idx]);
    item.name = edFilterName->text();
    item.filter = edFilterString->text();

    lvFilters->currentItem()->setText(item.name);
    selectedItem = item.name;
}

void FilterPreferencesDialog::on_btAdd_clicked(void)
{
    addFilter(FilterItem(edFilterName->text(), edFilterString->text()));
    lvFilters->setCurrentRow(lvFilters->count() - 1);
    on_lvFilters_itemSelectionChanged();
}

void FilterPreferencesDialog::on_btDel_clicked(void)
{
    int idx = static_cast<int>(lvFilters->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theItems.size())
        return;

    theItems[idx].deleted = true;

    delete lvFilters->takeItem(lvFilters->currentRow());
    on_lvFilters_itemSelectionChanged();
}

void FilterPreferencesDialog::on_lvFilters_itemSelectionChanged()
{
    if (lvFilters->currentRow() < 0)
    {
        selectedItem.clear();
        edFilterName->setText(selectedItem);
        edFilterString->setText(selectedItem);
        return;
    }

    QListWidgetItem* it = lvFilters->item(lvFilters->currentRow());

    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= theItems.size())
         return;

    FilterItem& item(theItems[idx]);
    edFilterName->setText(item.name);
    edFilterString->setText(item.filter);

    selectedItem = item.name;
}

QString FilterPreferencesDialog::getSelectedItem()
{
    return selectedItem;
}

void FilterPreferencesDialog::setSelectedItem(QString theValue)
{
    QList<QListWidgetItem *> L = lvFilters->findItems(theValue, Qt::MatchExactly);
    lvFilters->setCurrentItem(L[0]);
    on_lvFilters_itemSelectionChanged();
}

void FilterPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
        savePrefs();
    } else
        if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
            savePrefs();
            this->accept();
        }
}

void FilterPreferencesDialog::loadPrefs()
{
    FilterList* L = M_PREFS->getFiltersList()->getFilters();
    FilterListIterator i(*L);
    while (i.hasNext()) {
        i.next();
        addFilter(i.value());
    }
}

void FilterPreferencesDialog::savePrefs()
{
    FilterList* L = M_PREFS->getFiltersList()->getFilters();
    L->clear();
    for (int i = 0; i < theItems.size(); ++i) {
        FilterItem S(theItems[i]);
        L->insert(theItems[i].name, S);
    }
    M_PREFS->save();
}

