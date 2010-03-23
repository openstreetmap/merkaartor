//
// C++ Implementation: ProjPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/ProjPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

ProjPreferencesDialog::ProjPreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadPrefs();
}

ProjPreferencesDialog::~ProjPreferencesDialog()
{
}

void ProjPreferencesDialog::addProjection(const ProjectionItem & srv)
{
    theItems.push_back(srv);
    if (!srv.deleted) {
        QListWidgetItem* item = new QListWidgetItem(srv.name);
        item->setData(Qt::UserRole, (int) theItems.size()-1);
        lvProjections->addItem(item);
    }
}

void ProjPreferencesDialog::on_btApply_clicked(void)
{
    int idx = static_cast<int>(lvProjections->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theItems.size())
        return;

    ProjectionItem& item(theItems[idx]);
    item.name = edProjName->text();
    item.projection = edProj4String->text();

    lvProjections->currentItem()->setText(item.name);
    selectedItem = item.name;
}

void ProjPreferencesDialog::on_btAdd_clicked(void)
{
    addProjection(ProjectionItem(edProjName->text(), edProj4String->text()));
    lvProjections->setCurrentRow(lvProjections->count() - 1);
    on_lvProjections_itemSelectionChanged();
}

void ProjPreferencesDialog::on_btDel_clicked(void)
{
    int idx = static_cast<int>(lvProjections->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theItems.size())
        return;

    theItems[idx].deleted = true;
    delete lvProjections->takeItem(idx);
    on_lvProjections_itemSelectionChanged();
}

void ProjPreferencesDialog::on_lvProjections_itemSelectionChanged()
{
    QListWidgetItem* it = lvProjections->item(lvProjections->currentRow());

    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= theItems.size())
        return;

    ProjectionItem& item(theItems[idx]);
    edProjName->setText(item.name);
    edProj4String->setText(item.projection);

    selectedItem = item.name;
}

QString ProjPreferencesDialog::getSelectedItem()
{
    return selectedItem;
}

void ProjPreferencesDialog::setSelectedItem(QString theValue)
{
    QList<QListWidgetItem *> L = lvProjections->findItems(theValue, Qt::MatchExactly);
    lvProjections->setCurrentItem(L[0]);
    on_lvProjections_itemSelectionChanged();
}

void ProjPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
        savePrefs();
    } else
        if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
            savePrefs();
            this->accept();
        }
}

void ProjPreferencesDialog::loadPrefs()
{
    ProjectionList* L = M_PREFS->getProjectionsList()->getProjections();
    ProjectionListIterator i(*L);
    while (i.hasNext()) {
        i.next();
        addProjection(i.value());
    }
}

void ProjPreferencesDialog::savePrefs()
{
    ProjectionList* L = M_PREFS->getProjectionsList()->getProjections();
    L->clear();
    for (int i = 0; i < theItems.size(); ++i) {
        ProjectionItem S(theItems[i]);
        L->insert(theItems[i].name, S);
    }
    M_PREFS->save();
}

