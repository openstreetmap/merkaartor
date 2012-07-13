//
// C++ Implementation: TagTemplate
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "TagTemplate.h"

#include "Global.h"
#include "Features.h"
#include "FeatureCommands.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "RelationCommands.h"
#include "RelationMemberDelegate.h"

#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QListView>
#include <QRegExp>
#include <QMenu>

/** TagTemplateWidget **/

TagTemplateWidget::TagTemplateWidget()
    : theMainWidget(0), theWidget(0), theLabelWidget(0), theSelector(0)
{
}

TagTemplateWidget::~TagTemplateWidget()
{
    // No need to delete; will be destroyed automatically by parent + crash if no active widget.
    // delete theMainWidget;
    for (int i=0; i<theValues.size(); ++i)
        delete theValues[i];
    delete theSelector;
}

TagTemplateWidget* TagTemplateWidget::fromXml(const QDomElement& e)
{
    if (e.tagName() != "widget") {
        return NULL;
    }

    TagTemplateWidget* aTW = NULL;
    if (e.attribute("type") == "combo") {
        aTW = TagTemplateWidgetCombo::fromXml(e);
    } else if (e.attribute("type") == "yesno") {
        aTW = TagTemplateWidgetYesno::fromXml(e);
    } else if (e.attribute("type") == "constant") {
        aTW = TagTemplateWidgetConstant::fromXml(e);
    } else if (e.attribute("type") == "edit") {
        aTW = TagTemplateWidgetEdit::fromXml(e);
    } else if (e.attribute("type") == "memberlist") {
        aTW = TagTemplateWidgetMemberList::fromXml(e);
    } else
        Q_ASSERT(false);

    if (aTW) {
        aTW->theId = e.attribute("id");
        aTW->theType = e.attribute("type");
        aTW->theTag = e.attribute("tag");
    }

    return aTW;
}

bool TagTemplateWidget::parseCommonElements(const QDomElement& e)
{
    if (e.tagName() == "description") {
        theDescriptions.insert(e.attribute("locale"), e.firstChild().toText().nodeValue());
        return true;
    } else if (e.tagName() == "link") {
        theUrl = QUrl(e.attribute("src"));
        return true;
    } else if (e.tagName() == "selector") {
        theSelector = TagSelector::parse(e.attribute("expr"));
        return true;
    } else if (e.tagName() == "value") {
        TagTemplateWidgetValue* aTCV = TagTemplateWidgetValue::fromXml(e);
        if (aTCV)
            theValues.append(aTCV);
        return true;
    }
    return false;
}

void TagTemplateWidget::generateCommonElements(QDomElement& e)
{
    if (!theUrl.isEmpty()) {
        QDomElement c = e.ownerDocument().createElement("link");
        e.appendChild(c);

        c.setAttribute("src", theUrl.toString());
    }

    QHashIterator<QString, QString > itD(theDescriptions);
    while(itD.hasNext()) {
        itD.next();

        QDomElement d = e.ownerDocument().createElement("description");
        e.appendChild(d);

        d.setAttribute("locale", itD.key());

        QDomText v = e.ownerDocument().createTextNode(itD.value());
        d.appendChild(v);
    }

    if (theSelector) {
        QDomElement s = e.ownerDocument().createElement("selector");
        e.appendChild(s);
        s.setAttribute("expr", theSelector->asExpression(false));
    }

    for (int i=0; i<theValues.size(); ++i) {
        theValues[i]->toXML(e, false);
    }
}

/** TagTemplateWidgetValue **/

TagTemplateWidgetValue* TagTemplateWidgetValue::fromXml(const QDomElement& e)
{
    TagTemplateWidgetValue* aTCV = new TagTemplateWidgetValue(e.attribute("tag"));

    for(QDomNode d = e.firstChild(); !d.isNull(); d = d.nextSibling()) {
        QDomElement c = d.toElement();
        if (c.isNull())
            continue;

        aTCV->parseCommonElements(c);
    }
    return aTCV;
}

bool TagTemplateWidgetValue::toXML(QDomElement& xParent, bool /* header */)
{
    QDomElement v = xParent.ownerDocument().createElement("value");
    xParent.appendChild(v);

    v.setAttribute("tag", theTagValue);
    generateCommonElements(v);

    return true;
}


/** TagTemplateWidgetCombo **/

TagTemplateWidgetCombo::~TagTemplateWidgetCombo()
{
}

QWidget* TagTemplateWidgetCombo::getWidget(const Feature* F, const MapView* V)
{
    if (theSelector && (theSelector->matches(F,V->pixelPerM()) != TagSelect_Match && theSelector->matches(F,V->pixelPerM()) != TagSelect_DefaultMatch))
        return NULL;

    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    QHBoxLayout* aLayout = new QHBoxLayout(theWidget);
    aLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* aLabel = new QLabel();
    aLabel->setOpenExternalLinks(true);
    aLabel->setWordWrap(true);
    aLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    aLabel->setMaximumWidth(55);
    aLayout->addWidget(aLabel);

    QString url;
    if (theDescriptions.count(lang))
        theDescription = theDescriptions[lang];
    else
        if (theDescriptions.count(defLang))
            theDescription = theDescriptions[defLang];
        else
            theDescription = theTag;
    if (!theUrl.isEmpty())
        url = theUrl.toString();

    QComboBox* aCombo = new QComboBox();
    aCombo->setEditable(true);
    //	aCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    aCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    aCombo->setMinimumWidth(100);
    aLayout->addWidget(aCombo);

    aCombo->addItem(tr("Undefined"), qVariantFromValue(new TagTemplateWidgetValue("__NULL__")));
    QString val = F->tagValue(theTag, "__NULL__");
    int idx = -1;
    for (int i=0; i<theValues.size(); ++i) {
        if (theValues[i]->theDescriptions.count(lang))
            aCombo->addItem(theValues[i]->theDescriptions[lang], qVariantFromValue(theValues[i]));
        else
            if (theValues[i]->theDescriptions.count(defLang))
                aCombo->addItem(theValues[i]->theDescriptions[defLang], qVariantFromValue(theValues[i]));
            else
                aCombo->addItem(theValues[i]->theTagValue,  qVariantFromValue(theValues[i]));

        if (theValues[i]->theTagValue == val)
            idx = aCombo->count() - 1;
    }

    if (val != "__NULL__") {
        if (idx == -1) {
            aCombo->insertItem(1, val);
            aCombo->setCurrentIndex(1);
        } else {
            aCombo->setCurrentIndex(idx);
            if (!theValues[idx-1]->theUrl.isEmpty())
                url = theValues[idx-1]->theUrl.toString();
        }
    } else
        aCombo->setCurrentIndex(0);

    if (!url.isEmpty())
        aLabel->setText(QString("<a href=\"%1\">%2</a>").arg(url).arg(theDescription));
    else
        aLabel->setText(theDescription);

    connect(aCombo,SIGNAL(activated(int)), this, SLOT(on_combo_activated(int)));
    theMainWidget = aCombo;
    theLabelWidget = aLabel;

    return theWidget;
}

TagTemplateWidgetCombo* TagTemplateWidgetCombo::fromXml(const QDomElement& e)
{
    TagTemplateWidgetCombo* aCW = new TagTemplateWidgetCombo();

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        aCW->parseCommonElements(t);
    }

    return aCW;
}

bool TagTemplateWidgetCombo::toXML(QDomElement& xParent, bool header)
{
    bool OK = true;

    if (!header) {
        QDomElement e = xParent.ownerDocument().createElement("widgetref");
        xParent.appendChild(e);
        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        else
            e.setAttribute("id", theTag);
    } else {
        QDomElement e = xParent.ownerDocument().createElement("widget");
        xParent.appendChild(e);

        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        e.setAttribute("type", theType);
        e.setAttribute("tag", theTag);
        generateCommonElements(e);
    }

    return OK;
}

void TagTemplateWidgetCombo::on_combo_activated(int idx)
{
    QComboBox* W = dynamic_cast<QComboBox*>(theMainWidget);
    TagTemplateWidgetValue* aTCV = W->itemData(idx).value<TagTemplateWidgetValue*>();

    QString val;
    if (aTCV) {
        if (!aTCV->theUrl.isEmpty()) {
            ((QLabel *)theLabelWidget)->setText(QString("<a href=\"%1\">%2</a>").arg(aTCV->theUrl.toString()).arg(theDescription));
        } else {
            if (!theUrl.isEmpty()) {
                ((QLabel *)theLabelWidget)->setText(QString("<a href=\"%1\">%2</a>").arg(theUrl.toString()).arg(theDescription));
            } else {
                ((QLabel *)theLabelWidget)->setText(theDescription);
            }
        }

        val = aTCV->theTagValue;
    } else
        val = W->currentText();

    if (val == "__NULL__")
        emit tagCleared(theTag);
    else
        emit tagChanged(theTag, val);
}

void TagTemplateWidgetCombo::apply(const Feature*)
{
}

/** TagTemplateWidgetYesno **/

QWidget* TagTemplateWidgetYesno::getWidget(const Feature* F, const MapView* V)
{
    if (theSelector && (theSelector->matches(F,V->pixelPerM()) != TagSelect_Match && theSelector->matches(F,V->pixelPerM()) != TagSelect_DefaultMatch))
        return NULL;

    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    QHBoxLayout* aLayout = new QHBoxLayout(theWidget);
    aLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* aLabel = new QLabel();
    aLabel->setOpenExternalLinks(true);
    aLayout->addWidget(aLabel);

    QCheckBox* aCB = new QCheckBox();
    aCB->setTristate();
    aLayout->addWidget(aCB);

    if (theDescriptions.count(lang))
        theDescription = theDescriptions[lang];
    else
        if (theDescriptions.count(defLang))
            theDescription = theDescriptions[defLang];
        else
            theDescription = theTag;

    QString val = F->tagValue(theTag, "__NULL__");
    if (val == "yes" || val == "1" || val == "true")
        aCB->setCheckState(Qt::Checked);
    else
        if (val == "no" || val == "0" || val == "false")
            aCB->setCheckState(Qt::Unchecked);
        else
            aCB->setCheckState(Qt::PartiallyChecked);

    if (!theUrl.isEmpty())
        aLabel->setText(QString("<a href=\"%1\">%2</a>").arg(theUrl.toString()).arg(theDescription));
    else
        aLabel->setText(theDescription);

    connect(aCB,SIGNAL(stateChanged(int)), this, SLOT(on_checkbox_stateChanged (int)));
    theMainWidget = aCB;
    theLabelWidget = aLabel;

    return theWidget;
}

void TagTemplateWidgetYesno::on_checkbox_stateChanged(int state)
{
    //QCheckBox* W = dynamic_cast<QCheckBox*>(theMainWidget);
    if (state == Qt::PartiallyChecked) {
        emit tagCleared(theTag);
    } else {
        QString value = (state == Qt::Checked) ? "yes" : "no";
        emit tagChanged(theTag, value);
    }
}

void TagTemplateWidgetYesno::apply(const Feature*)
{
}

TagTemplateWidgetYesno* TagTemplateWidgetYesno::fromXml(const QDomElement& e)
{
    TagTemplateWidgetYesno* aCB = new TagTemplateWidgetYesno();

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        aCB->parseCommonElements(t);
    }

    return aCB;
}

bool TagTemplateWidgetYesno::toXML(QDomElement& xParent, bool header)
{
    bool OK = true;

    if (!header) {
        QDomElement e = xParent.ownerDocument().createElement("widgetref");
        xParent.appendChild(e);
        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        else
            e.setAttribute("id", theTag);
    } else {
        QDomElement e = xParent.ownerDocument().createElement("widget");
        xParent.appendChild(e);

        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        e.setAttribute("type", theType);
        e.setAttribute("tag", theTag);
        generateCommonElements(e);
    }

    return OK;
}

/** TagTemplateWidgetConstant **/

TagTemplateWidgetConstant::~TagTemplateWidgetConstant()
{
}

QWidget* TagTemplateWidgetConstant::getWidget(const Feature* F, const MapView* V)
{
    if (theSelector && (theSelector->matches(F,V->pixelPerM()) != TagSelect_Match && theSelector->matches(F,V->pixelPerM()) != TagSelect_DefaultMatch))
        return NULL;

    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    QHBoxLayout* aLayout = new QHBoxLayout(theWidget);
    aLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* aLabel = new QLabel();
    aLabel->setOpenExternalLinks(true);
    aLabel->setWordWrap(true);
    aLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    aLabel->setMaximumWidth(55);
    aLayout->addWidget(aLabel);

    QString url;
    if (theDescriptions.count(lang))
        theDescription = theDescriptions[lang];
    else
        if (theDescriptions.count(defLang))
            theDescription = theDescriptions[defLang];
        else
            theDescription = theTag;
    if (!theUrl.isEmpty())
        url = theUrl.toString();

    QLabel* aValue = new QLabel();
    aValue->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    aValue->setWordWrap(true);
    aLayout->addWidget(aValue);

    if (theValues.count()) {
        QString aValueStr;
        if (theValues[0]->theDescriptions.count(lang))
            aValueStr = theValues[0]->theDescriptions[lang];
        else
            if (theValues[0]->theDescriptions.count(defLang))
                aValueStr = theValues[0]->theDescriptions[defLang];
            else
                aValueStr = theValues[0]->theTagValue;

        aValue->setText(QString("<b>%1</b>").arg(aValueStr));

        if (!theValues[0]->theUrl.isEmpty())
            url = theValues[0]->theUrl.toString();

        if (!url.isEmpty())
            aLabel->setText(QString("<a href=\"%1\">%2</a>").arg(url).arg(theDescription));
        else
            aLabel->setText(theDescription);
    } else
        aValue->setText(QString("<b>%1</b>").arg(F->tagValue(theTag, "")));

    theMainWidget = aValue;
    theLabelWidget = aLabel;

    return theWidget;
}

void TagTemplateWidgetConstant::apply(const Feature* F)
{
    if (!theValues.count()) {
        ((QLabel*)theMainWidget)->setText(QString("<b>%1</b>").arg(F->tagValue(theTag, "")));
        return;
    }
    bool Regexp = false;
    bool OK = true;
    QString oldVal = F->tagValue(theTag, "__NULL__");
    QString newVal = theValues[0]->theTagValue;
    QRegExp subst("\\$\\[(.+)\\]");
    subst.setMinimal(true);
    int pos = 0;
    while ((pos = subst.indexIn(newVal, pos)) != -1) {
        Regexp = true;
        QString rep = F->tagValue(subst.cap(1), "__NULL__");
        if (rep == "__NULL__") {
            OK = false;
            break;
        }
        newVal.replace(subst.cap(0), rep);
        pos += rep.length();
    }
    if (!Regexp || (Regexp && OK)) {
        ((QLabel*)theMainWidget)->setText(QString("<b>%1</b>").arg(newVal));
        if (newVal != oldVal) {
            emit tagChanged(theTag, newVal);
        }
    } else
        ((QLabel*)theMainWidget)->setText(QString("<b>%1</b>").arg(F->tagValue(theTag, "")));
}

TagTemplateWidgetConstant* TagTemplateWidgetConstant::fromXml(const QDomElement& e)
{
    TagTemplateWidgetConstant* aCW = new TagTemplateWidgetConstant();

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        aCW->parseCommonElements(t);
    }

    return aCW;
}

bool TagTemplateWidgetConstant::toXML(QDomElement& xParent, bool header)
{
    bool OK = true;

    if (header) return OK;

    QDomElement e = xParent.ownerDocument().createElement("widget");
    xParent.appendChild(e);

    if (!theId.isEmpty())
        e.setAttribute("id", theId);
    e.setAttribute("type", theType);
    e.setAttribute("tag", theTag);
    generateCommonElements(e);

    return OK;
}

/** TagTemplateWidgetEdit **/

QWidget* TagTemplateWidgetEdit::getWidget(const Feature* F, const MapView* V)
{
    if (theSelector && (theSelector->matches(F,V->pixelPerM()) != TagSelect_Match && theSelector->matches(F,V->pixelPerM()) != TagSelect_DefaultMatch))
        return NULL;

    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    QHBoxLayout* aLayout = new QHBoxLayout(theWidget);
    aLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* aLabel = new QLabel();
    aLabel->setOpenExternalLinks(true);
    aLabel->setWordWrap(true);
    aLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    aLabel->setMaximumWidth(55);
    aLayout->addWidget(aLabel);

    QString url;
    if (theDescriptions.count(lang))
        theDescription = theDescriptions[lang];
    else
        if (theDescriptions.count(defLang))
            theDescription = theDescriptions[defLang];
        else
            theDescription = theTag;
    if (!theUrl.isEmpty())
        url = theUrl.toString();

    if (!url.isEmpty())
        aLabel->setText(QString("<a href=\"%1\">%2</a>").arg(url).arg(theDescription));
    else
        aLabel->setText(theDescription);

    QLineEdit* aValue = new QLineEdit();
    aLayout->addWidget(aValue);

    QString val = F->tagValue(theTag, "__NULL__");
    if (val != "__NULL__")
        aValue->setText(val);

    connect(aValue,SIGNAL(editingFinished()),this, SLOT(on_editingFinished()));
    theMainWidget = aValue;
    theLabelWidget = aLabel;

    return theWidget;
}

void TagTemplateWidgetEdit::apply(const Feature*)
{
}

TagTemplateWidgetEdit* TagTemplateWidgetEdit::fromXml(const QDomElement& e)
{
    TagTemplateWidgetEdit* aCW = new TagTemplateWidgetEdit();

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        aCW->parseCommonElements(t);
    }

    return aCW;
}

bool TagTemplateWidgetEdit::toXML(QDomElement& xParent, bool header)
{
    bool OK = true;

    if (!header) {
        QDomElement e = xParent.ownerDocument().createElement("widgetref");
        xParent.appendChild(e);
        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        else
            e.setAttribute("id", theTag);
    } else {
        QDomElement e = xParent.ownerDocument().createElement("widget");
        xParent.appendChild(e);

        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        e.setAttribute("type", theType);
        generateCommonElements(e);
    }

    return OK;
}

void TagTemplateWidgetEdit::on_editingFinished()
{
    QLineEdit* W = dynamic_cast<QLineEdit*>(theMainWidget);

    if (W->text().isEmpty()) {
        emit tagCleared(theTag);
    } else {
        emit tagChanged(theTag, W->text());
    }
}

/** TagTemplateWidgetMemberList **/

QWidget* TagTemplateWidgetMemberList::getWidget(const Feature* F, const MapView* V)
{
    if (theSelector && (theSelector->matches(F,V->pixelPerM()) != TagSelect_Match && theSelector->matches(F,V->pixelPerM()) != TagSelect_DefaultMatch))
        return NULL;
    if (!CHECK_RELATION(F))
        return NULL;
    theRelation = STATIC_CAST_RELATION(const_cast<Feature*>(F));

    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    QVBoxLayout* aLayout = new QVBoxLayout(theWidget);
    aLayout->setContentsMargins(0, 0, 0, 0);

    QWidget* memberlist = new QWidget(theWidget);
    memberlist->setContentsMargins(0, 0, 0, 0);
    RelationUi.setupUi(memberlist);
    RelationUi.MembersView->setContextMenuPolicy(Qt::CustomContextMenu);
    RelationUi.MembersView->horizontalHeader()->setStretchLastSection(true);

    RelationMemberDelegate* delegate = new RelationMemberDelegate(RestrictedRoles, RestrictedTypes, RelationUi.MembersView);
    RelationUi.MembersView->setItemDelegate(delegate);

    connect(RelationUi.MembersView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_Member_customContextMenuRequested(const QPoint &)));
    connect(RelationUi.MembersView, SIGNAL(clicked(QModelIndex)), this, SLOT(on_Member_clicked(QModelIndex)));
    connect(RelationUi.RemoveMemberButton,SIGNAL(clicked()),this, SLOT(on_RemoveMemberButton_clicked()));
    connect(RelationUi.btMemberUp, SIGNAL(clicked()), SLOT(on_btMemberUp_clicked()));
    connect(RelationUi.btMemberDown, SIGNAL(clicked()), SLOT(on_btMemberDown_clicked()));
    RelationUi.MembersView->setModel(theRelation->referenceMemberModel());
    aLayout->addWidget(memberlist);

    centerAction = new QAction(NULL, theWidget);
    connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
    centerZoomAction = new QAction(NULL, theWidget);
    connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));
    selectAction = new QAction(NULL, theWidget);
    connect(selectAction, SIGNAL(triggered()), this, SLOT(on_Member_selected()));
    centerAction->setText(tr("Center map"));
    centerZoomAction->setText(tr("Center && Zoom map"));
    selectAction->setText(tr("Select member"));

    theMainWidget = memberlist;
    theLabelWidget = NULL;

    return theWidget;
}

void TagTemplateWidgetMemberList::on_RemoveMemberButton_clicked()
{
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedIndexes();
    QModelIndex index;

    foreach(index, indexes)
    {
        QModelIndex idx = index.sibling(index.row(),0);
        if (idx.row() >= theRelation->size())
            continue;
        QVariant Content(theRelation->referenceMemberModel()->data(idx,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                CommandList* L = new CommandList(MainWindow::tr("Remove member '%1' on %2").arg(F->description()).arg(theRelation->description()), theRelation);
                if (theRelation->find(F) < theRelation->size())
                    L->add(new RelationRemoveFeatureCommand(theRelation,F,CUR_DOCUMENT->getDirtyOrOriginLayer(theRelation->layer())));
                if (L->empty())
                    delete L;
                else
                {
                    CUR_DOCUMENT->addHistory(L);
                    CUR_MAINWINDOW->invalidateView();
                    return;
                }
            }
        }
    }
}

void TagTemplateWidgetMemberList::on_Member_customContextMenuRequested(const QPoint & pos)
{
    QModelIndex ix = RelationUi.MembersView->indexAt(pos);
    if (ix.row() >= theRelation->size())
        return;
    if (ix.isValid()) {
        QMenu menu(RelationUi.MembersView);
        menu.addAction(centerAction);
        menu.addAction(centerZoomAction);
        menu.addAction(selectAction);
        menu.exec(RelationUi.MembersView->mapToGlobal(pos));
    }
}

void TagTemplateWidgetMemberList::on_centerAction_triggered()
{
    CoordBox cb;
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedIndexes();
    QModelIndex index;

    foreach(index, indexes)
    {
        QModelIndex idx = index.sibling(index.row(),0);
        if (idx.row() >= theRelation->size())
            continue;
        QVariant Content(theRelation->referenceMemberModel()->data(idx,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                //setSelection(F);
                cb = F->boundingBox();
            }
        }
    }
    Coord c = cb.center();
    CUR_VIEW->setCenter(c, CUR_VIEW->rect());
    CUR_MAINWINDOW->setUpdatesEnabled(true);
    CUR_MAINWINDOW->invalidateView(false);
}

void TagTemplateWidgetMemberList::on_centerZoomAction_triggered()
{
    CoordBox cb;
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedIndexes();
    QModelIndex index;

    foreach(index, indexes)
    {
        QModelIndex idx = index.sibling(index.row(),0);
        if (idx.row() >= theRelation->size())
            continue;
        QVariant Content(theRelation->referenceMemberModel()->data(idx,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                //setSelection(F);
                cb = F->boundingBox();
                CoordBox mini(cb.center()-COORD_ENLARGE, cb.center()+COORD_ENLARGE);
                cb.merge(mini);
                cb = cb.zoomed(1.1);
            }
        }
    }
    CUR_VIEW->setViewport(cb, CUR_VIEW->rect());
    CUR_MAINWINDOW->setUpdatesEnabled(true);
    CUR_MAINWINDOW->invalidateView(false);
}

void TagTemplateWidgetMemberList::on_Member_clicked(const QModelIndex & index)
{
    QList<Feature*> Highlighted;

    if (index.row() >= theRelation->size())
        return;

    QVariant Content(theRelation->referenceMemberModel()->data(index,Qt::UserRole));
    if (Content.isValid())
    {
        Feature* F = Content.value<Feature*>();
        if (F)
            Highlighted.push_back(F);
    }
    if (!Highlighted.isEmpty())
        PROPERTIES_DOCK->setHighlighted(Highlighted);
    CUR_VIEW->update();
}

void TagTemplateWidgetMemberList::on_Member_selected()
{
    QList<Feature*> Features;
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedIndexes();
    QModelIndex index;

    foreach(index, indexes)
    {
        QModelIndex idx = index.sibling(index.row(),0);
        if (idx.row() >= theRelation->size())
            continue;
        QVariant Content(theRelation->referenceMemberModel()->data(idx,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                Features.append(F);
            }
        }
    }
    if (!Features.isEmpty())
        PROPERTIES_DOCK->setMultiSelection(Features);
    CUR_MAINWINDOW->invalidateView(false);
}

void TagTemplateWidgetMemberList::on_btMemberUp_clicked()
{
    CommandList* theList = new CommandList(MainWindow::tr("Reorder members in relation %1").arg(theRelation->id().numId), theRelation);

    QModelIndex index;
    foreach(index, RelationUi.MembersView->selectionModel()->selectedIndexes())
    {
        if (index.row() >= theRelation->size())
            continue;
        RelationUi.MembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedRows(0);
    QModelIndexList newSel;
    foreach(index, indexes)
    {
        QVariant Content(theRelation->referenceMemberModel()->data(index,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                int pos = theRelation->find(F);
                if (!pos)
                    break;
                QString role = theRelation->getRole(pos);
                theList->add(new RelationRemoveFeatureCommand(theRelation, pos, CUR_DOCUMENT->getDirtyOrOriginLayer(theRelation->layer())));
                theList->add(new RelationAddFeatureCommand(theRelation, role, F, pos-1, CUR_DOCUMENT->getDirtyOrOriginLayer(theRelation->layer())));
                newSel.append(RelationUi.MembersView->model()->index(pos-1, 0));
            }
        }
    }
    RelationUi.MembersView->selectionModel()->clearSelection();
    foreach(index, newSel)
    {
        RelationUi.MembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    if (theList->empty())
        delete theList;
    else
    {
        CUR_DOCUMENT->addHistory(theList);
        CUR_MAINWINDOW->invalidateView();
    }
}

void TagTemplateWidgetMemberList::on_btMemberDown_clicked()
{
    CommandList* theList = new CommandList(MainWindow::tr("Reorder members in relation %1").arg(theRelation->id().numId), theRelation);

    QModelIndex index;
    foreach(index, RelationUi.MembersView->selectionModel()->selectedIndexes())
    {
        if (index.row() >= theRelation->size())
            continue;
        RelationUi.MembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    QModelIndexList indexes = RelationUi.MembersView->selectionModel()->selectedRows(0);
    QModelIndexList newSel;
    // We need to iterate backwards so that earlier moves don't corrupt the inputs to later ones
    for (int i = indexes.count()-1;  i >= 0;  i--)
    {
        index = indexes[i];
        QVariant Content(theRelation->referenceMemberModel()->data(index,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F) {
                int pos = theRelation->find(F);
                if (pos >= theRelation->size()-1)
                    break;
                QString role = theRelation->getRole(pos);
                theList->add(new RelationRemoveFeatureCommand(theRelation, pos, CUR_DOCUMENT->getDirtyOrOriginLayer(theRelation->layer())));
                theList->add(new RelationAddFeatureCommand(theRelation, role, F, pos+1, CUR_DOCUMENT->getDirtyOrOriginLayer(theRelation->layer())));
                newSel.append(RelationUi.MembersView->model()->index(pos+1, 0));
            }
        }
    }
    RelationUi.MembersView->selectionModel()->clearSelection();
    foreach(index, newSel)
    {
        RelationUi.MembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    if (theList->empty())
        delete theList;
    else
    {
        CUR_DOCUMENT->addHistory(theList);
        CUR_MAINWINDOW->invalidateView();
    }
}

void TagTemplateWidgetMemberList::apply(const Feature*)
{
}

TagTemplateWidgetMemberList* TagTemplateWidgetMemberList::fromXml(const QDomElement& e)
{
    TagTemplateWidgetMemberList* aCW = new TagTemplateWidgetMemberList();

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        if (!aCW->parseCommonElements(t)) {
            if (t.tagName() == "restrictroles") {
                aCW->RestrictedRoles = t.firstChild().toText().nodeValue().split(',');
            } else if (t.tagName() == "restricttypes") {
                QStringList sl = t.firstChild().toText().nodeValue().split(',');
                foreach (QString s, sl)  {
                    if (s == "node")
                        aCW->RestrictedTypes << IFeature::Point;
                    else if (s == "way")
                        aCW->RestrictedTypes << IFeature::LineString;
                    else if (s == "area")
                        aCW->RestrictedTypes << IFeature::Polygon;
                    else if (s == "relation")
                        aCW->RestrictedTypes << IFeature::OsmRelation;
                }
            }
        }
    }

    return aCW;
}

bool TagTemplateWidgetMemberList::toXML(QDomElement& xParent, bool header)
{
    bool OK = true;

    if (!header) {
        QDomElement e = xParent.ownerDocument().createElement("widgetref");
        xParent.appendChild(e);
        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        else
            e.setAttribute("id", theTag);
    } else {
        QDomElement e = xParent.ownerDocument().createElement("widget");
        xParent.appendChild(e);

        if (!theId.isEmpty())
            e.setAttribute("id", theId);
        e.setAttribute("type", theType);
        generateCommonElements(e);
        if (RestrictedRoles.size()) {
            QDomElement c = xParent.ownerDocument().createElement("restrictroles");
            e.appendChild(c);
            QDomText v = c.ownerDocument().createTextNode(RestrictedRoles.join(","));
            c.appendChild(v);
        }
        if (RestrictedTypes.size()) {
            QDomElement c = xParent.ownerDocument().createElement("restricttypes");
            e.appendChild(c);
            QStringList sl;
            foreach(IFeature::FeatureType ft, RestrictedTypes) {
                switch (ft) {
                case IFeature::Point:
                    sl << "node";
                    break;

                case IFeature::LineString:
                    sl << "way";
                    break;

                case IFeature::Polygon:
                    sl << "area";
                    break;

                case IFeature::OsmRelation:
                    sl << "relation";
                    break;
                }
            }

            QDomText v = c.ownerDocument().createTextNode(sl.join(","));
            c.appendChild(v);
        }
    }

    return OK;
}


/** TagTemplate **/

TagTemplate::TagTemplate()
    : theWidget(0), theSelector(0)
{
}

TagTemplate::TagTemplate(QString aName)
    : theWidget(0), theSelector(0)
{
    theDescriptions[getDefaultLanguage()] = aName;
}

TagTemplate::TagTemplate(QString aName, QString aSelector)
    : theWidget(0)
{
    theDescriptions[getDefaultLanguage()] = aName;
    theSelector = TagSelector::parse(aSelector);
}

TagTemplate::TagTemplate(const TagTemplate& aTemplate)
    : QObject(), theSelector(aTemplate.theSelector)
{
}

TagTemplate::~TagTemplate()
{
    delete theSelector;
}

TagSelectorMatchResult TagTemplate::matchesTag(const Feature* F, const MapView* V)
{
    TagSelectorMatchResult res;

    if (!theSelector) return TagSelect_NoMatch;
    // Special casing for multipolygon roads
    if (const Way* R = dynamic_cast<const Way*>(F))
    {
        // TODO create a isPartOfMultiPolygon(R) function for this
        for (int i=0; i<R->sizeParents(); ++i)
        {
            if (const Relation* Parent = dynamic_cast<const Relation*>(R->getParent(i)))
                if (!Parent->isDeleted())
                    if (Parent->tagValue("type","") == "multipolygon")
                        return TagSelect_NoMatch;
        }
    }
    if ((res = theSelector->matches(F,V->pixelPerM())))
        return res;
    // Special casing for multipolygon relations
    if (const Relation* R = dynamic_cast<const Relation*>(F))
    {
        if (R->tagValue("type","") == "multipolygon") {
            for (int i=0; i<R->size(); ++i)
                if (!R->get(i)->isDeleted())
                    if ((res = theSelector->matches(R->get(i),V->pixelPerM())))
                        return res;
        }
    }
    return TagSelect_NoMatch;
}

QWidget* TagTemplate::getWidget(const Feature* F, const MapView* V)
{
    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    theWidget = new QWidget();
    //if (theDescriptions.count(lang))
    //	theWidget->setTitle(theDescriptions[lang]);
    //else
    //	theWidget->setTitle(theDescriptions[defLang]);
    QVBoxLayout* aLayout = new QVBoxLayout(theWidget);
    aLayout->setContentsMargins(2, 2, 2, 2);

    for (int i=0; i<theFields.size(); ++i) {
        QWidget* W = theFields[i]->getWidget(F,V);
        if (W)
            aLayout->addWidget(W);
    }

    return theWidget;
}

void TagTemplate::setSelector(const QString& anExpression)
{
    delete theSelector;
    theSelector = TagSelector::parse(anExpression);
}

void TagTemplate::setSelector(TagSelector* aSel)
{
    delete theSelector;
    theSelector = aSel;
}

void TagTemplate::apply(const Feature* F)
{
    for (int i=0; i<theFields.size(); ++i) {
        theFields[i]->apply(F);
    }
}

TagTemplate* TagTemplate::fromXml(const QDomElement& e, TagTemplates* templates)
{
    TagTemplate* aTemplate = new TagTemplate();

    if (e.tagName() != "template") {
        return NULL;
    }

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        if (t.tagName() == "description") {
            aTemplate->theDescriptions.insert(t.attribute("locale"), t.firstChild().toText().nodeValue());
        } else
            if (t.tagName() == "selector") {
                TagSelector* aSel = TagSelector::parse(t.attribute("expr"));
                if (!aSel)
                    return NULL;
                aTemplate->setSelector(aSel);
            } else
                if (t.tagName() == "widget") {
                    TagTemplateWidget* aTW = TagTemplateWidget::fromXml(t);
                    if (aTW) {
                        templates->addWidget(aTW);
                        aTemplate->theFields.append(aTW);
                        connect(aTW, SIGNAL(tagChanged(QString, QString)), aTemplate, SLOT(on_tag_changed(QString, QString)));
                        connect(aTW, SIGNAL(tagCleared(QString)), aTemplate, SLOT(on_tag_cleared(QString)));
                    }
                } else
                    if (t.tagName() == "widgetref") {
                        TagTemplateWidget* aTW = templates->findWidget(t.attribute("id"));
                        if (aTW) {
                            aTemplate->theFields.append(aTW);
                            connect(aTW, SIGNAL(tagChanged(QString, QString)), aTemplate, SLOT(on_tag_changed(QString, QString)));
                            connect(aTW, SIGNAL(tagCleared(QString)), aTemplate, SLOT(on_tag_cleared(QString)));
                        }
                    }
    }
    return aTemplate;
}

bool TagTemplate::toXML(QDomElement& xParent)
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement("template");
    xParent.appendChild(e);

    if (theSelector) {
        QDomElement s = xParent.ownerDocument().createElement("selector");
        e.appendChild(s);
        s.setAttribute("expr", theSelector->asExpression(false));
    }

    QHashIterator<QString, QString > itD(theDescriptions);
    while(itD.hasNext()) {
        itD.next();

        QDomElement d = xParent.ownerDocument().createElement("description");
        e.appendChild(d);

        d.setAttribute("locale", itD.key());

        QDomText v = xParent.ownerDocument().createTextNode(itD.value());
        d.appendChild(v);
    }

    for (int i=0; i<theFields.size(); ++i) {
        theFields[i]->toXML(e, false);
    }

    return OK;
}

void TagTemplate::on_tag_changed(QString k, QString v)
{
    emit tagChanged(k, v);
}

void TagTemplate::on_tag_cleared(QString k)
{
    emit tagCleared(k);
}

/** TagTemplates **/

TagTemplates::TagTemplates()
    : theWidget(0), theFeature(0), curTemplate(0)
{
}

TagTemplates::~TagTemplates()
{
    for (int i=0; i< widgets.size(); ++i)
        delete widgets[i];
    for (int i=0; i< items.size(); ++i)
        delete items[i];
}

QWidget* TagTemplates::getWidget(const Feature* F, const MapView* V)
{
    QString lang = getDefaultLanguage();
    QString defLang = "en";
    if (lang == "-" || !M_PREFS->getTranslateTags())
        lang = "en";

    if (curTemplate) {
        disconnect(curTemplate, 0, this, 0);
        curTemplate = NULL;
    }
    if (F != theFeature)
        forcedTemplate = NULL;
    else
        curTemplate = forcedTemplate;
    theFeature = F;

    theWidget = new QWidget();
    //if (theDescriptions.count(lang))
    //	theWidget->setTitle(theDescriptions[lang]);
    //else
    //	theWidget->setTitle(theDescriptions[defLang]);
    QVBoxLayout* aLayout = new QVBoxLayout(theWidget);
    aLayout->setContentsMargins(2, 2, 2, 2);

    QHBoxLayout* headLayout = new QHBoxLayout();
    headLayout->setContentsMargins(0, 0, 0, 0);
    aLayout->addLayout(headLayout);

    theCombo = new QComboBox();
    theCombo->setEditable(true);
    theCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    //	QFont f = theCombo->font();
    //	f.setPointSize(6);
    //	theCombo->setFont(f);
    //	theCombo->setMaximumHeight(16);
    theCombo->setMinimumWidth(100);
    theCombo->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    headLayout->addWidget(theCombo);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    headLayout->addWidget(line);

    int idx = -1;
    theCombo->addItem(tr("Undefined"), "__NULL__");
    for (int i=0; i<items.size(); ++i) {
        if (items[i]->theDescriptions.count(lang))
            theCombo->addItem(items[i]->theDescriptions[lang], qVariantFromValue(items[i]));
        else
            theCombo->addItem(items[i]->theDescriptions[defLang], qVariantFromValue(items[i]));

        if (forcedTemplate) {
            if (items[i] == forcedTemplate) {
                idx = i+1;
            }
        }
    }
    if (!forcedTemplate) {
        for (int i=0; i<items.size(); ++i) {
            if (idx == -1 && items[i]->matchesTag(F,V) == TagSelect_Match) {
                curTemplate = items[i];
                idx = i+1;
            }
        }
    }
    if (!forcedTemplate && idx == -1) {
        for (int i=0; i<items.size(); ++i) {
            if (idx == -1 && items[i]->matchesTag(F,V) == TagSelect_DefaultMatch) {
                curTemplate = items[i];
                idx = i+1;
            }
        }
    }

    if (idx == -1)
        theCombo->setCurrentIndex(0);
    else
        theCombo->setCurrentIndex(idx);

    if (curTemplate) {
        aLayout->addWidget(curTemplate->getWidget(F,V));
        connect(curTemplate, SIGNAL(tagChanged(QString, QString)), this, SLOT(on_tag_changed(QString, QString)));
        connect(curTemplate, SIGNAL(tagCleared(QString)), this, SLOT(on_tag_cleared(QString)));
    }
    connect(theCombo,SIGNAL(activated(int)), this, SLOT(on_combo_activated(int)));

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    aLayout->addWidget(line);

    return theWidget;
}

void TagTemplates::on_combo_activated(int idx)
{
    if (idx > items.count() -1) {
        TagTemplate* newTmpl = new TagTemplate(theCombo->currentText());
        items.append(newTmpl);

        if (curTemplate) {
            for (int i=0; i<curTemplate->theFields.count(); ++i) {
                curTemplate->theFields[i]->getCurrentValue();
            }
        }
    } else {
        forcedTemplate = theCombo->itemData(idx).value<TagTemplate*>();

        emit templateChanged(forcedTemplate);
    }
}

TagTemplates* TagTemplates::fromXml(const QDomElement& e)
{
    TagTemplates* aTemplates = new TagTemplates();

    if (e.tagName() != "templates") {
        return NULL;
    }

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        if (t.tagName() == "template") {
            TagTemplate* tmpl = TagTemplate::fromXml(t, aTemplates);
            if (tmpl)
                aTemplates->items.append(tmpl);
            else
                return NULL;
        } else
            if (t.tagName() == "widgets") {
                for(QDomNode n = t.firstChild(); !n.isNull(); n = n.nextSibling())
                {
                    QDomElement w = n.toElement();
                    if (w.tagName() == "widget") {
                        TagTemplateWidget* aTW = TagTemplateWidget::fromXml(w);
                        if (aTW)
                            aTemplates->addWidget(aTW);
                        else
                            return NULL;
                    }
                }
            }
    }
    return aTemplates;
}

bool TagTemplates::mergeXml(const QDomElement& e)
{
    if (e.tagName() != "templates") {
        return false;
    }

    for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        QDomElement t = n.toElement();
        if (t.isNull())
            continue;

        if (t.tagName() == "template") {
            TagTemplate* tmpl = TagTemplate::fromXml(t, this);
            if (tmpl)
                items.append(tmpl);
            else
                return false;
        } else
            if (t.tagName() == "widgets") {
                for(QDomNode n = t.firstChild(); !n.isNull(); n = n.nextSibling())
                {
                    QDomElement w = n.toElement();
                    if (w.tagName() == "widget") {
                        TagTemplateWidget* aTW = TagTemplateWidget::fromXml(w);
                        if (aTW)
                            addWidget(aTW);
                        else
                            return false;
                    }
                }
            }
    }
    return true;
}

TagTemplate* TagTemplates::match(const Feature* F, const MapView* V)
{
    for (int i=0; i<items.size(); ++i) {
        if (items[i]->matchesTag(F,V) == TagSelect_Match)
            return items[i];
    }
    for (int i=0; i<items.size(); ++i) {
        if (items[i]->matchesTag(F,V) == TagSelect_DefaultMatch)
            return items[i];
    }
    return NULL;
}

bool TagTemplates::toXML(QDomDocument& doc)
{
    bool OK = true;

    QDomElement e = doc.createElement("templates");
    doc.appendChild(e);

    QDomElement w = doc.createElement("widgets");
    e.appendChild(w);

    for (int i=0; i<widgets.size(); ++i)
        OK = widgets[i]->toXML(w, true);

    for (int i=0; i<items.size(); ++i)
        OK = items[i]->toXML(e);

    return OK;
}

TagTemplateWidget* TagTemplates::findWidget(const QString& id)
{
    for (int i=0; i<widgets.size(); ++i) {
        if (widgets[i]->id() == id)
            return widgets[i];
    }

    return NULL;
}

void TagTemplates::on_tag_changed(QString k, QString v)
{
    emit tagChanged(k, v);
}

void TagTemplates::on_tag_cleared(QString k)
{
    emit tagCleared(k);
}

void TagTemplates::addWidget(TagTemplateWidget* aWidget)
{
    widgets.append(aWidget);
}

void TagTemplates::apply(const Feature* F)
{
    if (curTemplate)
        curTemplate->apply(F);
}
