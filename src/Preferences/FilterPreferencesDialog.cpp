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
#include "Global.h"

#include "FilterPreferencesDialog.h"
#include "TagSelector.h"

#include "SelectionDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

FilterPreferencesDialog::FilterPreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    btAdd->setEnabled(false);
    btApply->setEnabled(false);
    btDel->setEnabled(false);

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

    TagSelector* t = TagSelector::parse(srv.filter);
    qDebug() << t->asExpression(false);
}

void FilterPreferencesDialog::on_btApply_clicked(void)
{
    int idx = static_cast<int>(lvFilters->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theItems.size())
        return;
    TagSelector* t = TagSelector::parse(edFilterString->text());
    if (!t)
        return;

    FilterItem& item(theItems[idx]);
    item.name = edFilterName->text();
    item.filter = t->asExpression(false);

    lvFilters->currentItem()->setText(item.name);
    selectedItem = item.name;
}

void FilterPreferencesDialog::on_btAdd_clicked(void)
{
    TagSelector* t = TagSelector::parse(edFilterString->text());
    if (!t)
        return;

    addFilter(FilterItem(QUuid::createUuid(), edFilterName->text(), t->asExpression(false)));
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

void FilterPreferencesDialog::on_btFilterHelper_clicked()
{
    SelectionDialog* Sel = new SelectionDialog(CUR_MAINWINDOW, false);
    if (!Sel)
        return;

    Sel->edTagQuery->setText(edFilterString->text());
    if (Sel->exec() == QDialog::Accepted) {
        edFilterString->setText(Sel->edTagQuery->text());
    }
}

void FilterPreferencesDialog::on_lvFilters_itemSelectionChanged()
{
    if (lvFilters->currentRow() < 0)
    {
        selectedItem.clear();
        edFilterName->setText(selectedItem);
        edFilterString->setText(selectedItem);
        btDel->setEnabled(false);
        btApply->setEnabled(false);
        return;
    }

    QListWidgetItem* it = lvFilters->item(lvFilters->currentRow());
    on_lvFilters_itemClicked(it);
}

void FilterPreferencesDialog::on_lvFilters_itemClicked(QListWidgetItem * it)
{
    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= theItems.size()) {
        btDel->setEnabled(false);
        btApply->setEnabled(false);
        return;
    }

    FilterItem& item(theItems[idx]);
    edFilterName->setText(item.name);
    edFilterString->setText(item.filter);

    selectedItem = item.name;
    btDel->setEnabled(true);
    btApply->setEnabled(true);
}

void FilterPreferencesDialog::on_edFilterName_textChanged(const QString & text)
{
    btApply->setEnabled(false);
    btDel->setEnabled(false);
    if (text.isEmpty()) {
        btAdd->setEnabled(false);
    } else {
        btAdd->setEnabled(true);
    }
}

void FilterPreferencesDialog::on_edFilterString_textChanged(const QString & /*text*/)
{
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
    if (button == buttonBox->button(QDialogButtonBox::Apply)) {
        savePrefs();
    } else
        if (button == buttonBox->button(QDialogButtonBox::Ok)) {
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

