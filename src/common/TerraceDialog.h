//
// C++ Interface: TerraceDialog
//
// Description: 
//
//
// Author: James Hogan <james@albanarts.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TERRACEDIALOG_H
#define TERRACEDIALOG_H

#include <QWidget>

#include <ui_TerraceDialog.h>

class TerraceDialog: public QDialog , public Ui::TerraceDialog
{
    Q_OBJECT

public:
    TerraceDialog(QWidget *parent = 0);

    unsigned int numHouses() const;
    unsigned int maxHouses() const;
    bool hasHouseNumbers() const;
    QStringList houseNumbers() const;

    unsigned int calcNumbering(int type, const QString& ranges, QStringList* outNumbers = 0) const;
    void updateNumbering(int type, const QString& ranges);

public slots:
    void numberingTypeChanged(int type);
    void numberingRangeChanged(const QString& ranges);
    void updateNumbering();

private:
    void changeEvent(QEvent*);
};

#endif
