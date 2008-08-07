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

PictureViewerDialog::PictureViewerDialog(const QString& title, const QString &filename, QWidget *parent)
    :QDialog(parent)
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
		QString s = QFileDialog::getSaveFileName(this,tr("Output filename"),"",tr("Image files (*.png *.jpg)"));
		if (!s.isNull()) {
			pixWidget->pixmap()->save(s);
		}
	}
}