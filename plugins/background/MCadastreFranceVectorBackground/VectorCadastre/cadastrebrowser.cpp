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

#include "cadastrebrowser.h"
#include "ui_cadastrebrowser.h"

#include "cadastredownloaddialog.h"
#include <QDebug>
#include <QDesktopServices>
#include <QSettings>
#include <QMessageBox>
#include "clippedpathitem.h"
#include "graphicproducer.h"
#include <QtConcurrentRun>
#include <QGraphicsScale>

CadastreBrowser::CadastreBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CadastreBrowser)
{
    ui->setupUi(this);


    m_menuMapper = new QSignalMapper(this);
    m_openMenu = new QMenu(this);
    m_cacheFolder = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    qDebug() << m_cacheFolder;
    ui->openButton->setMenu(m_openMenu);
    connect(m_menuMapper, SIGNAL(mapped(QString)), this, SLOT(openCity(QString)));

    this->fillMenu();

    qRegisterMetaType<QPainterPath>("QPainterPath");
    qRegisterMetaType<GraphicContext>("GraphicContext");
    qRegisterMetaType<Qt::FillRule>("Qt::FillRule");
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState);
}

CadastreBrowser::~CadastreBrowser()
{
    delete ui;
}

void CadastreBrowser::on_downloadButton_clicked()
{
    CadastreDownloadDialog *dial = new CadastreDownloadDialog(this);
    dial->setModal(true);
    if (dial->exec()) {
        qDebug() << "Cool !";
        qDebug() << dial->getCityCode() << dial->getCityName() << dial->getBoundingBox();
        if (!m_cacheFolder.exists(dial->getCityName()))
            m_cacheFolder.mkdir(dial->getCityCode());
        QDir cityFolder(m_cacheFolder);
        cityFolder.cd(dial->getCityCode());

        QSettings parameters(cityFolder.absoluteFilePath("settings.ini"), QSettings::IniFormat);
        parameters.setValue("name", dial->getCityName());
        parameters.setValue("code", dial->getCityCode());
        parameters.setValue("bbox", dial->getBoundingBox());
        parameters.sync();

        QFile pdf(cityFolder.absoluteFilePath("map.pdf"));
        pdf.open(QIODevice::WriteOnly);
        pdf.write(dial->getResultDevice()->readAll());
        pdf.close();
        dial->getResultDevice()->close();
        dial->getResultDevice()->deleteLater();

        this->fillMenu();
        this->openCity(dial->getCityCode());
    }
    dial->deleteLater();
}

void CadastreBrowser::fillMenu()
{
    m_openMenu->clear();
    bool hasContent = false;
    foreach (QFileInfo fileInfo, m_cacheFolder.entryInfoList()) {
        if (fileInfo.isDir()) {
            QDir folder(fileInfo.absoluteFilePath());
            if (folder.exists("map.pdf") && folder.exists("settings.ini")) {
                hasContent = true;
                QSettings parameters(folder.absoluteFilePath("settings.ini"), QSettings::IniFormat);

                QString name = parameters.value("name").toString();
                QString code = parameters.value("code").toString();

                QAction *action = m_openMenu->addAction(name);
                m_menuMapper->setMapping(action, code);
                connect(action, SIGNAL(triggered()), m_menuMapper, SLOT(map()));
            }
        }
    }
    ui->openButton->setEnabled(hasContent);
}

void CadastreBrowser::openCity(const QString &cityCode)
{
    QDir folder = QDir(m_cacheFolder.absoluteFilePath(cityCode));
    QString pdfFileName = folder.absoluteFilePath("map.pdf");
    QSettings parameters(folder.absoluteFilePath("settings.ini"), QSettings::IniFormat);

    QString name = parameters.value("name").toString();
    QString code = parameters.value("code").toString();
    QString bbox = parameters.value("bbox").toString();
    ui->status->setText(QString("City %1 : code %2, bounding box %3").arg(name).arg(code).arg(bbox));

    m_scene.clear();
    ui->graphicsView->setScene(0);
    ui->openButton->setEnabled(false);
    ui->downloadButton->setEnabled(false);

    // This may leak
    GraphicProducer *gp = new GraphicProducer(this);
    connect(gp, SIGNAL(fillPath(QPainterPath,GraphicContext,Qt::FillRule)), this, SLOT(fillPath(QPainterPath,GraphicContext,Qt::FillRule)), Qt::QueuedConnection);
    connect(gp, SIGNAL(strikePath(QPainterPath,GraphicContext)), this, SLOT(strikePath(QPainterPath,GraphicContext)), Qt::QueuedConnection);
    connect(gp, SIGNAL(parsingDone(bool)), this, SLOT(documentParsed(bool)));
    connect(gp, SIGNAL(parsingDone(bool)), gp, SLOT(deleteLater()));

    ui->status->setText("Loading... " + ui->status->text());
    // Hack to launch in a new thread ?
    QFuture<bool> result = QtConcurrent::run(gp, &GraphicProducer::parsePDF, pdfFileName);
}

void CadastreBrowser::documentParsed(bool success)
{
    if (!success)
        QMessageBox::critical(this, "Error", "An error occured while parsing the document");
    else
        ui->graphicsView->setScene(&m_scene);
    ui->status->setText(ui->status->text().mid(QString("Loading... ").length()));
    ui->openButton->setEnabled(true);
    ui->downloadButton->setEnabled(true);
}

void CadastreBrowser::fillPath(const QPainterPath &path, const GraphicContext &context, Qt::FillRule fillRule)
{
    QPainterPath m_path = path;
    m_path.setFillRule(fillRule);

    ClippedPathItem *item = new ClippedPathItem(m_path);
    item->setClip(context.clipPath);
    item->setPen(Qt::NoPen);
    item->setBrush(context.brush);
    item->scale(1, -1);/*
    QList<QGraphicsTransform*> transforms;
    // WARNING : These transformations will leak !
    QGraphicsScale *symetry = new QGraphicsScale(&m_scene);
    symetry->setXScale(1);
    symetry->setYScale(-1);
    transforms << symetry;
    if (context.brush.color() == Qt::black) {
        QGraphicsScale *zoom = new QGraphicsScale(&m_scene);
        zoom->setXScale(2);
        zoom->setYScale(2);
        zoom->setOrigin(QVector3D(item->boundingRect().x() + item->boundingRect().width()/2, item->boundingRect().y() + item->boundingRect().height()/2, 0));
        transforms << zoom;
    }
    item->setTransformations(transforms);*/
    m_scene.addItem(item);
}

void CadastreBrowser::strikePath(const QPainterPath &path, const GraphicContext &context)
{
    ClippedPathItem *item = new ClippedPathItem(path);
    item->setClip(context.clipPath);
    item->setPen(context.pen);
    item->setBrush(Qt::NoBrush);
    item->scale(1, -1);
    m_scene.addItem(item);
}
