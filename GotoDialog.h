//
// C++ Interface: GotoDialog
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QWidget>
#include <QSettings>

#include "Map/Coord.h"
#include "Map/Projection.h"

#include <ui_GotoDialog.h>

class CoordBox;
class QItemSelection;
namespace NameFinder
{
	class NameFinderWidget;
}

class GotoDialog: public QDialog , public Ui::GotoDialog
{
	Q_OBJECT

public:
    GotoDialog(const Projection& aProj, QWidget *parent = 0);

	const CoordBox& newViewport() const { return theNewViewport; };
	NameFinder::NameFinderWidget *searchWidget;

public slots:
	void on_buttonBox_clicked(QAbstractButton * button);
	void on_searchButton_clicked();
	void searchWidget_selectionChanged();
	void on_NameFinderEdit_textChanged(const QString & text);
	void searchWidget_doubleClicked ();

protected:
	
private:
	Projection theProjection;
	CoordBox theNewViewport;

	void changeEvent(QEvent*);

};

#endif
