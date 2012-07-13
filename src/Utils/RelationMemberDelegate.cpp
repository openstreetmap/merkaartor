//
// C++ Implementation: RelationMemberDelegate
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Global.h"

#include "RelationMemberDelegate.h"
#include "Document.h"
#include "TagModel.h"

#include "SelectInteraction.h"
#include "EditInteraction.h"

#include <QLineEdit>
#include <QKeyEvent>

RelationMemberDelegate::RelationMemberDelegate(const QStringList &restrictedRoles, const QList<IFeature::FeatureType> &restrictedTypes, QObject *parent)
    : QItemDelegate(parent)
    , RestrictedRoles(restrictedRoles)
    , RestrictedTypes(restrictedTypes)
{
}

RelationMemberDelegate::~RelationMemberDelegate()
{
}

QWidget* RelationMemberDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& index) const
{
    QWidget* edit;

    if (index.column() == 0) {

        if (index.row() >= index.model()->rowCount()-1) {
            QLineEdit* le = new QLineEdit(parent);
            le->setReadOnly(true);
            le->setPlaceholderText(RelationMemberModel::newMemberText());
            edit = le;
        } else if (RestrictedRoles.size()) {
            QComboBox *cb = new QComboBox(parent);
            cb->setInsertPolicy(QComboBox::InsertAlphabetically);
            cb->insertItems(-1, RestrictedRoles);

            edit = cb;
        } else {
            QLineEdit* le = new QLineEdit(parent);
            edit = le;
        }
    } else {
        QLineEdit* le = new QLineEdit(parent);
        le->setReadOnly(true);
        edit = le;

        curIndex = index;
        curModel = const_cast<QAbstractItemModel*>(index.model());
        SelectInteraction* selI = new SelectInteraction();
        connect(selI, SIGNAL(setSelection(Feature*)), this, SLOT(onFeatureSelected(Feature*)));
        CUR_MAINWINDOW->launchInteraction(selI);
    }
    return edit;
}

void RelationMemberDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.column() == 0) {
        if (QComboBox *edit = dynamic_cast<QComboBox*>(editor)) {
            if (index.model()->data(index).toString() != RelationMemberModel::newMemberText()) {
                int idx = edit->findText(index.model()->data(index).toString());
                if (idx > -1)
                    edit->setCurrentIndex(idx);
                else
                    edit->setEditText(index.model()->data(index).toString());
            } else
                edit->clearEditText();
            //        edit->lineEdit()->selectAll();
        } else {
            if (QLineEdit *edit = dynamic_cast<QLineEdit*>(editor)) {
                edit->setText(index.model()->data(index).toString());
            }
        }
    } else {
        if (QLineEdit *edit = dynamic_cast<QLineEdit*>(editor))
            edit->setText(tr("Select feature..."));
    }
}

void RelationMemberDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.column() != 0)
        return;

    QString newVal;
    if (QComboBox *edit = dynamic_cast<QComboBox*>(editor))
        newVal = edit->currentText();
    else
        if (QLineEdit *edit = dynamic_cast<QLineEdit*>(editor))
            newVal = edit->text();

    if (newVal == index.model()->data(index).toString()) return;
    if (!newVal.isEmpty())
        model->setData(index, newVal);
}

void RelationMemberDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /* index */) const
{
    editor->setGeometry(option.rect);
}

/* On enter commit the current text and move to the next field */
bool RelationMemberDelegate::eventFilter(QObject* object, QEvent* event)
{
    QWidget* editor = qobject_cast<QWidget*>(object);
    if (!editor)
        return false;

    // Note that keys already bound to shortcuts will be QEvent::ShortcutOverride not QEvent::KeyPress
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent*)event;
        switch (keyEvent->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::EditNextItem);
            return true;
        }
    }
    return QItemDelegate::eventFilter(object, event);
}

void RelationMemberDelegate::onFeatureSelected(Feature *F)
{
    Feature* curF = NULL;
    if (curIndex.row() < curModel->rowCount() - 1)
        curF = curModel->data(curIndex, Qt::UserRole).value<Feature*>();
    if (F != curF) {
        curModel->setData(curIndex, qVariantFromValue((void*)F));
        CUR_MAINWINDOW->invalidateView();
    }
    QTimer::singleShot(0, CUR_MAINWINDOW, SLOT(launchInteraction()));
}
