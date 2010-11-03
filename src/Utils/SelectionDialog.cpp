//
// C++ Implementation: SelectionDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "SelectionDialog.h"
#include "MainWindow.h"
#include "Document.h"

#include <QCompleter>

SelectionDialog::SelectionDialog(QWidget *parent, bool showMaxResult)
 : QDialog(parent)
{
    setupUi(this);
    if (!showMaxResult)
        widgetMaxResult->setVisible(false);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    cbKey->setInsertPolicy(QComboBox::InsertAlphabetically);
    cbValue->setInsertPolicy(QComboBox::InsertAlphabetically);

    MainWindow* mw = (MainWindow *)(this->parent());

    QStringList ksl = mw->document()->getTagKeyList();
    QCompleter* completer = new QCompleter(ksl, (QObject *)this);

    cbKey->insertItems(-1, ksl);
    //special keys
    cbKey->insertItem(-1, ":zoomlevel");
    cbKey->insertItem(-1, ":version");
    cbKey->insertItem(-1, ":user");
    cbKey->insertItem(-1, ":uploaded");
    cbKey->insertItem(-1, ":time");
    cbKey->insertItem(-1, ":pixelperm");
    cbKey->insertItem(-1, ":dirty");
    cbKey->insertItem(-1, ":id");

    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    cbKey->setCompleter(completer);
    cbKey->setEditable(true);


    cbValue->insertItems(-1, mw->document()->getTagValueList("*"));
    //special values
    cbValue->insertItem(-1, "_NULL_");

    cbValue->setEditable(true);

    edName->setText(M_PREFS->getLastSearchName());
    cbKey->setEditText(M_PREFS->getLastSearchKey());
    cbValue->setEditText(M_PREFS->getLastSearchValue());
    sbMaxResult->setValue(M_PREFS->getLastMaxSearchResults());
    edTagQuery->setText(M_PREFS->getLastSearchTagSelector());
}

SelectionDialog::~SelectionDialog()
{
}

void SelectionDialog::on_cbKey_editTextChanged(const QString & text)
{
    cbValue->clear();

    MainWindow* mw = (MainWindow *)(this->parent());

    QStringList sl = mw->document()->getTagValueList(text);
    QCompleter* completer = new QCompleter(sl, (QObject *)this);
    cbValue->insertItems(-1, mw->document()->getTagValueList(text));
    cbValue->insertItem(-1, "_NULL_");
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    if (cbValue->completer())
        delete cbValue->completer();
    cbValue->setCompleter(completer);

    edTagQuery->setText("[" + text + "] is " + cbValue->currentText());

}

void SelectionDialog::on_cbValue_editTextChanged(const QString & text)
{
    if (!cbKey->currentText().isEmpty())
        edTagQuery->setText("[" + cbKey->currentText() + "] is " + text);
    else
        edTagQuery->setText("[*] is " + text);
}

void SelectionDialog::on_edName_textChanged(const QString &text)
{
    edTagQuery->setText("[name] is *" + text + "*");
}

void SelectionDialog::on_edID_textChanged(const QString &text)
{
    edTagQuery->setText("[:id] is " + text);
}

void SelectionDialog::on_buttonBox_accepted()
{
    M_PREFS->setLastSearchName(edName->text());
    M_PREFS->setLastSearchKey(cbKey->currentText());
    M_PREFS->setLastSearchValue(cbValue->currentText());
    M_PREFS->setLastMaxSearchResults(sbMaxResult->value());
    M_PREFS->setLastSearchTagSelector(edTagQuery->text());

    emit accept();
}
