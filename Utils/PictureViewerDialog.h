//
// C++ Interface: PictureViewerDialog
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PICTUREVIEWERDIALOG_H
#define PICTUREVIEWERDIALOG_H

#include <QWidget>

#include <ui_PictureViewerDialog.h>

class PictureViewerDialog: public QDialog , public Ui::PictureViewerDialog
{
	Q_OBJECT

public:
    PictureViewerDialog(const QString& title, const QString &filename, QWidget *parent=NULL);
	PictureViewerDialog(const QString& title, const QPixmap& thePixmap, QWidget *parent=NULL);

public slots:
	void on_buttonBox_clicked(QAbstractButton * button);

protected:

private:

};

#endif
