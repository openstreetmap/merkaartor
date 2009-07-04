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
#ifndef NAMEFINDERWIDGET_H
#define NAMEFINDERWIDGET_H

#include <QtGui/QWidget>
#include "httpquery.h"
#include <QBuffer>
#include "namefindertablemodel.h"

namespace Ui {
    class NameFinderWidgetUi;
}

class QItemSelectionModel;
class QItemSelection;

namespace NameFinder {


class NameFinderWidget : public QWidget {
        Q_OBJECT
        Q_DISABLE_COPY(NameFinderWidget)
public:
        explicit NameFinderWidget(QWidget *parent = 0);
        virtual ~NameFinderWidget();
        /**
         * Executes the search
         * @param object Specifies a feature to search for, can be
         * any string which the service recognizes.
         * WARNING: this widget currently
         * doesn't do any data validation on it!
         */
        void search(QString object);
        //! Returns the coordinates of selected map feature
	QPointF selectedCoords();
        //! Returns the zoom level of the selected map feature
        int selectedZoom();

signals:
	void selectionChanged();
	void doubleClicked();
	void done();

protected:
        virtual void changeEvent(QEvent *e);


private:
        Ui::NameFinderWidgetUi *m_ui;

        QBuffer buffer;
        HttpQuery *query;
        NameFinderTableModel *model;
        QList<NameFinderResult> *results;
	QItemSelectionModel *selection;


private slots:
	void display();
	void displayError(QHttp::Error);
	void selection_selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
	void doubleClick();
    };
}
#endif // NAMEFINDERWIDGET_H
