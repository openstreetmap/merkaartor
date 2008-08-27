#ifndef MDOCKANCESTOR_H
#define MDOCKANCESTOR_H


#ifndef _MOBILE

#include <QDockWidget>

class MDockAncestor : public QDockWidget
{
	public:
		MDockAncestor(QWidget *parent = 0) : QDockWidget(parent) {};
		void setWidget ( QWidget * widget ) { QDockWidget::setWidget(widget); };
		QWidget* getWidget();
		void setAllowedAreas ( Qt::DockWidgetAreas areas )  { QDockWidget::setAllowedAreas(areas); };

	protected:
		QWidget* mainWidget;
};

#else

#include <QDialog>

class QVBoxLayout;

class MDockAncestor : public QDialog
{
	public:
		MDockAncestor(QWidget *parent = 0);
		void setWidget ( QWidget * widget );
		QWidget* getWidget();
		void setAllowedAreas ( Qt::DockWidgetAreas  )  {  };
	
	protected:
		QVBoxLayout* theLayout;
		QWidget* mainWidget;
};

#endif

#endif //MDOCKANCESTOR_H
