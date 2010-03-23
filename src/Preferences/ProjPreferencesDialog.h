//
// C++ Interface: ProjPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ProjPreferencesDialog_H
#define ProjPreferencesDialog_H

#include <QWidget>
#include <QHttp>
#include <QBuffer>


#include <ui_ProjPreferencesDialog.h>
#include "Preferences/MerkaartorPreferences.h"
#include "ProjectionsList.h"

#include <QList>

/**
    @author cbro <cbro@semperpax.com>
*/

class ProjPreferencesDialog : public QDialog, public Ui::ProjPreferencesDialog
{
    Q_OBJECT

public:
    ProjPreferencesDialog(QWidget* parent = 0);
    ~ProjPreferencesDialog();

    void addProjection(const ProjectionItem & item);

public slots:
    void on_btApply_clicked();
    void on_btAdd_clicked();
    void on_btDel_clicked();
    void on_lvProjections_itemSelectionChanged();
    void on_buttonBox_clicked(QAbstractButton * button);

private:
    void loadPrefs();
    void savePrefs();
public:
    QList<ProjectionItem> theItems;
    QString getSelectedItem();
    void setSelectedItem(QString theValue);

private:
    QString selectedItem;
    int httpGetId;
    QBuffer* buf;

};

#endif
