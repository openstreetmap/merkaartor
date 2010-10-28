/* This file is part of Qadastre
 * Copyright (C) 2010 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "cadastredownloaddialog.h"
#include "ui_cadastredownloaddialog.h"

#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>

CadastreDownloadDialog::CadastreDownloadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CadastreDownloadDialog)
{
    ui->setupUi(this);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_cityModel = new QStandardItemModel(this);
    m_proxyModel->setSourceModel(m_cityModel);
    ui->cityListView->setModel(m_proxyModel);
    m_proxyModel->setSortRole(Qt::DisplayRole);
    m_proxyModel->sort(0);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setDynamicSortFilter(true);

    connect(ui->cityListView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(citySelectionChanged(QItemSelection,QItemSelection)));

    ui->departments->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_cadastre = new CadastreWrapper(this);
    connect(m_cadastre, SIGNAL(departmentAvailable()), this, SLOT(fillDepartments()));
    connect(m_cadastre, SIGNAL(citiesAvailable(QString)), this, SLOT(citiesReady(QString)));
    connect(m_cadastre, SIGNAL(cityDownloaded(QString,QString,QString,QIODevice*)), this, SLOT(cityDownloaded(QString,QString,QString,QIODevice*)));
    connect(m_cadastre, SIGNAL(downloadFailed(QString)), this, SLOT(downloadFailed(QString)));
    m_cadastre->requestDepartmentList();

    m_resultCode = QString();
    m_resultName = QString();
    m_resultBBox = QRect();
    m_resultProjection = QString();
    m_resultData = 0;
}

CadastreDownloadDialog::~CadastreDownloadDialog()
{
    delete ui;
}

void CadastreDownloadDialog::fillDepartments()
{
    qDebug() << "Fill departments";
    ui->departments->clear();
    ui->departments->addItem("Select a department");
    QMap<QString, QString> depts = m_cadastre->listDepartments();
    QMap<QString, QString>::const_iterator i = depts.constBegin();
    while (i != depts.constEnd()) {
        ui->departments->addItem(i.value(), i.key());
        i++;
    }
    ui->departments->setEnabled(true);
    ui->departments->setFocus();
    qDebug() << "/Fill departments";
}

void CadastreDownloadDialog::citiesReady(const QString &department)
{
    if (department == ui->departments->itemData(ui->departments->currentIndex())) {
        qDebug() << "Ok, good !";
        m_cityModel->clear();

        ui->cityFilter->setEnabled(true);
        ui->cityListView->setEnabled(true);

        QMap<QString, QString> cities = m_cadastre->listCities(department);
        m_cityModel->setRowCount(cities.count());

        QMap<QString, QString>::const_iterator i = cities.constBegin();
        int counter = 0;
        while (i != cities.constEnd()) {
            QStandardItem * item = new QStandardItem(i.value());
            item->setData(i.key());
            m_cityModel->setItem(counter, item);
            i++;
            counter++;
        }
    }
}

void CadastreDownloadDialog::on_departments_currentIndexChanged(int index)
{
    if (index >= 0) {
        QString dept = ui->departments->itemData(index).toString();
        if (!dept.isEmpty()) {
            // Request the cities then
            m_cadastre->requestCities(dept);
        }
    }
}

void CadastreDownloadDialog::on_cityFilter_textChanged(const QString &text)
{
    m_proxyModel->setFilterFixedString(text);
    if (m_proxyModel->rowCount() == 1) {
        qDebug() << "Cool !!";
        ui->cityListView->selectAll();
        ui->cityListView->selectionModel()->select(m_proxyModel->index(0, 0), QItemSelectionModel::ClearAndSelect);
    } else if (m_proxyModel->rowCount() == 0) {
        qDebug() << "Pas cool";
        ui->cityListView->selectionModel()->select(QModelIndex(), QItemSelectionModel::Clear);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void CadastreDownloadDialog::citySelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(selected.count() == 1);
}

void CadastreDownloadDialog::on_buttonBox_accepted()
{
    if (ui->cityListView->selectionModel()->selection().count() != 1) {
        QMessageBox::critical(this, "Select a city", "You must select a city first.");
        return;
    }
    QMap<int, QVariant> item = m_proxyModel->itemData(ui->cityListView->selectionModel()->selection().indexes()[0]);
    QString cityName = item.value(Qt::DisplayRole).toString();
    QString cityCode = item.value(Qt::UserRole+1).toString();

    qDebug() << cityName << cityCode;

    QProgressDialog *progressDialog = new QProgressDialog(this);
    progressDialog->setModal(true);
    connect(m_cadastre, SIGNAL(cityDownloaded(QString,QString,QRect,QString,QIODevice*)), progressDialog, SLOT(accept()));
    connect(m_cadastre, SIGNAL(downloadFailed(QString)), progressDialog, SLOT(reject()));
    m_cadastre->requestPDF(cityCode, cityName);
    progressDialog->setCancelButton(0);
    progressDialog->setMaximum(0);
    progressDialog->setMinimum(0);
    progressDialog->setLabelText("This operation can last a few minutes, depending on the city size...");
    if (progressDialog->exec())
        accept();
}

void CadastreDownloadDialog::cityDownloaded(const QString &code, const QString &name, const QRect &bbox, const QString& projection, QIODevice *dev)
{
    qDebug() << "City downloaded ??";
    m_resultCode = code;
    m_resultName = name;
    m_resultData = dev;
    m_resultBBox = bbox;
    m_resultProjection = projection;
}

void CadastreDownloadDialog::downloadFailed(const QString &name)
{
    QMessageBox::critical(this, "Download failed", QString("Download of %1 failed.").arg(name));
}
