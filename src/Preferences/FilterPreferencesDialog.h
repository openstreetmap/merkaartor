//
// C++ Interface: FilterPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FilterPreferencesDialog_H
#define FilterPreferencesDialog_H

#include <QWidget>
#include <QBuffer>


#include <ui_FilterPreferencesDialog.h>
#include "Preferences/MerkaartorPreferences.h"
#include "FilterList.h"

#include <QList>

/**
    @author cbro <cbro@semperpax.com>
*/

class FilterPreferencesDialog : public QDialog, public Ui::FilterPreferencesDialog
{
    Q_OBJECT

public:
    FilterPreferencesDialog(QWidget* parent = 0);
    ~FilterPreferencesDialog();

    void addFilter(const FilterItem & item);

public slots:
    void on_btApply_clicked();
    void on_btAdd_clicked();
    void on_btDel_clicked();
    void on_btFilterHelper_clicked();
    void on_lvFilters_itemSelectionChanged();
    void on_buttonBox_clicked(QAbstractButton * button);

private:
    void loadPrefs();
    void savePrefs();
public:
    QList<FilterItem> theItems;
    QString getSelectedItem();
    void setSelectedItem(QString theValue);

private:
    QString selectedItem;
    int httpGetId;
    QBuffer* buf;

};

#endif
