#include "Utils/MDockAncestor.h"

#include <QVBoxLayout>
#include <QApplication>

#ifdef _MOBILE

MDockAncestor::MDockAncestor(QWidget *parent)
	: QDialog(parent)
{
	setWindowState(Qt::WindowMaximized);
	theLayout = new QVBoxLayout(this);
	theLayout->setSpacing(4);
	theLayout->setMargin(4);
}

void MDockAncestor::setWidget ( QWidget * widget )
{
	mainWidget = widget;
	mainWidget->setParent(this); 
	theLayout->addWidget(mainWidget);
}

#endif

QWidget* MDockAncestor::getWidget()
{
	mainWidget = new QWidget();
	mainWidget->setParent(this); 
	
#ifndef _MOBILE
	QDockWidget::setWidget(mainWidget); 
#else
	theLayout->addWidget(mainWidget);
#endif

	return mainWidget;
}

