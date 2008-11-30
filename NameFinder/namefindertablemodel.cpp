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
#include "namefindertablemodel.h"

namespace NameFinder
{

	NameFinderTableModel::NameFinderTableModel ( QObject *parent )
			: QAbstractTableModel ( parent )
	{
		showColumns = 3;
		myResults = NULL;
	}


	NameFinderTableModel::~NameFinderTableModel()
	{
	}


	int NameFinderTableModel::rowCount ( const QModelIndex &parent ) const
	{
		if ( myResults == NULL )
			return 0;
		if ( !myResults->isEmpty() )
			return myResults->count();
		return 0;
	}

	int NameFinderTableModel::columnCount ( const QModelIndex &parent ) const
	{
		return showColumns;
	}

	void NameFinderTableModel::setResults ( QList<NameFinderResult> *results )
	{
		myResults = results;
		reset();
	}
	QVariant NameFinderTableModel::headerData ( int section, Qt::Orientation orientation, int role ) const
	{
		if ( role != Qt::DisplayRole )
			return QVariant();
		if ( orientation == Qt::Horizontal )
		{
			switch ( section )
			{
				case 0:
					return QString ( tr ( "Name" ) );
					break;
				case 1:
					return QString ( tr ( "Type" ) );
					break;
				case 2:
					return QString ( tr ( "Near" ) );
					break;
				default:
					return QString ( tr ( "Unknown field" ) );
			}
		}
		else
		{
			return section;
		}
		return QVariant();
	}

	QVariant NameFinderTableModel::data ( const QModelIndex &index, int role ) const
	{
		if ( !index.isValid() )
			return QVariant();
		if ( role == Qt::TextAlignmentRole )
		{
			return int ( Qt::AlignCenter | Qt::AlignVCenter );
		}
		else if ( role == Qt::DisplayRole )
		{
			if ( myResults == NULL )
				return QVariant();
			if ( !myResults->isEmpty() )
			{
				switch ( index.column() )
				{
					case 0:
						return myResults->at ( index.row() ).name;
						break;
					case 1:
						return myResults->at ( index.row() ).type;
						break;
					case 2:
						return myResults->value ( index.row() ).near.value ( 0 ).name;
						break;
					default:
						return QVariant();
				}
			}
			return QVariant();
		}
		return QVariant();
	}
	NameFinderResult NameFinderTableModel::resultAt ( int index )
	{
		return myResults->at ( index );
	}
}
