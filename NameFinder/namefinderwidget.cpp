/***************************************************************************
 *   Copyright (C) 2008 by Łukasz Jernaś   *
 *   deejay1@srem.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "namefinderwidget.h"
#include "ui_namefinderwidget.h"
#include "xmlstreamreader.h"
#include "NameFinderResult.h"
#include <QList>
#include <QHeaderView>

#include "Preferences/MerkaartorPreferences.h"

namespace NameFinder
{
	NameFinderWidget::NameFinderWidget ( QWidget *parent ) :
			QWidget ( parent ),
			m_ui ( new Ui::NameFinderWidgetUi )
	{
		m_ui->setupUi ( this );
		model = new NameFinderTableModel();
		m_ui->tableView->setModel ( model );
		m_ui->tableView->horizontalHeader()->setStretchLastSection(true);
		selection = m_ui->tableView->selectionModel();
		connect(selection, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this,SLOT(selection_selectionChanged(const QItemSelection&,const QItemSelection&)));
		connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT (doubleClick()));
	}

	NameFinderWidget::~NameFinderWidget()
	{
		delete m_ui;
		delete model;
		delete selection;
	}

	void NameFinderWidget::changeEvent ( QEvent *e )
	{
		switch ( e->type() )
		{
			case QEvent::LanguageChange:
				m_ui->retranslateUi ( this );
				break;
			default:
				break;
		}
	}

	void NameFinderWidget::search ( QString object )
	{
		query = new HttpQuery ( this, &buffer );
		if (M_PREFS->getProxyUse())
			query->setProxy(M_PREFS->getProxyHost(), M_PREFS->getProxyPort());
		connect ( query, SIGNAL ( done() ), this, SLOT ( display() ) );
		query->startSearch ( object );
	}

	void NameFinderWidget::display()
	{
		XmlStreamReader *reader = new XmlStreamReader ( &buffer );
		reader->read();
		model->setResults ( new QList<NameFinderResult> ( reader->getResults() ) );
	}

	QPointF NameFinderWidget::selectedCoords()
	{
		QModelIndexList selectedIndexes = selection->selectedIndexes();
		QModelIndex selectedIndex;
		NameFinderResult result;
		foreach (selectedIndex, selectedIndexes)
		{
			result = model->resultAt ( selectedIndex.row() );
			return QPointF ( result.lat, result.lon );
		}
		return QPointF ();

	}

	void NameFinderWidget::selection_selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
	{
		emit selectionChanged();
	}

	void NameFinderWidget::doubleClick()
	{
	    emit doubleClicked();
	}
}
