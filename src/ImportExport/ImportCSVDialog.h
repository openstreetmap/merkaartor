//***************************************************************
// CLass: %CLASS%
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#ifndef IMPORTCSVDIALOG_H
#define IMPORTCSVDIALOG_H

#include <QDialog>
#include <QIODevice>

#include "Maps/Projection.h"

class Feature;
class Layer;

namespace Ui {
    class ImportCSVDialog;
}

enum CSVFieldType {
    CSVString,
    CSVInt,
    CSVFloat,
    CSVLongitude,
    CSVLatitude
};

struct CSVField {
    QString name;
    CSVFieldType type;
    bool import;
};

typedef QList<CSVField> CSVFields;

class ImportCSVDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportCSVDialog(QIODevice* aDev, QWidget *parent = 0);
    ~ImportCSVDialog();

    bool import(Layer* aLayer);

protected slots:
    void initialize();

    void on_rbColonDelim_clicked();
    void on_rbSemiDelim_clicked();
    void on_rbTabDelim_clicked();
    void on_edCustomDelim_textEdited();

    void on_lvFields_itemSelectionChanged();
    void on_edFieldName_textEdited();
    void on_cbFieldType_currentIndexChanged (int index);
    void on_cbFieldImport_clicked(bool b);
    void on_edProjection_editingFinished();

    void on_btLoad_clicked();
    void on_btSave_clicked();

    void on_buttonBox_accepted();

protected:
    void changeEvent(QEvent *e);

    void analyze();
    void generatePreview(int sel=-1);
    Feature* generateOSM(QString line);

private:
    Ui::ImportCSVDialog *ui;

    QIODevice* m_dev;
    QString delim;
    int m_selField;

public:
    CSVFields Fields;
    Projection CSVProjection;
};

#endif // IMPORTCSVDIALOG_H
