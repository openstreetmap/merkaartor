//
// C++ Interface: PreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QWidget>
#include <QHttp>
#include <QBuffer>


#include "GeneratedFiles/ui_PreferencesDialog.h"

/**
	@author cbro <cbro@semperpax.com>
*/

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
	Q_OBJECT

public:
	PreferencesDialog(QWidget* parent = 0);
	~PreferencesDialog();

public slots:
	void on_cbMapAdapter_currentIndexChanged(int index);
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_BrowseStyle_clicked();
	void on_btAdapterSetup_clicked();
	void on_btColorChooser_clicked();

private:
	void loadPrefs();
	void savePrefs();
public:

private:

};

#endif
