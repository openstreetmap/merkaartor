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
    if (projection.compare("RGF93CC42", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 1 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=42.000000000 +lon_0=3.000000000 +lat_1=41.250000000 +lat_2=42.750000000 +x_0=1700000.000 +y_0=1200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC43", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 2 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=43.000000000 +lon_0=3.000000000 +lat_1=42.250000000 +lat_2=43.750000000 +x_0=1700000.000 +y_0=2200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC44", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 3 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=44.000000000 +lon_0=3.000000000 +lat_1=43.250000000 +lat_2=44.750000000 +x_0=1700000.000 +y_0=3200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC45", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 4 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=45.000000000 +lon_0=3.000000000 +lat_1=44.250000000 +lat_2=45.750000000 +x_0=1700000.000 +y_0=4200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC46", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 5 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=46.000000000 +lon_0=3.000000000 +lat_1=45.250000000 +lat_2=46.750000000 +x_0=1700000.000 +y_0=5200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC47", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 6 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=47.000000000 +lon_0=3.000000000 +lat_1=46.250000000 +lat_2=47.750000000 +x_0=1700000.000 +y_0=6200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC48", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 7 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=48.000000000 +lon_0=3.000000000 +lat_1=47.250000000 +lat_2=48.750000000 +x_0=1700000.000 +y_0=7200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC49", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 8 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=49.000000000 +lon_0=3.000000000 +lat_1=48.250000000 +lat_2=49.750000000 +x_0=1700000.000 +y_0=8200000.000 +units=m +no_defs";
    else if (projection.compare("RGF93CC50", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Projection conique conforme Zone 9 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=50.000000000 +lon_0=3.000000000 +lat_1=49.250000000 +lat_2=50.750000000 +x_0=1700000.000 +y_0=9200000.000 +units=m +no_defs";
    else if (projection.compare("LAMB1", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Lambert I +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=49.500000000 +lon_0=0.000000000 +k_0=0.99987734 +lat_1=49.500000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
    else if (projection.compare("LAMB2", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Lambert II +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=46.800000000 +lon_0=0.000000000 +k_0=0.99987742 +lat_1=46.800000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
    else if (projection.compare("LAMB3", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Lambert III +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=44.100000000 +lon_0=0.000000000 +k_0=0.99987750 +lat_1=44.100000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
    else if (projection.compare("LAMB4", Qt::CaseInsensitive) == 0)
        m_resultProjection = "+title=Lambert IV +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=42.165000000 +lon_0=0.000000000 +k_0=0.99994471 +lat_1=42.165000000 +x_0=234.358 +y_0=185861.369 +units=m +no_defs";
//    m_resultProjection = projection;
}

void CadastreDownloadDialog::downloadFailed(const QString &name)
{
    QMessageBox::critical(this, "Download failed", QString("Download of %1 failed.").arg(name));
}
