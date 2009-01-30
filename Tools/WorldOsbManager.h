//
// C++ Interface: WorldOsbManager
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WORLDOSBMANAGER_H
#define WORLDOSBMANAGER_H

#include "IProgressWindow.h"
#include <ui_WorldOsbManager.h>

class QFile;

class WorldOsbManager: public QDialog , public Ui::WorldOsbManager, public IProgressWindow
{
	Q_OBJECT

	public:
		WorldOsbManager(QWidget *parent = 0);
		~WorldOsbManager();

		void DoIt();
		bool generateRegion(quint32 rg);
		bool deleteRegion(quint32 rg);

	public slots:
		virtual void on_cbShowGrid_toggled(bool checked);
		virtual void on_buttonBox_clicked(QAbstractButton * button);
		virtual void on_WorldDirectoryBrowse_clicked();

	protected:
		QFile* WorldFile;

		void readWorld();

	private:
};

#endif
