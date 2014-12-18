///////////////////////////////////////////////////////////
//  CheckBoxList.h
//  Header of the Class CheckBoxList
//  Created on:      02-Jun-2008 6:53:56 PM
//  Original author: Nasser M. Al-Ansari (Da-Crystal)
///////////////////////////////////////////////////////////
//	Modification History:
//
//	Who(SNo)			Date			Description
//	
///////////////////////////////////////////////////////////

#ifndef CHECKBOXLIST_H
#define CHECKBOXLIST_H

#include <QtGui>
#include <QComboBox>

class CheckBoxList: public QComboBox
{
	Q_OBJECT;

public:
	CheckBoxList(QWidget *widget = 0);
	virtual ~CheckBoxList();
	bool eventFilter(QObject *object, QEvent *event);
	virtual void paintEvent(QPaintEvent *);
	void SetDisplayText(QString text);
	QString GetDisplayText() const;

private:
	QString m_DisplayText;
};

#endif // CHECKBOXLIST_H
