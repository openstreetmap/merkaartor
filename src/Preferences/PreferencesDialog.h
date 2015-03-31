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

#include "ui_PreferencesDialog.h"
#include "ui_OsmServerWidget.h"
#include "MerkaartorPreferences.h"

/**
    @author cbro <cbro@semperpax.com>
*/

class OsmServerWidget : public QWidget, public Ui::OsmServerWidget
{
    Q_OBJECT

public:
    OsmServerWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);

public slots:
    void on_tbOsmServerAdd_clicked();
    void on_tbOsmServerDel_clicked();
    void on_rbOsmServerSelected_clicked();
};

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
    void on_btHighlightColor_clicked();
    void on_btDirtyColor_clicked();
    void on_btRelationsColor_clicked();
    void on_btGpxTrackColor_clicked();

    /* GPS */
    void on_btGpsLogDirBrowse_clicked();

    /* Tools */
    void on_btAddTool_clicked(void);
    void on_btDelTool_clicked(void);
    void on_btApplyTool_clicked(void);
    void on_lvTools_itemSelectionChanged();
    void on_btBrowse_clicked();

    /* Data */
    void on_btAutoloadBrowse_clicked();

private:
    void updateStyles();
    void loadPrefs();
    void initLanguages(QComboBox*);
    void savePrefs();

    void changeEvent(QEvent *);
    QColor pickColor(QColor defaultColor);

private:
    QList<Tool> theTools;
    QColor BgColor;
    QColor FocusColor;
    QColor HoverColor;
    QColor HighlightColor;
    QColor DirtyColor;
    QColor RelationsColor;
    QColor GpxTrackColor;

signals:
    void preferencesChanged(PreferencesDialog* prefs);
};

#endif
