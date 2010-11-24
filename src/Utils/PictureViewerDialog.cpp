//
// C++ Implementation: PictureViewerDialog
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PictureViewerDialog.h"

#include <QFileDialog>
#include <QFile>

PictureViewerDialog::PictureViewerDialog(const QString& title, const QString &filename, QWidget *parent)
    :QDialog(parent), m_filename(filename)
{
    setupUi(this);

    pixWidget->loadFile(filename);
    setWindowTitle(title);
}

PictureViewerDialog::PictureViewerDialog(const QString& title, const QPixmap& thePixmap, QWidget *parent)
    :QDialog(parent)
{
    setupUi(this);

    pixWidget->setPixmap(thePixmap);
    setWindowTitle(title);
}

void PictureViewerDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if (buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
        if (m_filename.endsWith("svg", Qt::CaseInsensitive)) {
            QString s;
            QFileDialog dlg(this, tr("Output filename"), QString("%1.svg").arg(tr("untitled")), tr("SVG files (*.svg)") + "\n" + tr("All Files (*)"));
            dlg.setFileMode(QFileDialog::AnyFile);
            dlg.setDefaultSuffix("svg");
            dlg.setAcceptMode(QFileDialog::AcceptSave);

            if (dlg.exec()) {
                if (dlg.selectedFiles().size())
                    s = dlg.selectedFiles()[0];
            }
//			QString s = QFileDialog::getSaveFileName(this,tr("Output filename"),"",tr("SVG files (*.svg)"));
            if (!s.isNull()) {
                QFile(s).remove();
                QFile(m_filename).copy(s);
            }
        } else {
            QString s;
            QFileDialog dlg(this, tr("Output filename"), QString("%1.png").arg(tr("untitled")), tr("Image files (*.png *.jpg)") + "\n" + tr("All Files (*)"));
            dlg.setFileMode(QFileDialog::AnyFile);
            dlg.setDefaultSuffix("png");
            dlg.setAcceptMode(QFileDialog::AcceptSave);

            if (dlg.exec()) {
                if (dlg.selectedFiles().size())
                    s = dlg.selectedFiles()[0];
            }
//			QString s = QFileDialog::getSaveFileName(this,tr("Output filename"),"",tr("Image files (*.png *.jpg)"));
            if (!s.isNull()) {
                pixWidget->pixmap()->save(s);
            }
        }
    }
}
