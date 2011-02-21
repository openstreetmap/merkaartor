#include "MDockAncestor.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

#ifdef _MOBILE

MDockAncestor::MDockAncestor(QWidget *parent)
    : QWidget(parent), mainWidget(0)
{
    theLayout = new QVBoxLayout(this);
    theLayout->setSpacing(4);
    theLayout->setMargin(4);
}

void MDockAncestor::setWidget ( QWidget * widget )
{
    mainWidget = widget;
    mainWidget->setParent(this);
    theLayout->insertWidget(0, mainWidget);
}

#endif

QWidget* MDockAncestor::getWidget()
{
    if (!mainWidget) {
        mainWidget = new QWidget();
        mainWidget->setParent(this);

#ifndef _MOBILE
        QDockWidget::setWidget(mainWidget);
#else
        theLayout->addWidget(mainWidget);
#endif
    }

    return mainWidget;
}
