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
#include <QListWidgetItem>


#include <ui_PreferencesDialog.h>
#include "Preferences/MerkaartorPreferences.h"

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
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_BrowseStyle_clicked();
	void on_BrowseTemplate_clicked();
	void on_btBgColor_clicked();
	void on_btFocusColor_clicked();
	void on_btHoverColor_clicked();
	void on_btRelationsColor_clicked();
	
	/* GPS */
	void on_btGpsLogDirBrowse_clicked();

	/* Tools */
	void on_btAddTool_clicked(void);
	void on_btDelTool_clicked(void);
	void on_btApplyTool_clicked(void);
	void on_lvTools_itemClicked(QListWidgetItem* it);
	void on_btBrowse_clicked();

private:
	void updateStyles();
	void loadPrefs();
	void initLanguages(QComboBox*);
	void savePrefs();

	void changeEvent(QEvent *);

private:
	QList<Tool> theTools;
	QColor BgColor;
	QColor FocusColor;
	QColor HoverColor;
	QColor RelationsColor;

signals:
	void preferencesChanged();
};

#endif
