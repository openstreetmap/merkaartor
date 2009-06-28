#include "MDockAncestor.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

#ifdef _MOBILE

MDockAncestor::MDockAncestor(QWidget *parent)
	: QDialog(parent)
{
//	setWindowState(Qt::WindowMaximized);
	setWindowState(Qt::WindowFullScreen);
	theLayout = new QVBoxLayout(this);
	theLayout->setSpacing(4);
	theLayout->setMargin(4);

	QDialogButtonBox* aBB = new QDialogButtonBox(QDialogButtonBox::Close);
	theLayout->addWidget(aBB);
	connect(aBB, SIGNAL(rejected()), this, SLOT(hide()));
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
	mainWidget = new QWidget();
	mainWidget->setParent(this); 
	
#ifndef _MOBILE
	QDockWidget::setWidget(mainWidget); 
#else
	theLayout->addWidget(mainWidget);
#endif

	return mainWidget;
}

