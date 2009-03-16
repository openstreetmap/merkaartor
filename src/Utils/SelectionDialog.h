//
// C++ Interface: SelectionDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QDialog>
#include <ui_SelectionDialog.h>

/**
	@author cbro <cbro@semperpax.com>
*/
class SelectionDialog : public QDialog, public Ui::SelectionDialog
{
	Q_OBJECT

	public:
		SelectionDialog(QWidget *parent = 0);
    	~SelectionDialog();

	private slots:
		void on_cbKey_editTextChanged(const QString & text);

};

#endif
