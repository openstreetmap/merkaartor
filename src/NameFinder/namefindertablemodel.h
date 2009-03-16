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
#ifndef NAMEFINDERNAMEFINDERTABLEMODEL_H
#define NAMEFINDERNAMEFINDERTABLEMODEL_H

#include <QAbstractTableModel>
#include <NameFinderResult.h>
#include <QList>

namespace NameFinder {

    /** This class implements a model for QTableView to filter out the unrelevant data as easy as possible
    	@author Łukasz Jernaś <deejay1@srem.org>
    */
class NameFinderTableModel : public QAbstractTableModel {
        Q_OBJECT
public:
        NameFinderTableModel(QObject *parent = 0);

        ~NameFinderTableModel();

        int rowCount(const QModelIndex &parent) const;
        //! Returns the number of columns to display - currently capped at 3
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        //!
        void setResults(QList<NameFinderResult> *results);
        NameFinderResult resultAt(int index);
        QList<NameFinderResult> *myResults;

private:

        //! How many columns to show
        int showColumns;
    };

}

#endif
