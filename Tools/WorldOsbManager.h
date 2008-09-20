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

#include <ui_WorldOsbManager.h>

class WorldOsbManager: public QDialog , public Ui::WorldOsbManager
{
	Q_OBJECT

	public:
		WorldOsbManager(QWidget *parent = 0);

		void DoIt();
		void generateRegion(quint32 rg);

	public slots:
		virtual void on_cbShowGrid_toggled(bool checked);
		virtual void on_buttonBox_clicked(QAbstractButton * button);
		virtual void on_WorldDirectoryBrowse_clicked();

	protected:

	private:
};

#endif
