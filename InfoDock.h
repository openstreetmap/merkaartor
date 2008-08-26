//
// C++ Interface: InfoDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef INFODOCK_H
#define INFODOCK_H

#include "Utils/MDockAncestor.h"
#include <QTextBrowser>

class MainWindow;
class MapFeature;

/**
	@author cbro <cbro@semperpax.com>
*/
class InfoDock : public MDockAncestor
{
Q_OBJECT
public:
    InfoDock(MainWindow* aParent);

    ~InfoDock();

public:
	void setHtml(QString html);

private slots:
	void on_anchorClicked(const QUrl & link);

private:
	MainWindow* Main;
	QTextBrowser* theText;
};

#endif
