//
// C++ Interface: editcompleterdelegate
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EDITCOMPLETERDELEGATE_H
#define EDITCOMPLETERDELEGATE_H

#include <QItemDelegate>
#include <QComboBox>
#include <QModelIndex>
#include <QObject>
#include <QCompleter>

/**
	@author cbro <cbro@semperpax.com>
*/
        class EditCompleterDelegate : public QItemDelegate
{
Q_OBJECT
public:
    EditCompleterDelegate(QObject* parent = 0);

    ~EditCompleterDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif
