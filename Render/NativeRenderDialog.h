//
// C++ Interface: NativeRenderDialog
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NATIVERENDERDIALOG_H
#define NATIVERENDERDIALOG_H

#include <QWidget>
#include <QSettings>

#include "Map/Coord.h"

#include <ui_NativeRenderDialog.h>

class MapDocument;
class MapView;
class CoordBox;

class NativeRenderDialog: public QDialog , public Ui::NativeRenderDialog
{
	Q_OBJECT

public:
    NativeRenderDialog(MapDocument *aDoc, const CoordBox& aCoordBox, QWidget *parent = 0);
	void render();

public slots:
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_sbMinLat_valueChanged(double v);
	void on_sbMinLon_valueChanged(double v);
	void on_sbMaxLat_valueChanged(double v);
	void on_sbMaxLon_valueChanged(double v);
	void on_sbPreviewWidth_valueChanged(int v);
	void on_sbPreviewHeight_valueChanged(int v);

protected:
	void calcRatio();

private:
	MapDocument* theDoc;
	QSettings*	Sets;
	double		ratio;

};

#endif
