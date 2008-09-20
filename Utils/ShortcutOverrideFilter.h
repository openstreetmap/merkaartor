#ifndef SHORTCUTOVERRIDEFILTER_H
#define SHORTCUTOVERRIDEFILTER_H

#include <QObject>
#include <QEvent>
#include <QList>
#include <QKeySequence>

/**
	@author Travers Carter <tcarter@noggin.com.au>
*/
class ShortcutOverrideFilter : public QObject
{
Q_OBJECT
public:
    ShortcutOverrideFilter();
    ~ShortcutOverrideFilter();

	void addOverride(const QString &key);
	void addOverride(int key);
	bool eventFilter(QObject* object, QEvent* event);


private:
	QList<QKeySequence> overrides;

};

#endif
