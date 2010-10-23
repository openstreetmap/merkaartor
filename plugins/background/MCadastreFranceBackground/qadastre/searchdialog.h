/*
   This file is part of Qadastre.
   Copyright (C)  2010 Pierre Ducroquet <pinaraf@pinaraf.info>

   Qadastre is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Qadastre is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Qadastre. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QMap>

class CadastreWrapper;

namespace Ui {
    class SearchDialog;
}

class SearchDialog : public QDialog {
    Q_OBJECT
public:
    SearchDialog(QWidget *parent = 0);
    ~SearchDialog();

    QString cityCode();
    QString cityName();

public:
    CadastreWrapper *cadastre;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SearchDialog *ui;
    QMap<QString,QString> m_results;

private slots:
    void on_results_activated(int index);
    void on_searchButton_clicked();
    void resultsAvailable(QMap<QString,QString> results);
};

#endif // SEARCHDIALOG_H
