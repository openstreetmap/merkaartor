/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#include <QtGui>

#include "mainwindow.h"
#include "qttoolbardialog.h"

MainWindow::MainWindow()
{
    textEdit = new QTextEdit;
    setCentralWidget(textEdit);

    findWidget = new QLineEdit;
    findWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    connect(findWidget, SIGNAL(returnPressed()), this, SLOT(find()));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createToolBarManager();

    readSettings();

    connect(textEdit->document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    setCurrentFile("");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::configureToolBars()
{
    QtToolBarDialog dlg(this);
    dlg.setToolBarManager(toolBarManager);
    dlg.exec();
}

void MainWindow::saveToolBars()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Toolbars' State"), QString(), "*.state");
    if (fileName.isEmpty())
        return;
    QFileInfo fi(fileName);
    if (fi.suffix() != QString("state"))
        fileName += QString(".state");

    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QByteArray array = toolBarManager->saveState();
        file.write(array);
        file.close();
    }
}

void MainWindow::restoreToolBars()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Restore Toolbars' State"), QString(), "*.state");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QByteArray array = file.readAll();
        file.close();
        toolBarManager->restoreState(array);
    }
}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void MainWindow::find()
{
    QString text = findWidget->text();
    if (!textEdit->find(text)) {
        statusBar()->showMessage(tr("Search hit bottom, continuing from top"), 2000);
        QTextCursor oldCursor = textEdit->textCursor();
        int vpos = textEdit->verticalScrollBar()->value();
        int hpos = textEdit->horizontalScrollBar()->value();
        QTextCursor newCursor = oldCursor;
        newCursor.setPosition(0);
        textEdit->setTextCursor(newCursor);
        if (!textEdit->find(text)) {
            statusBar()->showMessage(tr("Pattern '%1' not found").arg(text), 2000);
            textEdit->setTextCursor(oldCursor);
            textEdit->verticalScrollBar()->setValue(vpos);
            textEdit->horizontalScrollBar()->setValue(hpos);
        }
    }
}

void MainWindow::createActions()
{
    findAct = 0;

    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setObjectName(QString::fromUtf8("newAct"));
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setObjectName(QString::fromUtf8("openAct"));
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setObjectName(QString::fromUtf8("saveAct"));
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setObjectName(QString::fromUtf8("saveAsAct"));
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setObjectName(QString::fromUtf8("exitAct"));
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setObjectName(QString::fromUtf8("cutAct"));
    cutAct->setShortcut(tr("Ctrl+X"));
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setObjectName(QString::fromUtf8("copyAct"));
    copyAct->setShortcut(tr("Ctrl+C"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setObjectName(QString::fromUtf8("pasteAct"));
    pasteAct->setShortcut(tr("Ctrl+V"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

    configureToolBarsAct = new QAction(tr("&Configure Toolbars..."), this);
    configureToolBarsAct->setObjectName(QString::fromUtf8("configureToolBarsAct"));
    configureToolBarsAct->setStatusTip(tr("Configure toolbars"));
    QObject::connect(configureToolBarsAct, SIGNAL(triggered()),
                this, SLOT(configureToolBars()));

    saveToolBarsAct = new QAction(tr("&Save Toolbars..."), this);
    saveToolBarsAct->setObjectName(QString::fromUtf8("saveToolBarsAct"));
    saveToolBarsAct->setStatusTip(tr("Save toolbars' state"));
    QObject::connect(saveToolBarsAct, SIGNAL(triggered()),
                this, SLOT(saveToolBars()));

    restoreToolBarsAct = new QAction(tr("&Restore Toolbars..."), this);
    restoreToolBarsAct->setObjectName(QString::fromUtf8("restoreToolBarsAct"));
    restoreToolBarsAct->setStatusTip(tr("Restore toolbars' state"));
    QObject::connect(restoreToolBarsAct, SIGNAL(triggered()),
                this, SLOT(restoreToolBars()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setObjectName(QString::fromUtf8("aboutAct"));
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setObjectName(QString::fromUtf8("aboutQtAct"));
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    settingsMenu->addAction(configureToolBarsAct);
    settingsMenu->addAction(saveToolBarsAct);
    settingsMenu->addAction(restoreToolBarsAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(QString::fromUtf8("fileToolBar"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName(QString::fromUtf8("editToolBar"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addSeparator();

    findAct = editToolBar->addWidget(findWidget);
    findAct->setText(tr("Find"));
    findAct->setObjectName(QString::fromUtf8("findAct"));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createToolBarManager()
{
    toolBarManager = new QtToolBarManager(this);
    toolBarManager->setMainWindow(this);

    QString fileStr = tr("File");
    QString editStr = tr("Edit");
    QString settingsStr = tr("Settings");
    QString helpStr = tr("Help");

    toolBarManager->addToolBar(fileToolBar, fileStr);
    toolBarManager->addToolBar(editToolBar, editStr);

    toolBarManager->addAction(saveAsAct, fileStr);
    toolBarManager->addAction(exitAct, fileStr);
    toolBarManager->addAction(configureToolBarsAct, settingsStr);
    toolBarManager->addAction(saveToolBarsAct, settingsStr);
    toolBarManager->addAction(restoreToolBarsAct, settingsStr);
    toolBarManager->addAction(aboutAct, helpStr);
    toolBarManager->addAction(aboutQtAct, helpStr);
}

void MainWindow::readSettings()
{
    QSettings settings("Qt Software", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    QByteArray toolBarsState = settings.value("toolBarsState").toByteArray();
    QByteArray docksState = settings.value("docksState").toByteArray();
    resize(size);
    move(pos);
    toolBarManager->restoreState(toolBarsState);
    restoreState(docksState);
}

void MainWindow::writeSettings()
{
    QSettings settings("Qt Software", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("toolBarsState", toolBarManager->saveState());
    settings.setValue("docksState", saveState());
}

bool MainWindow::maybeSave()
{
    if (textEdit->document()->isModified()) {
        int ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);

    QString shownName;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = strippedName(curFile);

    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Application")));
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
