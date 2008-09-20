//
// C++ Implementation: ShortcutOverrideFilter
//
// Description: Provide an event filter usable via installEventFilter
//              that will veto specified application shortcut sequences
//              for the target allowing it to handle them as it normally
//              would if they weren't bound to a shortcut.
//
// Author: Travers Carter <tcarter@noggin.com.au>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//

#include "Utils/ShortcutOverrideFilter.h"

#include <QWidget>
#include <QEvent>
#include <QKeyEvent>


ShortcutOverrideFilter::ShortcutOverrideFilter() {
}

ShortcutOverrideFilter::~ShortcutOverrideFilter() {}

void ShortcutOverrideFilter::addOverride(const QString &key) {
	overrides.append(QKeySequence(key));
}

void ShortcutOverrideFilter::addOverride(int key) {
	overrides.append(QKeySequence(key));
}

bool ShortcutOverrideFilter::eventFilter(QObject* object, QEvent* event) {
	QWidget* widget = qobject_cast<QWidget*>(object);
	int i;
	if (!widget)
		return false;

	// If a key sequence is bound to a shortcut, Qt dispatches a ShortcutOverride
	// event event instead of a KeyPress, if it is handled, then the normal keypress
	// event will follow, otherwise the shortcut is activated
	if (event->type() == QEvent::ShortcutOverride) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		QKeySequence pressed(keyEvent->key() + keyEvent->modifiers());

		for (i=0; i<overrides.size(); i++) {
			if (pressed == overrides.at(i)) {
				event->accept();
				return true;
			}
		}
	}
	return false;
}

