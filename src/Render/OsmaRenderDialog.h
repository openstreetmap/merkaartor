//
// C++ Interface: OsmaRenderDialog
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OSMARENDERDIALOG_H
#define OSMARENDERDIALOG_H

#include <QWidget>
#include <QSettings>

#include "Maps/Coord.h"
#include "Maps/Projection.h"

#include <ui_OsmaRenderDialog.h>

class MapDocument;
class Projection;
class CoordBox;

class OsmaRenderDialog: public QDialog , public Ui::OsmaRenderDialog
{
	Q_OBJECT

public:
    OsmaRenderDialog(MapDocument *aDoc, const CoordBox& aCoordBox, QWidget *parent = 0);
	void render();

public slots:
	void on_btBrowseOutFilename_clicked();
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_sbMinLat_valueChanged(double v);
	void on_sbMinLon_valueChanged(double v);
	void on_sbMaxLat_valueChanged(double v);
	void on_sbMaxLon_valueChanged(double v);
	void on_sbDPI_valueChanged(int v);
	void on_sbScale_valueChanged(double v);
	void on_svgOutputFilename_textChanged();

protected:
	void refreshLabels();

private:
	QString OutFilename;
	MapDocument* theDoc;
	QSettings*	SvgSets;
	QString		InkPath;
	double		HeightMeter;
	double		WidthMeter;

};

#endif
