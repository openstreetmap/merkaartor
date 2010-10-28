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

#ifndef CADASTREDOWNLOADDIALOG_H
#define CADASTREDOWNLOADDIALOG_H

#include "cadastrewrapper.h"
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace Ui {
    class CadastreDownloadDialog;
}

class CadastreDownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CadastreDownloadDialog(QWidget *parent = 0);
    ~CadastreDownloadDialog();

    QString getCityCode() { return m_resultCode; }
    QString getCityName() { return m_resultName; }
    QRect getBoundingBox() { return m_resultBBox; }
    QString getProjection() { return m_resultProjection; }
    QIODevice *getResultDevice() { return m_resultData; }

private slots:
    void on_buttonBox_accepted();
    void on_cityFilter_textChanged(const QString &text);
    void on_departments_currentIndexChanged(int index);
    void fillDepartments();
    void citiesReady(const QString &department);
    void citySelectionChanged(const QItemSelection & selected, const QItemSelection & deselected );
    void cityDownloaded(const QString &code, const QString &name, const QRect& bbox, const QString &projection, QIODevice* dev);
    void downloadFailed(const QString &name);
private:
    Ui::CadastreDownloadDialog *ui;
    CadastreWrapper *m_cadastre;
    QStandardItemModel *m_cityModel;
    QSortFilterProxyModel *m_proxyModel;

    QIODevice *m_resultData;
    QString m_resultCode;
    QRect m_resultBBox;
    QString m_resultName;
    QString m_resultProjection;
};

#endif // CADASTREDOWNLOADDIALOG_H
