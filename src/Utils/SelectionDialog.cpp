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
#include "Maps/MapDocument.h"

#include <QCompleter>

SelectionDialog::SelectionDialog(QWidget *parent)
 : QDialog(parent)
{
	setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

	cbKey->setInsertPolicy(QComboBox::InsertAlphabetically);
	cbValue->setInsertPolicy(QComboBox::InsertAlphabetically);

	MainWindow* mw = (MainWindow *)(this->parent());

	QCompleter* completer = new QCompleter(mw->document()->getTagList(), (QObject *)this);
	cbKey->insertItems(-1, mw->document()->getTagList());
	completer->setCompletionMode(QCompleter::InlineCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	cbKey->setCompleter(completer);
	cbKey->setEditable(true);

	cbValue->insertItems(-1, mw->document()->getTagValueList("*"));
	cbValue->setEditable(true);

	edName->setText(MerkaartorPreferences::instance()->getLastSearchName());
	cbKey->setEditText(MerkaartorPreferences::instance()->getLastSearchKey());
	cbValue->setEditText(MerkaartorPreferences::instance()->getLastSearchValue());
	sbMaxResult->setValue(MerkaartorPreferences::instance()->getLastMaxSearchResults());
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
	completer->setCompletionMode(QCompleter::InlineCompletion);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	if (cbValue->completer())
		delete cbValue->completer();
	cbValue->setCompleter(completer);
}


