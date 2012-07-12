//
// C++ Interface: RelationMemberDelegate
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RELATIONMEMBERDELEGATE_H
#define RELATIONMEMBERDELEGATE_H

#include <QItemDelegate>
#include <QComboBox>
#include <QModelIndex>
#include <QObject>
#include <QCompleter>

#include "Feature.h"

/**
    @author cbro <cbro@semperpax.com>
*/
class RelationMemberDelegate : public QItemDelegate
{
Q_OBJECT
public:
    RelationMemberDelegate(const QStringList& restrictedRoles = QStringList(), const QList<IFeature::FeatureType>& restrictedTypes = QList<IFeature::FeatureType>(), QObject* parent = 0);

    ~RelationMemberDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    bool eventFilter(QObject* object, QEvent* event);

protected:
    QStringList RestrictedRoles;
    QList<IFeature::FeatureType> RestrictedTypes;
    mutable QAbstractItemModel* curModel;
    mutable QModelIndex curIndex;

protected slots:
    void onFeatureSelected(Feature* F);
};

#endif
