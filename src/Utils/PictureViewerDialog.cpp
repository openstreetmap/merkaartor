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
			QString s = QFileDialog::getSaveFileName(this,tr("Output filename"),"",tr("SVG files (*.svg)"));
			if (!s.isNull()) {
				QFile(s).remove();
				QFile(m_filename).copy(s);
			}
		} else {
			QString s = QFileDialog::getSaveFileName(this,tr("Output filename"),"",tr("Image files (*.png *.jpg)"));
			if (!s.isNull()) {
				pixWidget->pixmap()->save(s);
			}
		}
	}
}
