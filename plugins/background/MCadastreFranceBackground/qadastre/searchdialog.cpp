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

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "cadastrewrapper.h"

#include <QMessageBox>
#include <QDebug>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
    for (int i = 1 ; i < 96 ; i++) {
        ui->department->addItem(QString::number(i));
    }
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    cadastre = CadastreWrapper::instance();
    connect(cadastre, SIGNAL(resultsAvailable(QMap<QString,QString>)), this, SLOT(resultsAvailable(QMap<QString,QString>)));
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SearchDialog::on_searchButton_clicked()
{
    if (ui->department->currentIndex() == -1)
        return;
    if (ui->name->text().isEmpty())
        return;
    QString department = QString("%1").arg(ui->department->currentIndex() + 1, 3, 10, QChar('0'));
    cadastre->searchVille(ui->name->text(), department);
    ui->results->clear();
    m_results.clear();
    ui->results->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void SearchDialog::on_results_activated(int index)
{
    Q_UNUSED(index)
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SearchDialog::resultsAvailable(QMap<QString, QString> results)
{
    qDebug() << "SearchDialog::resultsAvailable: " << results;
    m_results = results;
    if (results.count() == 0) {
        QMessageBox::warning(this, tr("No result"), tr("Your search gave no result."));
    } else {
        ui->results->setEnabled(true);
        QMap<QString, QString>::iterator i = results.begin();
        while (i != results.end()) {
            ui->results->addItem(i.value(), i.key());
            ++i;
        }
        ui->results->setCurrentIndex(0);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

QString SearchDialog::cityCode()
{
    return ui->results->itemData(ui->results->currentIndex()).toString();
}

QString SearchDialog::cityName()
{
    return QString("%1 (%2)").arg(ui->results->currentText()).arg(ui->department->currentText(), 3, '0');
}
