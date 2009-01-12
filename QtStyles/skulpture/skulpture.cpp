/*
 * skulpture.cpp - classical three-dimensional artwork
 *
 * Copyright 2007, 2008 Christoph Feck
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// ### This file is a mess

#include "skulpture_p.h"
#include <QtGui/QLayout>
#include <QtGui/QLCDNumber>
#include <QtGui/QPainter>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QScrollBar>
#include <QtGui/QIcon>
#include <QtGui/QStatusBar>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QScrollArea>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QWorkspace>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QPainterPath>
#include <QtGui/QGroupBox>
#include <QtGui/QDockWidget>
#include <QtGui/QToolButton>
#include <QtGui/QTextEdit>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtGui/QPlainTextEdit>
#endif
#include <QtGui/QComboBox>
#include <QtGui/QDial>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QCalendarWidget>
#include <QtGui/QToolBox>
#include <QtGui/QToolBar>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QDialog>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QKeyEvent>
#include <QtCore/QList>
#include <cstdio>
#include <QtCore/QDebug>

/*-----------------------------------------------------------------------*/

#include <QtGui/QStylePlugin>

class SkulptureStylePlugin : public QStylePlugin
{
	public:
		QStringList keys() const {
			return QString::fromUtf8("Skulpture").split(QChar(',', 0));
		}

		QStyle *create(const QString &key) {
			if (key.toLower() == QString::fromUtf8("skulpture")) {
				return new SkulptureStyle;
			}
			return 0;
		}
};


Q_EXPORT_PLUGIN2(skulpture, SkulptureStylePlugin)


/*-----------------------------------------------------------------------*/

SkulptureStyle::SkulptureStyle()
	: d(new Private)
{
	d->q = this;
}


SkulptureStyle::~SkulptureStyle()
{
	delete d;
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::polish(QApplication *application)
{
//	ParentStyle::polish(application);
//	return;
#if 0
	QString oldStyle = application->styleSheet();
	QString newStyle;
	QFile file(QString::fromUtf8("/home/pepo/Projekte/Qt/plugins/styles/SkulptureStyle.qss"));
	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		newStyle = stream.readAll();
	}
	application->setStyleSheet(newStyle + QChar('\n', 0) + oldStyle);
#endif
	application->installEventFilter(d->shortcut_handler);
	ParentStyle::polish(application);
	QPalette palette;
	polish(palette);
	application->setPalette(palette);
//	if (application->inherits("KApplication")) {
//		qDebug() << "KApplication is a" << application->metaObject()->className() << "(" << "object name:" << application->objectName() << ")";
//	}
}


void SkulptureStyle::unpolish(QApplication *application)
{
	ParentStyle::unpolish(application);
	application->removeEventFilter(d->shortcut_handler);
}


/*-----------------------------------------------------------------------*/

static inline bool is_frameless_popup(QWidget *widget)
{
	while (widget) {
		Qt::WindowFlags flags = widget->windowFlags();
		Qt::WindowType  type = Qt::WindowType(int(flags & Qt::WindowType_Mask));

		if (type & Qt::Window) {
			if (flags & Qt::FramelessWindowHint || type == Qt::Popup) {
				return true;
			}
			return false;
		}
		widget = widget->parentWidget();
	}
	return true;
}


static inline bool is_framed_scrollarea(QWidget *widget)
{
#if 1
	if (widget->inherits("Q3ScrollView")) {
		QFrame *frame = qobject_cast<QFrame *>(widget);
		if (frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken)) {
			return true;
		}
	}
	if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(widget)) {
		if (area->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken)) {
			int left, top, right, bottom;
			widget->getContentsMargins(&left, &top, &right, &bottom);
			if (!qobject_cast<QHeaderView *>(widget)
			 // ### parse runtime version for better comparison
			 && (QString::fromUtf8(qVersion()) >= QString::fromUtf8("4.4.") || !qobject_cast<QMdiArea *>(widget))
		//	 && !qobject_cast<QWorkspace *>(widget)
			 && !qobject_cast<QScrollArea *>(widget)
			 && !widget->inherits("KColorCells")
		//	 && !widget->inherits("QColumnView")
			 && !widget->inherits("DolphinColumnView")
			 && (!widget->inherits("KTextBrowser") || (left && top && right && bottom))
			 && !widget->inherits("KDEPrivate::KPageTabbedView")
		//	 && !widget->inherits("KFilePlacesView")
			 && !is_frameless_popup(widget)) {
				return true;
			 }
		}
	}
#endif
	return false;
}


static WidgetShadow *findShadow(QWidget *widget)
{
	QWidget *parent = widget->parentWidget();
	if (parent) {
		QList<WidgetShadow *> shadows = parent->findChildren<WidgetShadow *>();

		foreach (WidgetShadow *shadow, shadows) {
			if (shadow->widget() == widget) {
				return shadow;
			}
		}
	}
	return 0;
}


void SkulptureStyle::polish(QWidget *widget)
{
//	ParentStyle::polish(widget);
//	return;
#if 1
	//printf("polishing a \"%s\" (which is a \"%s\")\n", widget->metaObject()->className(), widget->metaObject()->superClass()->className());
#if 0
	QPalette palette = widget->palette();
	polish(palette);
	widget->setPalette(palette);
#endif
#if 1
	if (QMdiArea *area = qobject_cast<QMdiArea *>(widget)) {
		area->installEventFilter(d);
	}
	if (qobject_cast<QMdiSubWindow *>(widget)) {
		WidgetShadow *shadow = findShadow(widget);
		if (!shadow) {
			widget->installEventFilter(d);
			if (widget->parentWidget()) {
				WidgetShadow *shadow = new WidgetShadow(widget->parentWidget());
				shadow->setWidget(widget);
				shadow->updateZOrder();
				shadow->show();
			}
		}
	}
#endif
#if 1
	if (QLCDNumber *lcd = qobject_cast<QLCDNumber *>(widget)) {
		QPalette palette;
		palette.setColor(QPalette::Base, QColor(220, 230, 210));
		palette.setColor(QPalette::WindowText, QColor(60, 60, 60));
		lcd->setPalette(palette);
	//	lcd->installEventFilter(d);
	//	lcd->setContentsMargins(8, 8, 8, 8);
		lcd->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		lcd->setSegmentStyle(QLCDNumber::Flat);
	}
#endif
#if 0
	// FIXME Qt does not propagate scrollbars background role to ScrollBar*Page areas
	if (QScrollBar *bar = qobject_cast<QScrollBar *>(widget)) {
		if (bar->parentWidget()) {
			QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(bar->parentWidget());
			if (area && area->viewport()) {
				bar->setBackgroundRole(area->viewport()->backgroundRole());
			}
		}
	}
#endif
#if 1
	if (QDialog *dialog = qobject_cast<QDialog *>(widget)) {
		dialog->installEventFilter(d);
	}
#endif
#if 0
	if (QMainWindow *window = qobject_cast<QMainWindow *>(widget)) {
		window->setBackgroundRole(QPalette::Dark);
	}
	if (QDockWidget *dock = qobject_cast<QDockWidget *>(widget)) {
		dock->installEventFilter(d);
	}
	if (QStatusBar *bar = qobject_cast<QStatusBar *>(widget)) {
		bar->installEventFilter(d);
	}
#endif
	if (QToolBox *toolBox = qobject_cast<QToolBox *>(widget)) {
		toolBox->setBackgroundRole(QPalette::Window);
	//	toolBox->setContentsMargins(2, 2, 2, 2);
	//	toolBox->installEventFilter(d);
		toolBox->layout()->setSpacing(0);
	}
#if 0
	if (widget->inherits("KTitleWidget")) {
		QPalette palette = widget->palette();
		palette.setColor(QPalette::Base, palette.color(QPalette::Window));
		palette.setColor(QPalette::Text, palette.color(QPalette::WindowText));
		widget->setPalette(palette);
	}
#endif
	if (QFrame *frame = qobject_cast<QFrame *>(widget)) {
		switch (frame->frameShape()) {
			case QFrame::Panel:
			case QFrame::WinPanel:
			case QFrame::Box:
				frame->setFrameShape(QFrame::StyledPanel);
				break;
			case QFrame::HLine:
			case QFrame::VLine:
				frame->setEnabled(false);
				break;
			default:
				break;
		}
#if 1
		if (widget->inherits("SidebarTreeView")) {
			QPalette palette = widget->palette();
			palette.setColor(QPalette::Window, palette.color(QPalette::Base));
			((QAbstractScrollArea *) widget)->viewport()->setPalette(palette);
		//	printf("frame style is 0x%08x\n", ((QFrame *) widget)->frameStyle());
			((QFrame *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		}
#endif
#if 1
		if (widget->inherits("KFilePlacesView")) {
			QPalette palette = widget->palette();
			palette.setColor(QPalette::Window, palette.color(QPalette::Base));
			((QAbstractScrollArea *) widget)->viewport()->setPalette(palette);
		//	widget->setAutoFillBackground(true);
			((QFrame *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		}
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
		if (widget->inherits("QPlainTextEdit")) {
			QPlainTextEdit *edit = static_cast<QPlainTextEdit *>(widget);
		//	QPalette palette = edit->palette();
		//	palette.setColor(QPalette::Window, QColor(245, 245, 245));
		//	edit->setPalette(palette);
		//	edit->viewport()->setPalette(palette);
			edit->setBackgroundVisible(false);
			edit->viewport()->installEventFilter(d);
//			widget->setAttribute(Qt::WA_Hover, true);
		}
#endif
#if 1
		if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
			if (!qstrcmp(widget->metaObject()->className(), "SampleEdit")) {
				QWidget *bg = new QWidget(widget);
				bg->lower();
				bg->setObjectName(QString::fromUtf8("sample_background"));
				bg->setGeometry(2, 2, widget->width() - 4, widget->height() - 4);
				bg->setAutoFillBackground(true);
				bg->show();
			} else {
				d->mapper.setMapping(edit, edit);
				connect(edit, SIGNAL(textChanged()), &d->mapper, SLOT(map()));
				connect(&d->mapper, SIGNAL(mapped(QWidget *)), d, SLOT(textEditSourceChanged(QWidget *)));
				d->updateTextEditMargins(edit);
			}
			edit->viewport()->installEventFilter(d);
//			widget->setAttribute(Qt::WA_Hover, true);
			edit->setTabChangesFocus(true);
#if 0
			if (QTextBrowser *browser = qobject_cast<QTextBrowser *>(widget)) {
				connect(browser, SIGNAL(sourceChanged()), &d->mapper, SLOT(map()));
			}
#endif
		}
#endif
	}
#if 1
	if (QComboBox *combo = qobject_cast<QComboBox *>(widget)) {
		if (!combo->isEditable()) {
			combo->setBackgroundRole(QPalette::Button);
			combo->setForegroundRole(QPalette::ButtonText);
		}
	}
#endif
	if (qobject_cast<QCheckBox *>(widget)
	 || qobject_cast<QRadioButton *>(widget)) {
		widget->setBackgroundRole(QPalette::Window);
		widget->setForegroundRole(QPalette::WindowText);
	}
	if (qobject_cast<QScrollBar *>(widget)
//	 || qobject_cast<QLineEdit *>(widget)
	 || qobject_cast<QDial *>(widget)
	 || qobject_cast<QHeaderView*>(widget)
	 || qobject_cast<QSlider *>(widget)
	 || qobject_cast<QToolButton *>(widget)) {
		widget->setAttribute(Qt::WA_Hover, true);
	}
#if 0
	if (d->allowScrollBarSliderToCoverArrows && qobject_cast<QScrollBar *>(widget)) {
		widget->installEventFilter(d);
	}
#endif
	if (QProgressBar *pbar = qobject_cast<QProgressBar *>(widget)) {
		pbar->installEventFilter(d);
		if (pbar->isVisible() && !widget->inherits("StatusBarSpaceInfo")) {
			d->setAnimated(pbar, true);
		}
		return; // do not call ParentStyle, because QPlastiqueStyle may add its own animation timer.
	}
#if 1
	if (QToolBar *toolbar = qobject_cast<QToolBar *>(widget)) {
		QFont font;
		font.setPointSizeF(font.pointSizeF() / (1.19));
		QList<QToolButton *> children = toolbar->findChildren<QToolButton *>();
		foreach (QToolButton *child, children) {
			if (!child->icon().isNull()) {
				child->setFont(font);
			}
		}
		connect(toolbar, SIGNAL(orientationChanged(Qt::Orientation)), d, SLOT(updateToolBarOrientation(Qt::Orientation)));
	}
#endif
#if 0
	// FIXME does not work
	if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
#if 1
		QFont font;
		QFont oldfont;
		oldfont.setPointSizeF(oldfont.pointSizeF() * 1.0001);
		font.setPointSizeF(font.pointSizeF() / (1.19 /* * 1.19*/));
		font.setBold(true);
		menu->setFont(font);
	/*	QAction *action = menu->menuAction();
		action->setFont(oldfont);
		QList<QAction *> children = action->findChildren<QAction *>();
		foreach (QAction *child, children) {
			child->setFont(oldfont);
		}*/
#else
		menu->setStyleSheet(QString::fromUtf8("font-size: 6.5")/*.arg(menu->font().pointSizeF() / (1.19 * 1.19))*/);
#endif
	}
#endif
#if 0
	// FIXME does not work
	if (QGroupBox *group = qobject_cast<QGroupBox *>(widget)) {
		QFont oldfont;
#if 0
		if (group->testAttribute(Qt::WA_SetFont)) {
			QFont oldfont = group->fontInfo();
		}
#endif
		QFont font = oldfont;
		font.setPointSizeF(font.pointSizeF() * 1.19);
		font.setBold(true);
		group->setFont(font);
		QList<QWidget *> children = group->findChildren<QWidget *>();
		foreach (QWidget *child, children) {
			if (1 || !(child->testAttribute(Qt::WA_SetFont))) {
				printf("reset\n");
				child->setFont(oldfont);
			}
		}
	}
#endif
#if 1
	if (widget->inherits("Q3Header")) {
		QFont font;
		font.setPointSizeF(font.pointSizeF() / (1.19 /* 1.19*/));
		font.setBold(true);
		widget->setFont(font);
	}
#endif
	if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(widget)) {
		if (QAbstractItemView *iv = qobject_cast<QAbstractItemView *>(widget)) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
			// ### Qt issue
		//	iv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		//	iv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
		//	QApplication::setWheelScrollLines(64);
			iv = iv;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
			iv->viewport()->setAttribute(Qt::WA_Hover);
#endif

#if 1
			if (QHeaderView *header = qobject_cast<QHeaderView *>(widget)) {
				QFont font;
				font.setPointSizeF(font.pointSizeF() / (1.19 /* 1.19*/));
				font.setBold(true);
				header->setFont(font);
				// FIXME workaround for Qt 4.3
				header->headerDataChanged(header->orientation(), 0, 0);
				header->updateGeometry();
			}
#endif
		}
		if (is_framed_scrollarea(area)) {
			if (true /*|| !widget->inherits("KTextBrowser") || !widget->parentWidget() || !widget->parentWidget()->parentWidget() || !(widget->parentWidget()->parentWidget()->inherits("KTipDialog"))*/) {
			//	if (!widget->inherits("KHTMLView")
			//	&& !widget->inherits("DolphinIconsView")) {
					d->installFrameShadow(area);
			//	}
			}
		}
	}
#if 1
	if (widget->inherits("Konsole::TerminalDisplay")
	 || widget->inherits("KTextEditor::View")
	 || widget->inherits("KHTMLView")) {
	//	((QFrame *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		d->installFrameShadow(widget);
	}
#endif
	if (widget->inherits("Q3ScrollView")) {
		if (is_framed_scrollarea(widget)) {
			d->installFrameShadow(widget);
		}
	}
#endif
#if 0
	if (QTabWidget *tab = qobject_cast<QTabWidget *>(widget)) {
		if (QToolButton *button = qobject_cast<QToolButton *>(tab->cornerWidget(Qt::TopRightCorner))) {
			button->setAutoRaise(true);
		}
		if (QToolButton *button = qobject_cast<QToolButton *>(tab->cornerWidget(Qt::TopLeftCorner))) {
			button->setAutoRaise(true);
		}
	}
#endif
#if 1
	if (QToolButton *button = qobject_cast<QToolButton *>(widget)) {
		if (qobject_cast<QTabWidget *>(button->parentWidget())) {
			button->setAutoRaise(true);
		}
	}
#endif
	if (!qstrcmp(widget->metaObject()->className(), "QToolBoxButton")) {
		widget->setAttribute(Qt::WA_Hover, true);
	}
	if (!qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
		widget->installEventFilter(d);
		widget->setAttribute(Qt::WA_Hover, true);
	}
	ParentStyle::polish(widget);
}


void SkulptureStyle::unpolish(QWidget *widget)
{
	ParentStyle::unpolish(widget);
//	return;
	if (qobject_cast<QMdiArea *>(widget)) {
		widget->removeEventFilter(d);
	}
	if (QMdiSubWindow *win = qobject_cast<QMdiSubWindow *>(widget)) {
		win->removeEventFilter(d);
		WidgetShadow *shadow = findShadow(win);
		if (shadow) {
			shadow->hide();
			shadow->setParent(0);
			shadow->deleteLater();
		}
	}
#if 1
	if (QDialog *dialog = qobject_cast<QDialog *>(widget)) {
		dialog->removeEventFilter(d);
	}
#endif
#if 0
	if (QLCDNumber *lcd = qobject_cast<QLCDNumber *>(widget)) {
		lcd->removeEventFilter(d);
	}
	if (QToolBox *toolBox = qobject_cast<QToolBox *>(widget)) {
		toolBox->removeEventFilter(d);
	}
	if (QDockWidget *dock = qobject_cast<QDockWidget *>(widget)) {
		dock->removeEventFilter(d);
	}
	if (QStatusBar *status = qobject_cast<QStatusBar *>(widget)) {
		status->removeEventFilter(d);
	}
#endif
#if 0
	if (/*d->allowScrollBarSliderToCoverArrows &&*/ qobject_cast<QScrollBar *>(widget)) {
		widget->installEventFilter(d);
	}
#endif
	if (QProgressBar *pbar = qobject_cast<QProgressBar *>(widget)) {
		pbar->removeEventFilter(d);
		d->setAnimated(pbar, false);
		return;
	}
	if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(widget)) {
		area->removeEventFilter(d);
		if (/*QAbstractItemView *iv =*/qobject_cast<QAbstractItemView *>(widget)) {
#if 1
			if (QHeaderView *header = qobject_cast<QHeaderView *>(widget)) {
				QFont font;
			//	font.setPointSizeF(font.pointSizeF() / (1.19 * 1.19));
			//	font.setBold(true);
				header->setFont(font);
				// FIXME workaround for Qt 4.3
				header->headerDataChanged(header->orientation(), 0, 0);
				header->updateGeometry();
			}
#endif
		}
	/*	if (QMdiArea *area = qobject_cast<QMdiArea *>(widget)) {
			area->viewport()->removeEventFilter(d);
		}
	*/
		d->removeFrameShadow(area);
	}
#if 1
	if (widget->inherits("Konsole::TerminalDisplay")
	 || widget->inherits("KTextEditor::View")
	 || widget->inherits("KHTMLView")) {
		widget->removeEventFilter(d);
		d->removeFrameShadow(widget);
	}
#endif
	if (widget->inherits("Q3ScrollView")) {
		widget->removeEventFilter(d);
		d->removeFrameShadow(widget);
	}
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
	if (widget->inherits("QPlainTextEdit")) {
		QPlainTextEdit *edit = static_cast<QPlainTextEdit *>(widget);
		edit->viewport()->removeEventFilter(d);
	}
#endif
	if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
		if (!qstrcmp(widget->metaObject()->className(), "SampleEdit")) {
			QList<QObject *> children = widget->children();
			foreach (QObject *child, children) {
				if (child->objectName() == QString::fromUtf8("sample_background")) {
					child->setParent(0);
					child->deleteLater();
				}
			}
		} else {
			d->mapper.removeMappings(edit);
		}
		edit->viewport()->removeEventFilter(d);
	}
	if (QToolBar *toolbar = qobject_cast<QToolBar *>(widget)) {
		QFont font;
	//	font.setPointSizeF(font.pointSizeF() / (1.19));
		QList<QToolButton *> children = toolbar->findChildren<QToolButton *>();
		foreach (QToolButton *child, children) {
			if (!child->icon().isNull()) {
				child->setFont(font);
			}
		}
		disconnect(toolbar, SIGNAL(orientationChanged(Qt::Orientation)), d, SLOT(updateToolBarOrientation(Qt::Orientation)));
	}
	if (!qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
		widget->removeEventFilter(d);
	}
}


/*-----------------------------------------------------------------------*/

bool SkulptureStyle::Private::eventFilter(QObject *watched, QEvent *event)
{
#if 0
	// can't happen, because widgets are the only ones to install it
	if (!watched->isWidgetType()) {
		return QObject::eventFilter(watched, event);
	}
#endif
	QWidget *widget = reinterpret_cast<QWidget *>(watched);
//	printf("handling a \"%s\" (which is a \"%s\")\n", widget->metaObject()->className(), widget->metaObject()->superClass()->className());
	if (qobject_cast<QMdiSubWindow *>(widget)) {
		WidgetShadow *shadow = findShadow(widget);
		switch (event->type()) {
			case QEvent::Move:
			case QEvent::Resize:
				if (shadow) {
					shadow->updateGeometry();
				}
				break;
			case QEvent::ZOrderChange:
				if (shadow) {
					shadow->updateZOrder();
				}
				break;
			case QEvent::Hide:
				if (shadow) {
					shadow->setParent(0);
					shadow->hide();
					shadow->deleteLater();
				}
				break;
			case QEvent::Show:
				if (!shadow) {
					if (widget->parentWidget()) {
						shadow = new WidgetShadow(widget->parentWidget());
						shadow->setWidget(widget);
						shadow->updateZOrder();
					}
				} else {
					shadow->updateZOrder();
				}
			default:
				break;
		}
	}
	switch (event->type()) {
		case QEvent::Paint:
#if 1 // highlight current line in QTextEdit / QPlainTextEdit
			if (widget->objectName() == QLatin1String("qt_scrollarea_viewport")) {
				if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget->parent())) {
					if (!qstrcmp(edit->metaObject()->className(), "SampleEdit")) {
						QList<QObject *> children = edit->children();
						foreach (QObject *child, children) {
							if (child->objectName() == QString::fromUtf8("sample_background")) {
								QWidget *bg = qobject_cast<QWidget *>(child);
								if (bg) {
									QPalette palette = edit->palette();
									palette.setColor(QPalette::Window, palette.color(QPalette::Base));
									bg->setPalette(palette);
								}
							}
						}
					}
				//	updateTextEditMargins(edit);
					highlightCurrentEditLine(edit);
				}
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
				else if (widget->parent()->inherits("QPlainTextEdit")) {
					highlightCurrentEditLine(static_cast<QPlainTextEdit *>(widget->parent()));
				}
#endif
			}
#endif
#if 0
			if (QDialog *dialog = qobject_cast<QDialog *>(widget)) {
				QPainter painter(dialog);
				QRect r = dialog->rect();
				QLinearGradient dialogGradient1(r.topLeft(), r.bottomRight());
				dialogGradient1.setColorAt(0.0, QColor(255, 255, 255, 30));
				dialogGradient1.setColorAt(1.0, QColor(0, 0, 0, 10));
			//	painter.fillRect(r, dialogGradient1);

				QRadialGradient dialogGradient2(r.left() + r.width() / 2, r.top(), r.height());
				dialogGradient2.setColorAt(0.0, QColor(255, 255, 225, 160));
				dialogGradient2.setColorAt(1.0, QColor(0, 0, 0, 0));
			//	painter.fillRect(r, dialogGradient2);

				QLinearGradient dialogGradient3(r.topLeft(), r.bottomLeft());
				dialogGradient3.setColorAt(0.0, QColor(255, 255, 255, 30));
				dialogGradient3.setColorAt(1.0, QColor(0, 0, 0, 20));
				painter.fillRect(r, dialogGradient3);

				paintThinFrame(&painter, dialog->rect().adjusted(0, 0, 0, 0), dialog->palette(), 60, -20);
				paintThinFrame(&painter, dialog->rect().adjusted(1, 1, -1, -1), dialog->palette(), -20, 60);
			}
#endif
#if 0
			if (QStatusBar *status = qobject_cast<QStatusBar *>(widget)) {
				QPainter painter(status);
				paintThinFrame(&painter, status->rect(), status->palette(), -20, 60);
			}
			if (QToolBox *toolBox = qobject_cast<QToolBox *>(widget)) {
				QPainter painter(toolBox);
				paintThinFrame(&painter, toolBox->rect(), toolBox->palette(), 60, -20);
				paintThinFrame(&painter, toolBox->rect().adjusted(1, 1, -1, -1), toolBox->palette(), -60, 140);
			}
			if (QLCDNumber *lcd = qobject_cast<QLCDNumber *>(watched)) {
				// TODO nicer digits, antialiased, slight italics
			}
			if (QDockWidget *dock = qobject_cast<QDockWidget *>(widget)) {
				// ### rendering a frame around dock widgets does not work, because
				// the subwidgets are placed at the edges.
			}
#endif
			if (!qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
				QPainter painter(widget);
				QStyleOption option;
				option.initFrom(widget);
				if (option.state & QStyle::State_Enabled && option.state & QStyle::State_MouseOver) {
				//	painter.fillRect(widget->rect(), Qt::red);
				} else {
					painter.setOpacity(0.2);
				}
				QRect r = QRect(widget->rect().center() - QPoint(4, 4), QSize(8, 8));
				painter.drawPixmap(r, q->standardIcon(QStyle::SP_TitleBarCloseButton, &option, widget).pixmap(8, 8));
				event->accept();
				return true;
			}
			break;
		case QEvent::Show:
			if (QProgressBar *pbar = qobject_cast<QProgressBar *>(watched)) {
				if (!widget->inherits("StatusBarSpaceInfo")) {
					setAnimated(pbar, true);
				}
			}
			/* fall through */
		case QEvent::Move:
		case QEvent::Resize:
			if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
				if (!qstrcmp(widget->metaObject()->className(), "SampleEdit")) {
					QList<QObject *> children = widget->children();
					foreach (QObject *child, children) {
						if (child->objectName() == QString::fromUtf8("sample_background")) {
							QWidget *bg = qobject_cast<QWidget *>(child);
							if (bg) {
								bg->setGeometry(2, 2, widget->width() - 4, widget->height() - 4);
							}
						}
					}
				} else {
					textEditSourceChanged(edit);
				}
			}
			else if (qobject_cast<QMdiArea *>(widget)) {
				QList<WidgetShadow *> shadows = widget->findChildren<WidgetShadow *>();
				foreach (WidgetShadow *shadow, shadows) {
					shadow->updateGeometry();
				}
			}
			if (qobject_cast<QAbstractScrollArea *>(widget)
			 || widget->inherits("Q3ScrollView")
#if 1
			 || widget->inherits("Konsole::TerminalDisplay")
			 || widget->inherits("KTextEditor::View")
			 || widget->inherits("KHTMLView")
#endif
			   ) {
				updateFrameShadow(widget);
			}
			break;
		case QEvent::Destroy:
		case QEvent::Hide:
			setAnimated(reinterpret_cast<QProgressBar *>(watched), false);
			break;
#if 0
		case QEvent::MouseButtonRelease:
			if (allowScrollBarSliderToCoverArrows && qobject_cast<QScrollBar *>(widget)) {
				widget->update();
			}
			break;
#endif
		default:
			break;
	}
	return QObject::eventFilter(watched, event);
}


/*-----------------------------------------------------------------------*/

QRect SkulptureStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
//	return ParentStyle::subElementRect(element, option, widget);
	switch (element) {
		case SE_ToolBoxTabContents:
			return ParentStyle::subElementRect(element, option, widget).adjusted(11, 0, 0, 0);
		case SE_TabWidgetLeftCorner:
		case SE_TabWidgetRightCorner:
			return ParentStyle::subElementRect(element, option, widget).adjusted(1, 1, -1, 1);
		case SE_FrameContents:
			return option->rect.adjusted(2, 2, -2, -2);
#if 1
		case SE_CheckBoxIndicator:
		case SE_CheckBoxContents:
		case SE_CheckBoxFocusRect:
		case SE_CheckBoxClickRect:
		case SE_CheckBoxLayoutItem:
		case SE_RadioButtonIndicator:
		case SE_RadioButtonContents:
		case SE_RadioButtonFocusRect:
		case SE_RadioButtonClickRect:
		case SE_RadioButtonLayoutItem:
			// FIXME: only to work around QPlastiqueStyle
			return QWindowsStyle::subElementRect(element, option, widget);
#endif
		case SE_LineEditContents:
			return option->rect.adjusted(3, 0, -3, 0);
		case SE_DockWidgetCloseButton:
		case SE_DockWidgetFloatButton: {
				bool floating = false;
				bool vertical = false;
				if (option->type == QStyleOption::SO_DockWidget) {
					if (widget) {
						QStyleOptionDockWidget *dockOptions = (QStyleOptionDockWidget *) option;
						if (dockOptions->floatable) {
							const QDockWidget *dock = qobject_cast<const QDockWidget *>(widget);
							if (dock) {
								floating = dock->isFloating();
								vertical = dock->features() & QDockWidget::DockWidgetVerticalTitleBar;
							}
						}
					}
				}
				QRect r = ParentStyle::subElementRect(element, option, widget);
				if (!vertical) {
					if (floating) {
						if (option->direction == Qt::LeftToRight) {
							return r.adjusted(-6, 0, -6, 0);
						} else {
							return r.adjusted(6, 0, 6, 0);
						}
					} else {
						if (option->direction == Qt::LeftToRight) {
							return r.adjusted(-3, 1, -3, 1);
						} else {
							return r.adjusted(3, 1, 3, 1);
						}
					}
				} else {
					if (floating) {
						return r.adjusted(0, 6, 0, 6);
					} else {
						return r.adjusted(1, 3, 1, 3);
					}
				}
		}
		case SE_DockWidgetTitleBarText:
			return ParentStyle::subElementRect(element, option, widget).adjusted(4, -3, -4, 5);
		case SE_DockWidgetIcon:
			return ParentStyle::subElementRect(element, option, widget).adjusted(4, -3, 4, 5);
		default:
			break;
	}
	return ParentStyle::subElementRect(element, option, widget);
}


/*-----------------------------------------------------------------------*/

extern QSize sizeFromContentsMenuItem(const QStyleOptionMenuItem *option, const QSize &contentsSize);
extern QSize sizeFromContentsMenu(const QStyleOption *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style);
extern QSize sizeFromContentsToolButton(const QStyleOptionToolButton *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style);

QSize SkulptureStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
//	printf("SizeFromContents %d ... ", type);
//	QSize result = ParentStyle::sizeFromContents(type, option, contentsSize, widget);
//	printf("SizeFromContents OK\n");
//	return result;
#define FF -1
	switch (type) {
		case CT_Menu:
			return sizeFromContentsMenu(option, contentsSize, widget, this);
#if 1
		// FIXME: only to work around QPlastiqueStyle
		case CT_CheckBox:
		case CT_RadioButton: {
				QSize size = QWindowsStyle::sizeFromContents(type, option, contentsSize, widget);
				size.rheight() -= 1;
				return size;
			}
#endif
		case CT_SpinBox:
			return QWindowsStyle::sizeFromContents(type, option, contentsSize, widget) + QSize(5, -1 - FF);
		case CT_ToolButton: {
				const QStyleOptionToolButton *buttonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option);
				if (buttonOption) {
					return sizeFromContentsToolButton(buttonOption, contentsSize, widget, this);
				}
			}
			return contentsSize;
		case CT_PushButton: {
				QSize size = QCommonStyle::sizeFromContents(type, option, contentsSize, widget) + QSize(8, 0 - FF);
				int width = size.width();
				const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option);
				/* make text buttons same size */
				if (buttonOption && !buttonOption->text.isEmpty()) {
					if (width < 64) {
						width = 64;
					} else {
						width = ((width - 33) / 32) * 32 + 64;
					}
				}
				size.setWidth(width);
				return size;
			}
		case CT_TabBarTab: {
				const QStyleOptionTab *tabOption = qstyleoption_cast<const QStyleOptionTab *>(option);
				if (tabOption && int(tabOption->shape) & 2) {
					return contentsSize + QSize(8, 24);
				}
			}
			return contentsSize + QSize(24, 8);
		case CT_LineEdit:
			return contentsSize + QSize(8, 6 - FF);
//		case CT_TabWidget:
//			return contentsSize + QSize(8, 8);
		case CT_MenuBarItem: {
				QSize size = QWindowsStyle::sizeFromContents(type, option, contentsSize, widget);
				return size + QSize(0, 1);
			}
			break;
#if 1
		case CT_MenuItem: {
				const QStyleOptionMenuItem *menuOption = qstyleoption_cast<const QStyleOptionMenuItem *>(option);
				if (menuOption) {
					return sizeFromContentsMenuItem(menuOption, contentsSize);
				}
			}
			break;
#endif
		case CT_Slider: {
				QSize size = QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
			//	size.rheight() -= 1;
				return size;
#if 0
				const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option);
				if (sliderOption) {
					QSize size = contentsSize;
					// FIXME: Qt has a fixed tickmarks size... disable ticks for now
					if (sliderOption->orientation == Qt::Horizontal) {
						if (sliderOption->tickPosition & QSlider::TicksAbove) {
							printf("ha\n");
							size.rheight() -= 5;
						}
						if (sliderOption->tickPosition & QSlider::TicksBelow) {
							printf("hb\n");
							size.rheight() -= 5;
						}
					} else {
						if (sliderOption->tickPosition & QSlider::TicksAbove) {
							printf("va\n");
							size.rwidth() -= 5;
						}
						if (sliderOption->tickPosition & QSlider::TicksBelow) {
							printf("vb\n");
							size.rwidth() -= 5;
						}
					}
					return size;
				}
#endif
			}
			break;
		case CT_ComboBox:
		/*	if (widget && widget->inherits("QComboBox")) {
				if (widget->parentWidget() && qobject_cast<QToolBar *>(widget->parentWidget())) {
					return contentsSize + QSize(34, 4);
				}
			}
		*/	return contentsSize + QSize(34, 6 - FF);
		default:
			break;
	}
	return ParentStyle::sizeFromContents(type, option, contentsSize, widget);
}


/*-----------------------------------------------------------------------*/

extern QRect subControlRectScrollBar(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style);
extern QRect subControlRectSlider(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style);
extern QRect subControlRectToolButton(const QStyleOptionToolButton *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style);

QRect SkulptureStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
//	printf("SubControlRect %d ... ", control);
//	QRect result = ParentStyle::subControlRect(control, option, subControl, widget);
//	printf("SubControlRect OK\n");
//	return result;
	switch (control) {
#if 1
		case CC_Slider:
			if (const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
				return subControlRectSlider(sliderOption, subControl, widget, 0, 0, this);
			}
			break;
#else
		case CC_Slider: {
				QRect r = QCommonStyle::subControlRect(control, option, subControl, widget);
				return r;
			}
			break;
#endif
		case CC_TitleBar: {
				QRect r = QCommonStyle::subControlRect(control, option, subControl, widget);
				if (subControl != QStyle::SC_TitleBarSysMenu) {
					return r.adjusted(-2, -2, -3, -3);
				} else {
					return r.adjusted(0, -1, 0, -1);
				}
			}
			break;
		case CC_ScrollBar:
			if (d->allowScrollBarSliderToCoverArrows) {
				if (const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
					return subControlRectScrollBar(sliderOption, subControl, widget, 0, 0, this);
				}
			} else {
				return QCommonStyle::subControlRect(QStyle::CC_ScrollBar, option, subControl, widget);
			}
			break;
		case CC_ToolButton:
			if (const QStyleOptionToolButton *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
				return subControlRectToolButton(toolButtonOption, subControl, widget, 0, 0, this);
			}
			break;
		case CC_SpinBox: {
				// FIXME do not depend on plastique style
				QRect r = QPlastiqueStyle::subControlRect(control, option, subControl, widget);
				switch (subControl) {
					case SC_SpinBoxEditField:
						if (QString::fromUtf8(qVersion()) == QString::fromUtf8("4.3.0")) {
						//	printf("on 4.3.0\n");
							return r;
						}
					//	printf("on 4.3.x\n");
						return r.adjusted(-2, -2, 2, 2);
					default:
						break;
				}
				return r;
			}
			break;
		case CC_ComboBox:
			switch (subControl) {
				case SC_ComboBoxEditField: {
					const QStyleOptionComboBox *comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox *>(option);
					if (comboBoxOption) {
						if (!comboBoxOption->editable) {
							int s = option->direction == Qt::LeftToRight ? 4 : 0;
							return ParentStyle::subControlRect(control, option, subControl, widget).adjusted(s, 0, s - 4, 0);
						} else {
							// FIXME do not depend on plastique style
							QRect r = QPlastiqueStyle::subControlRect(control, option, subControl, widget);
							if (QString::fromUtf8(qVersion()) == QString::fromUtf8("4.3.0")) {
								return r;
							}
							return r.adjusted(-2, -2, 2, 2);
						}
					}
				}
				default:
					break;
			}
			break;
		case CC_GroupBox:
			switch (subControl) {
				case SC_GroupBoxCheckBox:
			//	case SC_GroupBoxFrame:
				case SC_GroupBoxLabel: {
					const QStyleOptionGroupBox *groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox *>(option);
					if (groupBoxOption) {
						if (!(groupBoxOption->features & QStyleOptionFrameV2::Flat)) {
							int x = option->direction == Qt::RightToLeft ? 8 : -8;
							int y = (subControl == SC_GroupBoxCheckBox) ? 0 : 1;
							return ParentStyle::subControlRect(control, option, subControl, widget).adjusted(x, y, x, y);
						}
					}
				}
				default:
					break;
			}
			break;
		default:
			break;
	}
	return ParentStyle::subControlRect(control, option, subControl, widget);
}


/*-----------------------------------------------------------------------*/

QStyle::SubControl SkulptureStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
#if 0
	switch (control) {
		case CC_Slider:
			return QCommonStyle::hitTestComplexControl(control, option, position, widget);
		case CC_SpinBox:
			const QStyleOptionSpinBox *spinBoxOption = qstyleoption_cast<const QStyleOptionSpinBox *>(option);
			if (spinBoxOption) {
			}

	}
#endif
	return ParentStyle::hitTestComplexControl(control, option, position, widget);
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const Private::DrawElementEntry *entry = d->draw_hash.value(element + Private::DE_Primitive);
//	entry = 0;

	Q_ASSERT(option);
//	printf("drawPrimitive %d ... ", element);

	if (entry && (!entry->type || option->type == entry->type)) {
		entry->func(painter, option, widget ? widget->backgroundRole() : QPalette::Window, widget ? widget->foregroundRole() : QPalette::WindowText, entry->data, entry->id, widget, this);
	} else {
		// painter->fillRect(option->rect, Qt::red);
		ParentStyle::drawPrimitive(element, option, painter, widget);
	}
//	printf("drawPrimitive OK\n");
}


void SkulptureStyle::drawControl(ControlElement control, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const Private::DrawElementEntry *entry = d->draw_hash.value(control + Private::DE_Element);
//	entry = 0;

	Q_ASSERT(option);
//	printf("drawControl %d ... ", control);

	if (entry && (!entry->type || option->type == entry->type)) {
		entry->func(painter, option, widget ? widget->backgroundRole() : QPalette::Window, widget ? widget->foregroundRole() : QPalette::WindowText, entry->data, entry->id, widget, this);
	} else {
		// painter->fillRect(option->rect, Qt::red);
		ParentStyle::drawControl(control, option, painter, widget);
	}
//	printf("drawControl OK\n");
}


void SkulptureStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const Private::DrawElementEntry *entry = d->draw_hash.value(control + Private::DE_Complex);
//	entry = 0;

	Q_ASSERT(option);
//	printf("drawComplex %d ... ", control);

	if (entry && (!entry->type || option->type == entry->type)) {
		entry->func(painter, option, widget ? widget->backgroundRole() : QPalette::Window, widget ? widget->foregroundRole() : QPalette::WindowText, entry->data, entry->id, widget, this);
	} else {
		// painter->fillRect(option->rect, Qt::red);
		ParentStyle::drawComplexControl(control, option, painter, widget);
	}
//	printf("drawComplex OK\n");
}


/*-----------------------------------------------------------------------*/

//#include "skulpture.moc"


/*
 * skulpture_animations.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QProgressBar>
#include <QtCore/QTimeLine>
#include <QtCore/QTimerEvent>


/*-----------------------------------------------------------------------*/
/*
 * starts/stops timer
 *
 */

void SkulptureStyle::Private::setAnimated(QWidget *widget, bool animated)
{
	if (!widget) {
		return;
	}

	if (animated && animateProgressBars) {
		animations.insert(widget);
		if (!timer) {
			timer = startTimer(60);
		}
	} else {
		animations.remove(widget);
		if (animations.isEmpty()) {
			if (timer) {
				killTimer(timer);
				timer = 0;
			}
		}
	}
}


bool SkulptureStyle::Private::isAnimated(QWidget *widget)
{
	if (!widget || !timer) {
		return false;
	}

	return animations.contains(widget);
}


void SkulptureStyle::Private::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer) {
		foreach (QWidget *widget, animations) {
			// FIXME: move this logic to progressbar
			QProgressBar *bar = qobject_cast<QProgressBar *>(widget);
			if (bar) {
				if (bar->minimum() >= bar->maximum()
				 || bar->value() < bar->maximum()) {
					bar->update();
				}
			} else {
				widget->update();
			}
		}
	}
	event->ignore();
}


/*
 * skulpture_arrows.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/
/*
 * Arrow shapes are paths. They are scaled, rotated, or flipped
 * before they are rendered.
 *
 * To create an arrow shape, use this coordinate system:
 *
 *		(-1, 1)		...		(1, 1)
 *
 *		...		(0, 0)		...
 *
 *		(-1, -1)	...		(1, -1)
 *
 * The center (0,0) is at the center of the arrow. The arrow
 * MUST POINT DOWN and should fill the whole rectangle.
 * The returned path should be closed.
 *
 * FIXME: the following does not work:
 * If the path is not closed, it will be mirrored along the y-axis
 * and closed. So to create a simple arrow, use:
 *	shape.moveTo(-1, 1);
 *	shape.lineTo(0, -1);
 *
 */
#if 0
class ArrowGlyph
{
	public:
		virtual QPainterPath shape();
		virtual ~ArrowGlyph();
};


ArrowGlyph::~ArrowGlyph()
{
}
#endif

static inline QPainterPath arrowShape()
{
	QPainterPath shape;
#if 0
	const qreal wf = 0.3;		// width of arrow body, 0 ... 1
	const qreal hf = 0.0;		// start of arrow body, 1 ... -1
	if (hf == 1.0) {
		// special case: closed triangle
		shape.moveTo(-1, 1);
		shape.lineTo(1, 1);
		shape.lineTo(0, -1);
		shape.lineTo(-1, 1);
	} else {
		// arrow
		const qreal h2f = hf;	// start of arrow, 1 ... -1
		shape.moveTo(-wf, 1);
		shape.lineTo(wf, 1);
		shape.lineTo(wf, hf);
		shape.lineTo(1, h2f);
		shape.lineTo(0, -1);
		shape.lineTo(-1, h2f);
		shape.lineTo(-wf, hf);
		shape.lineTo(-wf, 1);
	}
#elif 1
	const qreal wf = 0.8;		// width of inner arrow 0 ... 1
	const qreal hf = 0.2;		// position of inner arrow, -1 ... 1
	if (wf == 0.0) {
		// special case: closed triangle
		shape.moveTo(-1, 1);
		shape.lineTo(1, 1);
		shape.lineTo(0, -1);
		shape.lineTo(-1, 1);
	} else if (wf == 1.0) {
		// special case: full inner width
		shape.moveTo(-1, 1);
		shape.lineTo(0, hf);
		shape.lineTo(1, 1);
		shape.lineTo(0, -1);
		shape.lineTo(-1, 1);
	} else {
		// concave arrow
		shape.moveTo(-1, 1);
		shape.lineTo(-wf, 1);
		shape.lineTo(0, hf);
		shape.lineTo(wf, 1);
		shape.lineTo(1, 1);
		shape.lineTo(0, -1);
		shape.lineTo(-1, 1);
	}
#else
	// dummy shape to test matrix
	shape.moveTo(-1, 1);
	shape.lineTo(0, 1);
	shape.lineTo(0, -1);
	shape.lineTo(-1, 1);
#endif
	return shape;
}


static inline QPainterPath arrowPath(const QStyleOption *option, Qt::ArrowType arrow, bool spin)
{
	int h = option->fontMetrics.height() * (spin ? 3 : 4) / 8;
	int w = option->fontMetrics.height() / 2;
	h /= 2; w /= 2;
	if (arrow == Qt::DownArrow || arrow == Qt::RightArrow) {
		h = -h;
	}
	if (arrow == Qt::LeftArrow || arrow == Qt::RightArrow) {
		QMatrix arrowMatrix(0, w, h, 0, 0, 0);
		return arrowMatrix.map(arrowShape());
	} else {
		QMatrix arrowMatrix(w, 0, 0, h, 0, 0);
		return arrowMatrix.map(arrowShape());
	}
}


void paintScrollArrow(QPainter *painter, const QStyleOption *option, Qt::ArrowType arrow, bool spin)
{
	painter->save();
	// FIXME: combine translations with path matrix
	painter->translate(option->rect.center());
#if 1
	painter->setRenderHint(QPainter::Antialiasing, true);
#else
	painter->setRenderHint(QPainter::Antialiasing, false);
#endif
	if (painter->renderHints() & QPainter::Antialiasing) {
		painter->translate(0.5, 0.5);
	}
	switch (arrow) {
		case Qt::UpArrow:
			painter->translate(0, -0.5);
			break;
		case Qt::DownArrow:
			painter->translate(0, 0.5);
			break;
		case Qt::LeftArrow:
			painter->translate(-0.5, 0);
			break;
		case Qt::RightArrow:
			painter->translate(0.5, 0);
			break;
		case Qt::NoArrow:
			break;
	}
	painter->setPen(Qt::NoPen);
	QColor color = option->palette.color(QPalette::Text);
	if ((option->state & QStyle::State_MouseOver) && option->state & QStyle::State_Enabled /* && !(option->state & QStyle::State_Sunken)*/) {
		color = option->palette.color(QPalette::Highlight).dark(200);
	//	painter->setPen(QPen(Qt::white, 1.0));
	} else {
	//	painter->setPen(QPen(Qt::white, 0.5));
	}
	painter->setBrush(color);
	qreal opacity = painter->opacity();
	painter->setOpacity(0.7 * opacity);
	painter->drawPath(arrowPath(option, arrow, spin));
	painter->restore();
}


void paintIndicatorArrowDown(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::DownArrow, false);
}


void paintIndicatorArrowLeft(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::LeftArrow, false);
}


void paintIndicatorArrowRight(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::RightArrow, false);
}


void paintIndicatorArrowUp(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::UpArrow, false);
}


/*-----------------------------------------------------------------------*/
/*
 * Arrow shape description language
 *
 * (-1|1),(0|-1)
 *
 *
 * wf: [0...1]; hf: [-1...1]; (-1, 1),(-wf, 1),(0, hf),(wf, 1),(1, 1),(0, -1),(-1, 1);
 *
 * global variables the expressions can contain:
 *
 * none
 *
 * types:
 * bool
 * intlist
 * strlist
 * floatrange
 *
 */

/*
 * skulpture_buttons.cpp
 *
 */

#include "skulpture_p.h"
//#include <QtGui/QGradient>
#include <QtGui/QPainter>
#include <cstdio>


/*-----------------------------------------------------------------------*/
/*
 * create a path from a rectangle, with round corners.
 *
 * this is similar to Qt's RoundRect, but with more
 * control over the cubic points.
 *
 */

static QPainterPath button_path(const QRectF &rect, qreal k)
{
	// 0.1 no roundness
	// 0.2 - 0.3 minimal roundness
	// 0.5 some roundness
	// 0.7 nice roundness
	// 1.0 default roundness
	// 2.0 really round
	k *= 0.1;
#if 0
	// Oxygen like shape
	const qreal tlh_edge = 3.0 * k;
	const qreal tlv_edge = 6.0 * k;
	const qreal tlh_control = 2.0 * k;
	const qreal tlv_control = 4.0 * k;
	const qreal blh_edge = 3.0 * k;
	const qreal blv_edge = 6.0 * k;
	const qreal blh_control = 2.0 * k;
	const qreal blv_control = 4.0 * k;
	const qreal trh_edge = 3.0 * k;
	const qreal trv_edge = 6.0 * k;
	const qreal trh_control = 2.0 * k;
	const qreal trv_control = 4.0 * k;
	const qreal brh_edge = 3.0 * k;
	const qreal brv_edge = 6.0 * k;
	const qreal brh_control = 2.0 * k;
	const qreal brv_control = 4.0 * k;
#elif 0
	// testing new shape
	const qreal tlh_edge = 6.0 * k;
	const qreal tlv_edge = 3.0 * k;
	const qreal tlh_control = 4.0 * k;
	const qreal tlv_control = 2.0 * k;
	const qreal blh_edge = 6.0 * k;
	const qreal blv_edge = 6.0 * k;
	const qreal blh_control = 4.0 * k;
	const qreal blv_control = 4.0 * k;
	const qreal trh_edge = 6.0 * k;
	const qreal trv_edge = 3.0 * k;
	const qreal trh_control = 4.0 * k;
	const qreal trv_control = 2.0 * k;
	const qreal brh_edge = 6.0 * k;
	const qreal brv_edge = 6.0 * k;
	const qreal brh_control = 4.0 * k;
	const qreal brv_control = 4.0 * k;
#else
	// circle shape
	const qreal tlh_edge = 6.0 * k;
	const qreal tlv_edge = 6.0 * k;
	const qreal tlh_control = 4.0 * k;
	const qreal tlv_control = 4.0 * k;
	const qreal blh_edge = 6.0 * k;
	const qreal blv_edge = 6.0 * k;
	const qreal blh_control = 4.0 * k;
	const qreal blv_control = 4.0 * k;
	const qreal trh_edge = 6.0 * k;
	const qreal trv_edge = 6.0 * k;
	const qreal trh_control = 4.0 * k;
	const qreal trv_control = 4.0 * k;
	const qreal brh_edge = 6.0 * k;
	const qreal brv_edge = 6.0 * k;
	const qreal brh_control = 4.0 * k;
	const qreal brv_control = 4.0 * k;
#endif
	QPainterPath path;
	path.moveTo(rect.left() + tlh_edge, rect.top());
	path.lineTo(rect.right() - trh_edge, rect.top());
	path.cubicTo(rect.right() - trh_edge + trh_control, rect.top(), rect.right(), rect.top() + trv_edge - trv_control, rect.right(), rect.top() + trv_edge);
	path.lineTo(rect.right(), rect.bottom() - brv_edge);
	path.cubicTo(rect.right(), rect.bottom() - brv_edge + brv_control, rect.right() - brh_edge + brh_control, rect.bottom(), rect.right() - brh_edge, rect.bottom());
	path.lineTo(rect.left() + blh_edge, rect.bottom());
	path.cubicTo(rect.left() + blh_edge - blh_control, rect.bottom(), rect.left(), rect.bottom() - blv_edge + blv_control, rect.left(), rect.bottom() - blv_edge);
	path.lineTo(rect.left(), rect.top() + tlv_edge);
	path.cubicTo(rect.left(), rect.top() + tlv_edge - tlv_control, rect.left() + tlh_edge - tlh_control, rect.top(), rect.left() + tlh_edge, rect.top());
	return path;
}


/*-----------------------------------------------------------------------*/
/*
 * create a gradient for the inner fill of a button
 *
 */

static QBrush button_gradient(const QRectF &rect, const QColor &color, const QStyleOptionButton *option)
{
	qreal ch = color.hueF();
	qreal cs = color.saturationF() * 1.0;
	qreal cv = color.valueF() * 1.0;
	int ca = color.alpha();
	QColor col;

	if (0 && option->state & QStyle::State_MouseOver) {
		// Oxygen like highlight
		QRadialGradient gradient((rect.topLeft() + rect.topRight()) / 2.0, rect.width() / 2.0);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.01));
		gradient.setColorAt(1.0, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(0.0, col);
		return gradient;
	} else {
		if (rect.height() > 64) {
			return QColor(color);
		}
		QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
#if 1
		// flat tin
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.02));
		// FIXME other users of ca
		col.setAlpha(ca);
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		col.setAlpha(ca);
		gradient.setColorAt(1.0, col);
#elif 0
		// Skulpture default
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.02));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv + 0.02));
		gradient.setColorAt(0.4, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.01));
		gradient.setColorAt(0.6, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(1.0, col);
#elif 0
		// tin
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.02));
		gradient.setColorAt(0.10, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(0.85, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(1.0, col);
#elif 0
		// fat tin
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.02));
		gradient.setColorAt(0.15, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(0.80, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.07));
		gradient.setColorAt(1.0, col);
#elif 0
		// plastik
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(1.0, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(0.0, col);
#elif 0
		// fat plastik
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(0.1, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.04));
		gradient.setColorAt(0.85, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.07));
		gradient.setColorAt(1.0, col);
#elif 0
		// oxygen (really flat)
		gradient.setColorAt(0.0, color);
		gradient.setColorAt(1.0, color);
#elif 0
		// wet plastik (glassy, nearly like SkandaleStyle 0.0.0, but more flat)
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.01));
		gradient.setColorAt(0.40, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.05));
		gradient.setColorAt(0.405, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(1.0, col);
#elif 0
		// aluminium
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(0.40, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(0.405, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
		gradient.setColorAt(1.0, col);
#elif 0
		// platin
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.05));
		gradient.setColorAt(0.40, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.05));
		gradient.setColorAt(0.405, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(1.0, col);
#elif 0
		// silver
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.07));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.05));
		gradient.setColorAt(0.40, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(0.405, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(1.0, col);
#elif 0
		// rubber
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.01));
		gradient.setColorAt(0.30, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.00));
		gradient.setColorAt(0.70, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.07));
		gradient.setColorAt(1.0, col);
#elif 0
		// ugly silver
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.09));
		gradient.setColorAt(0.0, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.03));
		gradient.setColorAt(0.10, col);
		col.setHsvF(ch, cs, qMin(1.0, cv + 0.05));
		gradient.setColorAt(0.60, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.01));
		gradient.setColorAt(0.90, col);
		col.setHsvF(ch, cs, qMax(0.0, cv - 0.07));
		gradient.setColorAt(1.0, col);
#endif
		return gradient;
	}
}


/*-----------------------------------------------------------------------*/
/*
 * style options:
 *	features: Flat, HasMenu, DefaultButton, AutoDefaultButton, CommandLinkButton
 *	state: Sunken, On, Raised
 *	state: Enabled, HasFocus, MouseOver, Active
 *
 */

void paintButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole)
{
	const QRectF &c_rect = option->rect;
	const qreal t = 1.0;
	QRectF rect = c_rect;
#if 0
	painter->setPen(Qt::NoPen);
	painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.5), QColor(255, 0, 0, 255), QColor(0, 0, 255, 255)));
	painter->drawPath(button_path(rect, 1.5));
	return;
#endif
	bool frame = true;
	if (option->features & QStyleOptionButton::Flat && !(option->state & QStyle::State_Sunken)) {
		frame = false;
	}
	painter->setPen(Qt::NoPen);
	if ((option->features & QStyleOptionButton::DefaultButton) && (option->state & QStyle::State_Enabled)) {
		painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.3), blend_color(QColor(0, 0, 0, 10), option->palette.color(QPalette::Highlight).light(110), 0.2), blend_color(QColor(0, 0, 0, 15), option->palette.color(QPalette::Highlight).light(110), 0.2)));
	} else {
		painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.3), shaded_color(option->palette.color(QPalette::Window), -10), shaded_color(option->palette.color(QPalette::Window), -15)));
	}
	painter->drawPath(button_path(rect, 1.5));
	rect.adjust(t, t, -t, -t);
	QBrush bgbrush = option->palette.brush(option->state & QStyle::State_Enabled ? (bgrole == QPalette::NoRole ? QPalette::Button : bgrole) : QPalette::Window);
	if (bgbrush.style() == Qt::SolidPattern && bgbrush.color().alpha() == 0) {
		QColor color = option->palette.color(QPalette::Window);
		color.setAlpha(0);
		bgbrush = color;
	}
	if (frame) {
		if (option->state & QStyle::State_Enabled) {
			if (0 && option->state & QStyle::State_HasFocus) {
				painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.1), blend_color(QColor(0, 0, 0, 35), option->palette.color(QPalette::Highlight).dark(120), 0.7), blend_color(QColor(0, 0, 0, 35), option->palette.color(QPalette::Highlight).dark(120), 0.7)));
			} else if (option->state & QStyle::State_Sunken || option->state & QStyle::State_On) {
				painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.1), shaded_color(option->palette.color(QPalette::Window), -35), shaded_color(option->palette.color(QPalette::Window), -75)));
			} else {
				painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.1), shaded_color(option->palette.color(QPalette::Window), -75), shaded_color(option->palette.color(QPalette::Window), -45)));
			}
		} else {
			painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.1), shaded_color(option->palette.color(QPalette::Window), -35), shaded_color(option->palette.color(QPalette::Window), -35)));
		}
		painter->drawPath(button_path(rect, 1.3));
		rect.adjust(t, t, -t, -t);

		if (bgbrush.style() == Qt::SolidPattern) {
			QColor bgcolor = bgbrush.color();
			if (option->state & QStyle::State_On) {
				bgcolor = blend_color(bgcolor, option->palette.color(QPalette::Highlight), 0.2);
				bgbrush = button_gradient(rect, bgcolor, option);
			}
			if (option->state & QStyle::State_Enabled) {
				if (option->state & QStyle::State_Sunken) {
					bgcolor = bgcolor.light(102);
				} else if (option->state & QStyle::State_MouseOver) {
					bgcolor = bgcolor.light(104);
				}
				bgbrush = button_gradient(rect, bgcolor, option);
			}
			painter->setBrush(bgbrush);
		//	painter->setBrush(option->palette.color(QPalette::Button));
			painter->drawPath(button_path(rect, 1.1));
			if (option->state  & QStyle::State_Enabled) {
				if (option->state & QStyle::State_Sunken || option->state & QStyle::State_On) {
					painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 0.9), shaded_color(bgcolor, -10), shaded_color(bgcolor, -20)));
				} else {
					painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 0.9), shaded_color(bgcolor, -20), shaded_color(bgcolor, 160)));
				}
				painter->drawPath(button_path(rect, 1.1));
			}
		}
		painter->setBrush(bgbrush);
	} else {
		QColor bgcolor = option->palette.color(QPalette::Window);
		if (option->state & QStyle::State_MouseOver) {
			bgcolor = bgcolor.light(104);
		}
		if (option->state & QStyle::State_On) {
			bgcolor = blend_color(bgcolor, option->palette.color(QPalette::Highlight), 0.2);
		}
		painter->setBrush(bgcolor);
	}
	rect.adjust(t, t, -t, -t);
//	painter->setBrush(option->palette.color(QPalette::Button));
	painter->save();
	// make transparent buttons appear transparent
	painter->setCompositionMode(QPainter::CompositionMode_Source);
	painter->drawPath(button_path(rect, 0.9));
	painter->restore();
//	painter->drawRect(c_rect.adjusted(3, 3, -3, -3));
}


void paintPushButtonBevel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole fgrole, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	QStyleOptionButton opt = *option;

	opt.features &= ~(QStyleOptionButton::HasMenu);
	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_PushButtonBevel, &opt, painter, widget);
	if (option->features & QStyleOptionButton::Flat) {
		if (!(option->state & (QStyle::State_Sunken | QStyle::State_On))) {
			if (option->state & QStyle::State_MouseOver) {
				painter->fillRect(option->rect, QColor(255, 255, 255, 60));
			}
		}
	}
	if (option->features & QStyleOptionButton::HasMenu) {
		int size = style->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, widget);
//		QFont font;
//		font.setPointSizeF(font.pointSizeF() / 1.19);
//		opt.fontMetrics = QFontMetrics(font);
		opt.palette.setColor(QPalette::Text, opt.palette.color(fgrole));
		opt.state &= ~(QStyle::State_MouseOver);
		if (option->direction == Qt::LeftToRight) {
			opt.rect = QRect(option->rect.right() - size - 2, option->rect.top(), size, option->rect.height());
		} else {
			opt.rect = QRect(option->rect.left() + 4, option->rect.top(), size, option->rect.height());
		}
		if (option->state & (QStyle::State_Sunken | QStyle::State_On)) {
			opt.rect.translate(style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, widget), style->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, widget));
		}
		style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
	}
}


/*
 * skulpture_cache.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPixmapCache>
#include <QtCore/QDebug>
#include <QtGui/QGradient>
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/

static const bool UsePixmapCache = true;


/*-----------------------------------------------------------------------*/
/*
 * paint a pushbutton to painter
 *
 */

extern void paintButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole);

static const int button_edge_size = 16;
static const int button_inner_width = 32;

void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QPixmap pixmap;
	QRect r = option->rect;
	r.setWidth(button_inner_width + 2 * button_edge_size);

	if (/*option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ r.height() > 64) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		uint features = uint(option->features) & (QStyleOptionButton::Flat | QStyleOptionButton::DefaultButton);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		pixmapName.sprintf("scp-cbp-%x-%x-%x-%x-%llx-%x", features, uint(bgrole), state, option->direction, option->palette.cacheKey(), r.height());
	}
	if (!useCache || !QPixmapCache::find(pixmapName, pixmap)) {
		pixmap =  QPixmap(r.size());
		pixmap.fill(Qt::transparent);
	//	pixmap.fill(Qt::red);
		QPainter p(&pixmap);
		QStyleOptionButton but = *option;
		but.rect = QRect(QPoint(0, 0), r.size());
	//	### neither Clear nor Source works?
	//	p.setCompositionMode(QPainter::CompositionMode_Clear);
	//	p.setCompositionMode(QPainter::CompositionMode_Source);
	//	p.fillRect(but.rect, Qt::transparent);
	//	p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		p.setFont(painter->font());
		p.setRenderHint(QPainter::Antialiasing, true);
		paintButtonPanel(&p, &but, bgrole);
		p.end();
		if (useCache) {
			QPixmapCache::insert(pixmapName, pixmap);
		//	qDebug() << "inserted into cache:" << pixmapName;
		}
	}
	int rem;
	if (option->rect.width() == r.width()) {
		rem = r.width();
	} else {
		int side = qMin(option->rect.width() / 2, button_inner_width + button_edge_size);
		painter->drawPixmap(r.topLeft(), pixmap, QRect(0, 0, side, r.height()));
		int midw = option->rect.width() - 2 * side;
		rem = option->rect.width() - side;
		r.translate(side, 0);
		while (midw > 0) {
			int w = qMin(button_inner_width, midw);
			rem -= w;
			painter->drawPixmap(r.topLeft(), pixmap, QRect(button_edge_size, 0, w, r.height()));
			r.translate(w, 0);
			midw -= button_inner_width;
		}
	}
	painter->drawPixmap(r.topLeft(), pixmap, QRect(r.width() - rem, 0, rem, r.height()));
}


/*-----------------------------------------------------------------------*/

void paintPanelButtonTool(QPainter *painter, const QStyleOption *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);
	QStyleOptionButton button;

	if (widget && !qstrcmp(widget->metaObject()->className(), "QDockWidgetTitleButton")) {
		if (!(option->state & QStyle::State_MouseOver)) return;
	}
	button.QStyleOption::operator=(*option);
	button.features = QStyleOptionButton::None;
//	button.state &= ~(QStyle::State_Sunken | QStyle::State_Selected | QStyle::State_On);
	// ### qtconfig creates its color select buttons as disabled...
	// ### the same is done in for TitleBar buttons
	if (button.state == QStyle::State_Sunken || button.state == QStyle::State_Raised) {
		button.state |= QStyle::State_Enabled;
	} else if (!(button.state & QStyle::State_Enabled) && (option->state & QStyle::State_AutoRaise)) {
		return;
	}
	// ### don't know if tool buttons should have that big frame...
	button.rect.adjust(-1, -1, 1, 1);
	// FIXME bgrole
	paintCommandButtonPanel(painter, &button, QPalette::Button);
}


/*-----------------------------------------------------------------------*/

static void paintIndicatorCached(QPainter *painter, const QStyleOption *option, void (*paintIndicator)(QPainter *painter, const QStyleOption *option), bool useCache, const QString &pixmapName)
{
	QPixmap pixmap;

	if (!useCache || !QPixmapCache::find(pixmapName, pixmap)) {
		pixmap =  QPixmap(option->rect.size());
#if 1
		pixmap.fill(Qt::transparent);
	//	pixmap.fill(Qt::red);
#else
		pixmap.fill(option->palette.color(QPalette::Window));
#endif
		QPainter p(&pixmap);
		QStyleOption opt = *option;
		opt.rect = QRect(QPoint(0, 0), option->rect.size());
	//	p.setCompositionMode(QPainter::CompositionMode_Clear);
	//	p.setCompositionMode(QPainter::CompositionMode_Source);
	//	p.fillRect(opt.rect, Qt::transparent);
	//	p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		p.setFont(painter->font());
		p.setRenderHint(QPainter::Antialiasing, true);
		paintIndicator(&p, &opt);
		p.end();
		if (useCache) {
			QPixmapCache::insert(pixmapName, pixmap);
		//	qDebug() << "inserted into cache:" << pixmapName;
		}
	}
	painter->drawPixmap(option->rect, pixmap);
}


/*-----------------------------------------------------------------------*/
/*
 * paint a checkbox to the painter
 *
 * the size/position of the checkbox is
 * option->rect
 *
 * the following option->state flags modify appearance:
 * - enabled
 * - on, off, nochange
 * - mouse over
 * - selected
 *
 */

static void paintCheckBox(QPainter *painter, const QStyleOption *option)
{
	QRect r = option->rect;
//	painter->fillRect(option->rect, Qt::red);
	r.adjust(2, 2, -2, -2);
	if (option->state & QStyle::State_NoChange) {
		paintThinFrame(painter, r.adjusted(-1, -1, 1, 1), option->palette, 30, -10);
		paintThinFrame(painter, r, option->palette, -50, -60);
		paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, 0, 60);
		QColor color = option->palette.color(QPalette::Window);
		if (option->state & QStyle::State_Enabled) {
			if (option->state & QStyle::State_Sunken) {
				color = color.dark(110);
			} else if (option->state & QStyle::State_MouseOver) {
				color = color.light(106);
			}
		} else {
			color = color.dark(106);
		}
		painter->fillRect(r.adjusted(2, 2, -2, -2), color);
	} else {
		QColor color = option->palette.color(QPalette::Base);
		if (!(option->state & QStyle::State_On) && !(option->state & QStyle::State_Enabled)) {
			color = option->palette.color(QPalette::Window);
		}
		painter->fillRect(r.adjusted(1, 1, -1, -1), color);
		QLinearGradient checkGradient(r.topLeft(), r.bottomLeft());
		checkGradient.setColorAt(0.0, shaded_color(color, -15));
		checkGradient.setColorAt(1.0, shaded_color(color, 60));
		painter->fillRect(r.adjusted(1, 1, -1, -1), checkGradient);
		paintRecessedFrame(painter, r.adjusted(-1, -1, 1, 1), option->palette, RF_Small);
		if (option->state & QStyle::State_Enabled) {
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, 140, 200);
		} else {
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, 180, 180);
		}
#if 0
		// FIXME dont use round highlight, but works better
		QRadialGradient on_gradient(rf.center(), r.width() / 2.0 + 2);
		QColor endc = option->palette.color(QPalette::Highlight).light(150);
		QColor midc = endc;

		if (option->state & QStyle::State_MouseOver) {
			midc.setAlpha(250);
		} else {
			midc.setAlpha(120);
		}
		endc.setAlpha(0);
		if (option->state & QStyle::State_Sunken) {
			on_gradient.setColorAt(0.0, option->palette.color(QPalette::Base));
		} else if (option->state & QStyle::State_On) {
			on_gradient.setColorAt(0.0, option->palette.color(QPalette::Highlight).dark(150));
			on_gradient.setColorAt(0.3, option->palette.color(QPalette::Highlight));
		} else if (!(option->state & QStyle::State_MouseOver)) {
			return;
		}
		on_gradient.setColorAt(0.4, option->palette.color(QPalette::Highlight).light(150));

		on_gradient.setColorAt(0.5, midc);
		on_gradient.setColorAt(1.0, endc);
		painter->setPen(Qt::NoPen);
		painter->setBrush(on_gradient);
		painter->drawRect(r);
#endif
		{
		QRectF r = option->rect;
		r.adjust(2, 2, -2, -2);
		QPainterPath path;
		qreal a;
		r.moveCenter(QPointF(0, 0));
		// FIXME make a loop
		if (option->state & (QStyle::State_On | QStyle::State_Sunken | QStyle::State_MouseOver)) {
			painter->save();
			r.adjust(1, 1, -1, -1);
			path = QPainterPath();
			path.setFillRule(Qt::WindingFill);
			a = r.width() / 4.0;
			path.addRect(r.adjusted(a, 0, -a, 0));
			path.addRect(r.adjusted(0, a, 0, -a));
			painter->setPen(Qt::NoPen);
			painter->setBrush(option->palette.color(QPalette::Highlight).light(150));
			painter->setOpacity(painter->opacity() * 0.5);
			painter->translate(QRectF(option->rect).center());
			painter->rotate(50);
			painter->drawPath(path);
			painter->restore();
		}
		if (option->state & (QStyle::State_On | QStyle::State_Sunken)) {
			painter->save();
			r.adjust(1, 1, -1, -1);
			path = QPainterPath();
			path.setFillRule(Qt::WindingFill);
			a = r.width() / 3.0;
			path.addRect(r.adjusted(a, 0, -a, 0));
			path.addRect(r.adjusted(0, a, 0, -a));
			painter->setPen(Qt::NoPen);
			painter->setBrush(option->palette.color(QPalette::Highlight));
			painter->setOpacity(painter->opacity() * (option->state & QStyle::State_On ? 0.5 : 0.7));
			painter->translate(QRectF(option->rect).center());
			painter->rotate(50);
			painter->drawPath(path);
			painter->restore();
		}
		if (option->state & QStyle::State_On) {
			painter->save();
			path = QPainterPath();
			path.setFillRule(Qt::WindingFill);
			r.adjust(1, 1, -1, -1);
			a = r.width() / 3.0;
			path.addRect(r.adjusted(a, 0, -a, 0));
			path.addRect(r.adjusted(0, a, 0, -a));
			painter->setPen(Qt::NoPen);
			painter->setBrush(option->palette.color(QPalette::Highlight).dark(option->state & QStyle::State_Sunken ? 80 : 150));
			painter->setOpacity(painter->opacity() * 0.8);
			painter->translate(QRectF(option->rect).center());
			painter->rotate(50);
			painter->drawPath(path);
			painter->restore();
		}
		}
	}
}


void paintIndicatorCheckBox(QPainter *painter, const QStyleOptionButton *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QRect r = option->rect;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ r.width() * r.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_NoChange | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-icb-%x-%x-%llx-%x-%x", state, option->direction, option->palette.cacheKey(), r.width(), r.height());
	}
	paintIndicatorCached(painter, option, paintCheckBox, useCache, pixmapName);
}


/*-----------------------------------------------------------------------*/
/*
 * paint a radiobutton to the painter
 *
 * the size/position of the radiobutton is
 * option->rect
 *
 * the following option->state flags modify appearance:
 * - enabled
 * - on, off
 * - mouse over
 * - selected
 *
 */

static void paintRadioButton(QPainter *painter, const QStyleOption *option)
{
//	const qreal angle = option->direction == Qt::LeftToRight ? 135.0 : 45.0;

//	painter->fillRect(option->rect, Qt::red);
	int d = qMin(option->rect.width(), option->rect.height()) - 4;
	QRect r(option->rect.left() + (option->rect.width() - d) / 2, option->rect.top() + (option->rect.height() - d) / 2, d, d);
//	painter->fillRect(r, Qt::white);
	QLinearGradient border_gradient(r.topLeft(), r.bottomRight());
	border_gradient.setColorAt(0.0, option->palette.color(QPalette::Window).lighter(120));
//	border_gradient.setColorAt(0.1, option->palette.color(QPalette::Window));
//	border_gradient.setColorAt(0.7, option->palette.color(QPalette::Window).darker(400));
	border_gradient.setColorAt(1.0, option->palette.color(QPalette::Window).darker(20));
	int t = 2;
	painter->setPen(QPen(border_gradient, t));
#if 0
	QLinearGradient dial_gradient(r.topLeft(), r.bottomLeft());
	dial_gradient.setColorAt(0.0, option->palette.color(QPalette::Window).darker(105));
	dial_gradient.setColorAt(0.5, option->palette.color(QPalette::Window).lighter(102));
	dial_gradient.setColorAt(1.0, option->palette.color(QPalette::Window).lighter(105));
#elif 0
	QLinearGradient dial_gradient(r.topLeft(), r.bottomLeft());
	dial_gradient.setColorAt(0.0, option->palette.color(QPalette::Window).lighter(102));
	dial_gradient.setColorAt(0.4, option->palette.color(QPalette::Window).darker(102));
	dial_gradient.setColorAt(0.5, option->palette.color(QPalette::Window).darker(105));
	dial_gradient.setColorAt(1.0, option->palette.color(QPalette::Window).lighter(102));
#else
	QConicalGradient dial_gradient(r.center(), -100);
	if (option->state & QStyle::State_Enabled) {
		dial_gradient.setColorAt(0.0, option->palette.color(QPalette::Base).darker(102));
		dial_gradient.setColorAt(0.3, option->palette.color(QPalette::Base).darker(106));
		dial_gradient.setColorAt(0.7, option->palette.color(QPalette::Base).darker(110));
		dial_gradient.setColorAt(1.0, option->palette.color(QPalette::Base).darker(102));
	} else {
		dial_gradient.setColorAt(0.0, option->palette.color(QPalette::Window).lighter(102));
		dial_gradient.setColorAt(0.4, option->palette.color(QPalette::Window).darker(102));
		dial_gradient.setColorAt(0.5, option->palette.color(QPalette::Window).darker(105));
		dial_gradient.setColorAt(1.0, option->palette.color(QPalette::Window).lighter(102));
	}
#endif
	painter->setBrush(dial_gradient);
	t = t / 2;
	painter->drawEllipse(r.adjusted(t, t, -t, -t));

	QLinearGradient border2_gradient(r.topLeft(), r.bottomRight());
	border2_gradient.setColorAt(1.0, option->palette.color(QPalette::Window).darker(option->state & QStyle::State_On ?  120 : 110));
	border2_gradient.setColorAt(0.5, option->palette.color(QPalette::Window));
	border2_gradient.setColorAt(0.0, option->palette.color(QPalette::Window).darker(200));
	painter->drawEllipse(r.adjusted(t, t, -t, -t));
//	t = option->state & QStyle::State_On ? 2 : 1;
	t = 1;
	painter->setPen(QPen(border2_gradient, t));
	painter->setBrush(Qt::NoBrush);
	painter->drawEllipse(r);
	if (1 || option->state & QStyle::State_On) {
		QRectF rf = r;
		QRadialGradient on_gradient(rf.center(), d / 2.0 + 2);
		QColor endc = option->palette.color(QPalette::Highlight).light(150);
		QColor midc = endc;
#if 0
		midc.setAlpha(120);
		endc.setAlpha(0);
		on_gradient.setColorAt(0.0, option->palette.color(QPalette::Highlight).dark(150));
		on_gradient.setColorAt(0.3, option->palette.color(QPalette::Highlight));
		on_gradient.setColorAt(0.4, option->palette.color(QPalette::Highlight).light(150));
#else
		if (option->state & QStyle::State_MouseOver) {
			midc.setAlpha(10);
		} else {
			midc.setAlpha(120);
		}
		endc.setAlpha(0);
		if (option->state & QStyle::State_Sunken) {
			on_gradient.setColorAt(0.0, option->palette.color(QPalette::Base));
		} else if (option->state & QStyle::State_On) {
			on_gradient.setColorAt(0.0, option->palette.color(QPalette::Highlight).dark(150));
			on_gradient.setColorAt(0.3, option->palette.color(QPalette::Highlight));
		} else if (!(option->state & QStyle::State_MouseOver)) {
			return;
		}
		on_gradient.setColorAt(0.4, option->palette.color(QPalette::Highlight).light(150));
#endif
		on_gradient.setColorAt(0.5, midc);
		on_gradient.setColorAt(1.0, endc);
		painter->setPen(Qt::NoPen);
		painter->setBrush(on_gradient);
		painter->drawEllipse(r);
	//	painter->drawEllipse(r.adjusted(-2, -2, 2, 2));
	}
}


void paintIndicatorRadioButton(QPainter *painter, const QStyleOptionButton *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QRect r = option->rect;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ r.width() * r.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-irb-%x-%x-%llx-%x-%x", state, option->direction, option->palette.cacheKey(), r.width(), r.height());
	}
	paintIndicatorCached(painter, option, paintRadioButton, useCache, pixmapName);
}


/*-----------------------------------------------------------------------*/

void paintIndicatorMenuCheckMark(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	// FIXME QPlastiqueStyle does not call this, but calls
	// PE_IndicatorCheckBox/RadioButton directly
	QStyleOptionButton buttonOption;

	buttonOption.QStyleOption::operator=(*option);
//	buttonOption.rect.adjust(-2, -2, 2, 2);
//	qDebug("here!");
//	printf("state 0x%08x\n", uint(buttonOption.state));
	if (option->state & QStyle::State_Enabled) {
		if (buttonOption.state & QStyle::State_On) {
			buttonOption.state |= QStyle::State_Sunken;
		}
	}
	buttonOption.state |= QStyle::State_On;
	if (widget) {
		buttonOption.palette = widget->palette();
		if (option->state & QStyle::State_Enabled) {
			if (option->state & QStyle::State_Active) {
				buttonOption.palette.setCurrentColorGroup(QPalette::Active);
			} else {
				buttonOption.palette.setCurrentColorGroup(QPalette::Inactive);
			}
		} else {
			buttonOption.palette.setCurrentColorGroup(QPalette::Disabled);
		}
	}
	if (option->checkType == QStyleOptionMenuItem::Exclusive) {
		QSize size(style->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight, option, widget));
		buttonOption.rect = QRect(option->rect.center() - QPoint(size.width() / 2, size.height() / 2), size);
		paintIndicatorRadioButton(painter, &buttonOption);
	} else {
		QSize size(style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_IndicatorHeight, option, widget));
		buttonOption.rect = QRect(option->rect.center() - QPoint(size.width() / 2, size.height() / 2), size);
		paintIndicatorCheckBox(painter, &buttonOption);
	}
}


/*-----------------------------------------------------------------------*/

static void paintGrip(QPainter *painter, const QStyleOption *option)
{
//	painter->fillRect(option->rect, Qt::red);
	int d = qMin(option->rect.width(), option->rect.height());
	// good values are 3 (very small), 4 (small), 5 (good), 7 (large), 9 (huge)
	// int d = 5;
	QRectF rect(QRectF(option->rect).center() - QPointF(d / 2.0, d / 2.0), QSizeF(d, d));
	const qreal angle = option->direction == Qt::LeftToRight ? 135.0 : 45.0;
//	const qreal angle = 90;
	QColor color;
	qreal opacity = 0.9;

	painter->save();
	painter->setPen(Qt::NoPen);
	if (option->state & QStyle::State_Enabled) {
		if (option->state & QStyle::State_Sunken) {
			color = option->palette.color(QPalette::Highlight).dark(110);
		} else {
			color = option->palette.color(QPalette::Button);
		}
	} else {
		color = option->palette.color(QPalette::Window);
		opacity = 0.5;
	}

	QConicalGradient gradient1(rect.center(), angle);
	gradient1.setColorAt(0.0, shaded_color(color, -110));
	gradient1.setColorAt(0.25, shaded_color(color, -30));
	gradient1.setColorAt(0.5, shaded_color(color, 180));
	gradient1.setColorAt(0.75, shaded_color(color, -30));
	gradient1.setColorAt(1.0, shaded_color(color, -110));
	painter->setBrush(color);
	painter->drawEllipse(rect);
	painter->setBrush(gradient1);
	painter->setOpacity(opacity);
	painter->drawEllipse(rect);
	painter->setOpacity(1.0);
	if (d > 2) {
		QConicalGradient gradient2(rect.center(), angle);
		gradient2.setColorAt(0.0, shaded_color(color, -40));
		gradient2.setColorAt(0.25, shaded_color(color, 0));
		gradient2.setColorAt(0.5, shaded_color(color, 210));
		gradient2.setColorAt(0.75, shaded_color(color, 0));
		gradient2.setColorAt(1.0, shaded_color(color, -40));
		rect.adjust(1, 1, -1, -1);
		painter->setBrush(color);
		painter->drawEllipse(rect);
		painter->setBrush(gradient2);
		painter->setOpacity(opacity);
		painter->drawEllipse(rect);
		painter->setOpacity(1.0);
		if (d > 8) {
			QConicalGradient gradient3(rect.center(), angle);
			gradient3.setColorAt(0.0, shaded_color(color, -10));
			gradient3.setColorAt(0.25, shaded_color(color, 0));
			gradient3.setColorAt(0.5, shaded_color(color, 180));
			gradient3.setColorAt(0.75, shaded_color(color, 0));
			gradient3.setColorAt(1.0, shaded_color(color, -10));
			rect.adjust(2, 2, -2, -2);
			painter->setBrush(color);
			painter->drawEllipse(rect);
			painter->setBrush(gradient3);
			painter->setOpacity(opacity);
			painter->drawEllipse(rect);
			painter->setOpacity(1.0);
		}
	}

	painter->restore();
}


void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QRect r = option->rect;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ r.width() * r.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-isg-%x-%x-%x-%llx-%x-%x", uint(bgrole), state, option->direction, option->palette.cacheKey(), r.width(), r.height());
	}
	paintIndicatorCached(painter, option, paintGrip, useCache, pixmapName);
}


/*-----------------------------------------------------------------------*/

void paintDialBase(QPainter *painter, const QStyleOption *option)
{
//	painter->fillRect(option->rect, Qt::red);
//	painter->save();
//	painter->setRenderHint(QPainter::Antialiasing, true);
	int d = qMin(option->rect.width(), option->rect.height());
/*	if (d > 20 && option->notchTarget > 0) {
		d += -1;
	}
*/	QRectF r((option->rect.width() - d) / 2.0, (option->rect.height() - d) / 2.0, d, d);
	const qreal angle = option->direction == Qt::LeftToRight ? 135.0 : 45.0;
//	const qreal angle = 90;

	painter->setPen(Qt::NoPen);
	QColor border_color = option->palette.color(QPalette::Window);
#if 0
	{
		QRadialGradient depth_gradient(r.center(), d / 2);
//		depth_gradient.setColorAt(0.0, QColor(0, 0, 0, 255));
		depth_gradient.setColorAt(0.5, QColor(0, 0, 0, 255));
		depth_gradient.setColorAt(1.0, QColor(0, 0, 0, 0));
		painter->setBrush(depth_gradient);
		painter->drawEllipse(r);
	}
#endif
#if 1
	if (option->state & QStyle::State_HasFocus && option->state & QStyle::State_KeyboardFocusChange) {
		painter->setBrush(option->palette.color(QPalette::Highlight).dark(180));
		r.adjust(1, 1, -1, -1);
		painter->drawEllipse(r);
		painter->setBrush(border_color);
		r.adjust(1, 1, -1, -1);
		painter->drawEllipse(r);
		r.adjust(1, 1, -1, -1);
	} else {
		painter->setBrush(border_color);
		r.adjust(1, 1, -1, -1);
		painter->drawEllipse(r);
		r.adjust(1, 1, -1, -1);
		QConicalGradient border_gradient(r.center(), angle);
		if (!(option->state & QStyle::State_Enabled)) {
			border_color = border_color.lighter(120);
		}
		border_gradient.setColorAt(0.0, border_color.darker(180));
		border_gradient.setColorAt(0.3, border_color.darker(130));
		border_gradient.setColorAt(0.5, border_color.darker(170));
		border_gradient.setColorAt(0.7, border_color.darker(130));
		border_gradient.setColorAt(1.0, border_color.darker(180));
		painter->setBrush(border_gradient);
//		painter->setBrush(Qt::blue);
		painter->drawEllipse(r);
		r.adjust(1, 1, -1, -1);
	}
	d -= 6;

	QColor dial_color;
	if (option->state & QStyle::State_Enabled) {
		dial_color = option->palette.color(QPalette::Button).lighter(101);
		if (option->state & QStyle::State_MouseOver) {
			dial_color = dial_color.lighter(103);
		}
	} else {
		dial_color = option->palette.color(QPalette::Window);
	}
	qreal t = option->state & QStyle::State_Enabled ? 2.0 : 1.5;
	if (1) {
		// ###: work around Qt 4.3.0 bug? (this works for 4.3.1)
		QConicalGradient border_gradient(r.center(), angle);
		border_gradient.setColorAt(0.0, dial_color.lighter(120));
		border_gradient.setColorAt(0.2, dial_color);
		border_gradient.setColorAt(0.5, dial_color.darker(130));
		border_gradient.setColorAt(0.8, dial_color);
		border_gradient.setColorAt(1.0, dial_color.lighter(120));
		painter->setPen(QPen(border_gradient, t));
	} else {
		painter->setPen(QPen(Qt::red, t));
	}
#if 0
	QLinearGradient dial_gradient(r.topLeft(), r.bottomLeft());
	dial_gradient.setColorAt(0.0, dial_color.darker(105));
	dial_gradient.setColorAt(0.5, dial_color.lighter(102));
	dial_gradient.setColorAt(1.0, dial_color.lighter(105));
#elif 1
	QLinearGradient dial_gradient(option->direction == Qt::RightToLeft ? r.topRight() : r.topLeft(), option->direction == Qt::RightToLeft ? r.bottomLeft() : r.bottomRight());
//	QLinearGradient dial_gradient(r.topLeft(), r.bottomLeft());
	if (true || option->state & QStyle::State_Enabled) {
#if 1
		dial_gradient.setColorAt(0.0, dial_color.darker(106));
		dial_gradient.setColorAt(1.0, dial_color.lighter(104));
#else
		dial_gradient.setColorAt(0.0, dial_color.lighter(101));
		dial_gradient.setColorAt(0.5, dial_color.darker(103));
		dial_gradient.setColorAt(1.0, dial_color.lighter(104));
#endif
	} else {
		dial_gradient.setColorAt(0.0, dial_color);
		dial_gradient.setColorAt(1.0, dial_color);
	}
#elif 0
	QConicalGradient dial_gradient(r.center(), angle);
	dial_gradient.setColorAt(0.0, dial_color.lighter(102));
	dial_gradient.setColorAt(0.5, dial_color.darker(103));
	dial_gradient.setColorAt(1.0, dial_color.lighter(102));
#else
	QBrush dial_gradient(dial_color);
#endif
	painter->setBrush(dial_gradient);
	t = t / 2;
	painter->drawEllipse(r.adjusted(t, t, -t, -t));

//	painter->setPen(Qt::NoPen);
//	painter->setBrush(dial_color);
//	painter->drawEllipse(r.adjusted(d / 4, d / 4, - d / 4, - d / 4));

#if 0
	QLinearGradient border2_gradient(r.topLeft(), r.bottomRight());
	border2_gradient.setColorAt(1.0, dial_color.darker(425));
	border2_gradient.setColorAt(0.9, dial_color);
	border2_gradient.setColorAt(0.0, dial_color.darker(400));
	painter->setPen(QPen(border2_gradient, 1.3));
	painter->setBrush(Qt::NoBrush);
	painter->drawEllipse(r.adjusted(0.3, 0.3, -0.3, -0.3));
#endif
//	painter->restore();
#endif
}


void paintCachedDialBase(QPainter *painter, const QStyleOptionSlider *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QRect r = option->rect;
	int d = qMin(r.width(), r.height());

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ d > 128) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus | QStyle::State_KeyboardFocusChange);
		}
	//	state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-qdb-%x-%x-%llx-%x", state, option->direction, option->palette.cacheKey(), d);
	}
	paintIndicatorCached(painter, option, paintDialBase, useCache, pixmapName);
}


void paintIndicatorDial(QPainter *painter, const QStyleOptionSlider *option)
{
	int d = qMin(option->rect.width(), option->rect.height());
	QRect rect(option->rect.center() - QPoint((d - 1) / 2, (d - 1) / 2), QSize(d, d));
	QStyleOptionSlider opt;
	opt.QStyleOption::operator=(*option);
	opt.rect = rect;
	paintCachedDialBase(painter, &opt);
}


/*-----------------------------------------------------------------------*/

void paintBranchChildren(QPainter *painter, const QStyleOption *option)
{
	painter->setBrush(option->palette.color(QPalette::Text));
	painter->setPen(Qt::NoPen);
	painter->drawEllipse(option->rect);
}


void paintCachedIndicatorBranchChildren(QPainter *painter, const QStyleOption *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;
	QRect r = option->rect;
	int d = qMin(r.width(), r.height());

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ d > 64) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_Open);
	//	if (!(state & QStyle::State_Enabled)) {
	//		state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus | QStyle::State_KeyboardFocusChange);
	//	}
	//	state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-qibc-%x-%x-%llx-%x", state, option->direction, option->palette.cacheKey(), d);
	}
	paintIndicatorCached(painter, option, paintBranchChildren, useCache, pixmapName);
}


/*
 * skulpture_color.cpp
 *
 */

#include "skulpture_p.h"


/*-----------------------------------------------------------------------*/

QColor shaded_color(const QColor &color, int shade)
{
#if 1
	const qreal contrast = 1.0;
	int r, g, b;
	color.getRgb(&r, &g, &b);
	int gray = qGray(r, g, b);
	gray = qMax(r, qMax(g, b));
	gray = (r + b + g + 3 * gray) / 6;
	if (shade < 0) {
		qreal k = 220.0 / 255.0 * shade;
		k *= contrast;
		int a = 255;
		if (gray > 0) {
			a = int(k * 255 / (0 - gray));
			if (a < 0) a = 0;
			if (a > 255) a = 255;
		}
		return QColor(0, 0, 0, a);
	} else {
		qreal k = (255 - 220.0) / (255.0) * shade;
		k *= contrast;
		int a = 255;
		if (gray < 255) {
			a = int(k * 255 / (255 - gray));
			if (a < 0) a = 0;
			if (a > 255) a = 255;
		}
		return QColor(255, 255, 255, a);
	}
#else
	if (shade < 0) {
		return QColor(0, 0, 0, -shade);
	} else {
		return QColor(255, 255, 255, shade);
	}
#endif
}


QColor blend_color(const QColor &c0, const QColor &c1, qreal blend)
{
#if 0 // more exact, but probably slower
	QColor c;

	blend = qMin(1.0, qMax(0.0, blend));
	c.setRgbF(
		c0.redF() * (1.0 - blend) + c1.redF() * blend,
		c0.greenF() * (1.0 - blend) + c1.greenF() * blend,
		c0.blueF() * (1.0 - blend) + c1.blueF() * blend,
		c0.alphaF() * (1.0 - blend) + c1.alphaF() * blend
	);
	return c;
#else
	int b = int(0.5 + 256.0 * blend);
	b = qMin(256, qMax(0, b));
	QRgb rgba0 = c0.rgba();
	QRgb rgba1 = c1.rgba();
	return QColor(
		qRed(rgba0) + (((qRed(rgba1) - qRed(rgba0)) * b) >> 8),
		qGreen(rgba0) + (((qGreen(rgba1) - qGreen(rgba0)) * b) >> 8),
		qBlue(rgba0) + (((qBlue(rgba1) - qBlue(rgba0)) * b) >> 8),
		qAlpha(rgba0) + (((qAlpha(rgba1) - qAlpha(rgba0)) * b) >> 8)
	);
#endif
}


/*-----------------------------------------------------------------------*/
/*
 * Don't be too fancy about colors, because KDE 4
 * has a different color system
 */

static void computePaletteGroups(QPalette &palette)
{
#if 0 // force colors (for KDE4 alpha)
	palette.setColor(QPalette::Active, QPalette::Window, QColor(221, 220, 218));
	palette.setColor(QPalette::Active, QPalette::WindowText, QColor(0, 0, 0));
	palette.setColor(QPalette::Active, QPalette::Button, QColor(226, 225, 222));
	palette.setColor(QPalette::Active, QPalette::ButtonText, QColor(0, 0, 0));
	palette.setColor(QPalette::Active, QPalette::Base, QColor(250, 250, 250));
	palette.setColor(QPalette::Active, QPalette::Text, QColor(0, 0, 0));
//	palette.setColor(QPalette::Active, QPalette::Highlight, QColor(250, 210, 0));
//	palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(0, 0, 0));
#endif
	palette.setColor(QPalette::Inactive, QPalette::Window, palette.color(QPalette::Active, QPalette::Window));
	palette.setColor(QPalette::Inactive, QPalette::WindowText, palette.color(QPalette::Active, QPalette::WindowText));
	palette.setColor(QPalette::Inactive, QPalette::Button, palette.color(QPalette::Active, QPalette::Button));
	palette.setColor(QPalette::Inactive, QPalette::ButtonText, palette.color(QPalette::Active, QPalette::ButtonText));
	palette.setColor(QPalette::Inactive, QPalette::Base, palette.color(QPalette::Active, QPalette::Base));
	palette.setColor(QPalette::Inactive, QPalette::Text, palette.color(QPalette::Active, QPalette::Text));
//	palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(250, 210, 0));
//	palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(0, 0, 0));
#if 0
	palette.setColor(QPalette::Active, QPalette::WindowText, QColor(0, 0, 0));
	palette.setColor(QPalette::Active, QPalette::ButtonText, QColor(0, 0, 0));
	palette.setColor(QPalette::Active, QPalette::Text, QColor(0, 0, 0));
#endif
	palette.setColor(QPalette::Active, QPalette::AlternateBase, palette.color(QPalette::Active, QPalette::Base).dark(103));
	palette.setColor(QPalette::Inactive, QPalette::AlternateBase, palette.color(QPalette::Inactive, QPalette::Base).dark(103));
	palette.setColor(QPalette::Disabled, QPalette::AlternateBase, palette.color(QPalette::Disabled, QPalette::Base).dark(103));

	palette.setColor(QPalette::Disabled, QPalette::Window, palette.color(QPalette::Window));
	palette.setColor(QPalette::Disabled, QPalette::WindowText, shaded_color(palette.color(QPalette::Window), -40));
	palette.setColor(QPalette::Disabled, QPalette::Button, palette.color(QPalette::Button));
	palette.setColor(QPalette::Disabled, QPalette::ButtonText, shaded_color(palette.color(QPalette::Button), -40));
	palette.setColor(QPalette::Disabled, QPalette::Base, palette.color(QPalette::Window).light(100));
	palette.setColor(QPalette::Disabled, QPalette::Text, palette.color(QPalette::Base).dark(140));
	palette.setColor(QPalette::Disabled, QPalette::Highlight, palette.color(QPalette::Window).dark(120));
	// ### does Qt use these for QHLine / QVLine ?
#if 1
//	palette.setColor(QPalette::Active, QPalette::Dark, QColor(0, 0, 0, 20));
//	palette.setColor(QPalette::Active, QPalette::Light, QColor(255, 255, 255, 60));
//	palette.setColor(QPalette::Inactive, QPalette::Dark, QColor(0, 0, 0, 20));
//	palette.setColor(QPalette::Inactive, QPalette::Light, QColor(255, 255, 255, 60));
	palette.setColor(QPalette::Disabled, QPalette::Dark, shaded_color(palette.color(QPalette::Window), -20));
	palette.setColor(QPalette::Disabled, QPalette::Light, shaded_color(palette.color(QPalette::Window), 60));
#endif
}


QPalette SkulptureStyle::standardPalette() const
{
	QPalette palette = ParentStyle::standardPalette();
#if 0
palette.setColor(QPalette::Window, QColor(225, 222, 215));
palette.setColor(QPalette::Button, QColor(230, 227, 220));
palette.setColor(QPalette::Base, QColor(250, 250, 248));
#elif 0 // dark scheme (testing)
palette.setColor(QPalette::Window, QColor(70, 70, 70));
palette.setColor(QPalette::Button, QColor(90, 90, 90));
palette.setColor(QPalette::Base, QColor(0, 0, 0));
palette.setColor(QPalette::Text, QColor(255, 255, 255));
palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
palette.setColor(QPalette::WindowText, QColor(255, 255, 255));
#elif 0 // Skulpture 0.0.3
//palette.setColor(QPalette::Window, QColor(235, 235, 235));
palette.setColor(QPalette::Window, QColor(220, 220, 220));
//palette.setColor(QPalette::Window, QColor(195, 195, 195));
//palette.setColor(QPalette::Button, QColor(255, 255, 255));
palette.setColor(QPalette::Button, QColor(225, 225, 225));
//palette.setColor(QPalette::Button, QColor(25, 25, 25));
//palette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
palette.setColor(QPalette::Base, QColor(250, 250, 250));
#else // Skulpture 0.0.4
palette.setColor(QPalette::Window, QColor(221, 220, 218));
palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
palette.setColor(QPalette::Button, QColor(226, 225, 222));
palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
palette.setColor(QPalette::Base, QColor(250, 250, 250));
palette.setColor(QPalette::Text, QColor(0, 0, 0));
#endif

#if 0 // turkise
palette.setColor(QPalette::Highlight, QColor(120, 230, 220));
palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
#elif 0 // green
palette.setColor(QPalette::Highlight, QColor(150, 210, 150));
//palette.setColor(QPalette::Highlight, QColor(220, 180, 60));
palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
#elif 1 // gold
palette.setColor(QPalette::Highlight, QColor(250, 210, 0));
//palette.setColor(QPalette::Highlight, QColor(180, 180, 180));
palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
#elif 0 // light blue
palette.setColor(QPalette::Highlight, QColor(160, 210, 250));
palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
#else // dark blue
palette.setColor(QPalette::Highlight, QColor(80, 120, 200));
palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
#endif

//	palette.setColor(QPalette::Button, QColor(215, 225, 210));
//	palette.setColor(QPalette::Button, QColor(230, 230, 230));
//	palette.setColor(QPalette::Button, QColor(240, 200, 210));
//	palette.setColor(QPalette::Highlight, QColor(40, 70, 80));
//	palette.setColor(QPalette::Highlight, QColor(80, 130, 160));
//	palette.setColor(QPalette::Highlight, QColor(80, 130, 60));
//	palette.setColor(QPalette::Highlight, QColor(140, 70, 50));
	computePaletteGroups(palette);
	return palette;
}


void SkulptureStyle::polish(QPalette &palette)
{
	ParentStyle::polish(palette);
	computePaletteGroups(palette);
}


/*
 * skulpture_combobox.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/

void paintComboBox(QPainter *painter, const QStyleOptionComboBox *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	QStyleOptionComboBox opt;
	opt = *option;
	if (opt.state & QStyle::State_On) {
		opt.state |= QStyle::State_Sunken;
	} else {
		opt.state &= ~QStyle::State_Sunken;
	}
	opt.state &= ~QStyle::State_On;
	QRect r  = ((QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, widget);
	if (option->direction == Qt::LeftToRight) {
		r.adjust(-2, 0, 1, 0);
	} else {
		r.adjust(-1, 0, 2, 0);
	}
	if (option->subControls & (QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxEditField)) {
		if (!option->editable) {
			QStyleOptionButton buttonOption;
			buttonOption.QStyleOption::operator=(opt);
			// buttonOption.palette.setColor(bgrole, buttonOption.palette.color(QPalette::Window));
			style->drawPrimitive(QStyle::PE_PanelButtonCommand, &buttonOption, painter, widget);
			// separator
			int ox = r.width() - 1;
			if (opt.state & QStyle::State_Sunken) {
				ox -= 1;
			}
			if (option->direction == Qt::LeftToRight) {
				painter->fillRect(option->rect.adjusted(option->rect.width() - ox - 1, 2, -ox, -2), shaded_color(option->palette.color(QPalette::Button), -30));
				painter->fillRect(option->rect.adjusted(option->rect.width() - ox, 2, -ox + 1, -2), shaded_color(option->palette.color(QPalette::Button), 80));
			} else {
				painter->fillRect(option->rect.adjusted(ox - 1, 2, ox - option->rect.width(), -2), shaded_color(option->palette.color(QPalette::Button), -30));
				painter->fillRect(option->rect.adjusted(ox, 2, ox - option->rect.width() + 1, -2), shaded_color(option->palette.color(QPalette::Button), 80));
			}
		} else {
			QColor color = option->palette.color(QPalette::Window);
			if (option->state & QStyle::State_Enabled) {
				color = option->palette.color(QPalette::Window).lighter(107);
			}
			if (option->direction == Qt::LeftToRight) {
				painter->fillRect(r.adjusted(3, 0, -1, 0), color);
			} else {
				painter->fillRect(r.adjusted(1, 0, -3, 0), color);
			}

			QStyleOptionFrame frameOpt;
			frameOpt.QStyleOption::operator=(*option);
			frameOpt.rect = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxFrame, widget);
			frameOpt.state |= QStyle::State_Sunken;
			frameOpt.lineWidth = style->pixelMetric(QStyle::PM_ComboBoxFrameWidth, &frameOpt, widget);
			frameOpt.midLineWidth = 0;
			color = option->palette.color(QPalette::Window);
			if (option->state & QStyle::State_Enabled) {
				color = option->palette.color(QPalette::Base);
				if (option->state & QStyle::State_HasFocus) {
					color = blend_color(color, option->palette.color(QPalette::Highlight), 0.15);
				}
			}
			QRect edit = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxEditField, widget).adjusted(2, 2, -2, -2);
			painter->fillRect(edit, color);
			if (option->state & QStyle::State_Enabled && option->rect.height() <= 64) {
				QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
				if (color.valueF() > 0.9) {
					panelGradient.setColorAt(0.0, shaded_color(color, -20));
				}
				panelGradient.setColorAt(0.6, shaded_color(color, 0));
				panelGradient.setColorAt(1.0, shaded_color(color, 10));
				painter->fillRect(edit, panelGradient);
			}
			style->drawPrimitive(QStyle::PE_FrameLineEdit, &frameOpt, painter, widget);
		}
	}

	// arrow
	if (option->subControls & (QStyle::SC_ComboBoxArrow)) {
		int ox = r.width() - 2;
		// ### work around Qt bug
	//	r.setHeight((r.height() & -2) + 0);
		if (option->direction == Qt::LeftToRight) {
			QRect ar(int(option->rect.width()) - ox + 2, 2, ox - 6, r.height());
			if (opt.state & QStyle::State_Sunken) {
				ar.adjust(1, 1, 1, 1);
			}
			opt.rect = ar;
		} else {
			QRect ar(6, 2, ox - 6, r.height());
			if (opt.state & QStyle::State_Sunken) {
				ar.adjust(-1, 1, -1, 1);
			}
			opt.rect = ar;
		}
		opt.state &= QStyle::State_Enabled;
#if 0
		((QPlastiqueStyle *) style)->QPlastiqueStyle::drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
#else
		opt.rect.adjust(-1, 0, -1, 0);
	//	painter->fillRect(opt.rect, Qt::red);
		style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
#endif
	}

	// focus frame
	if ((option->state & QStyle::State_HasFocus) && !option->editable) {
		QStyleOptionFocusRect focus;
		focus.QStyleOption::operator=(*option);
		focus.rect = style->subElementRect(QStyle::SE_ComboBoxFocusRect, option, widget);
//		focus.rect = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxEditField, widget).adjusted(-2, 0, 2, 0);
		focus.state |= QStyle::State_FocusAtBorder;
		focus.backgroundColor = option->palette.color(bgrole);
		style->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, painter, widget);
	}
}


void paintComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	QStyleOptionComboBox opt = *option;
	opt.palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
//	((QWindowsStyle *) style)->QWindowsStyle::drawControl(QStyle::CE_ComboBoxLabel, &opt, painter, widget);
	((QPlastiqueStyle *) style)->QPlastiqueStyle::drawControl(QStyle::CE_ComboBoxLabel, &opt, painter, widget);
}


/*
 * skulpture_dial.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QAbstractSlider>
#include <cmath>


/*-----------------------------------------------------------------------*/

extern void paintIndicatorDial(QPainter *painter, const QStyleOptionSlider *option);
extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole);

void paintDial(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	int d = qMin(option->rect.width(), option->rect.height());
	QStyleOptionSlider opt = *option;
	const QAbstractSlider *slider;
	// always highlight knob if pressed (even if mouse is not over knob)
	if ((option->state & QStyle::State_HasFocus) && (slider = qobject_cast<const QAbstractSlider *>(widget))) {
		if (slider->isSliderDown()) {
			opt.state |= QStyle::State_MouseOver;
		}
	}

	// tickmarks
	opt.palette.setColor(QPalette::Inactive, QPalette::WindowText, QColor(120, 120, 120, 255));
	opt.palette.setColor(QPalette::Active, QPalette::WindowText, QColor(120, 120, 120, 255));
	opt.state &= ~QStyle::State_HasFocus;
	((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_Dial, &opt, painter, widget);

	// focus rectangle
	if (option->state & QStyle::State_HasFocus) {
		QStyleOptionFocusRect focus;
		opt.state |= QStyle::State_HasFocus;
		focus.QStyleOption::operator=(opt);
		focus.rect.adjust(-1, -1, 1, 1);
		style->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, painter, widget);
	}
	opt.palette = option->palette;

	// dial base
	if (d <= 256) {
		paintIndicatorDial(painter, &opt);
	} else {
		// large dials are slow to render, do not render them
	}

	// dial knob
	d -= 6;
	int gripSize = (option->fontMetrics.height() / 4) * 2 - 1;
	opt.rect.setSize(QSize(gripSize, gripSize));
	opt.rect.moveCenter(option->rect.center());
	// angle calculation from qcommonstyle.cpp (c) Trolltech 1992-2007, ASA.
	qreal angle;
	int sliderPosition = option->upsideDown ? option->sliderPosition : (option->maximum - option->sliderPosition);
	int range = option->maximum - option->minimum;
	if (!range) {
		angle = M_PI / 2;
	} else if (option->dialWrapping) {
		angle = M_PI * 1.5 - (sliderPosition - option->minimum) * 2 * M_PI / range;
	} else {
		angle = (M_PI * 8 - (sliderPosition - option->minimum) * 10 * M_PI / range) / 6;
	}

	qreal rr = d / 2.0 - gripSize - 2;
	opt.rect.translate(int(0.5 + rr * cos(angle)), int(0.5 - rr * sin(angle)));
	paintCachedGrip(painter, &opt, option->state & QStyle::State_Enabled ? QPalette::Button : QPalette::Window);
}


/*
 * skulpture_dock.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QDockWidget>


/*-----------------------------------------------------------------------*/

void paintFrameDockWidget(QPainter *painter, const QStyleOptionFrame *option)
{
	paintThinFrame(painter, option->rect, option->palette, -60, 160);
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
}


void paintDockWidgetTitle(QPainter *painter, const QStyleOptionDockWidget *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	const QDockWidget *dock = qobject_cast<const QDockWidget *>(widget);
	bool vertical = dock && (dock->features() & QDockWidget::DockWidgetVerticalTitleBar);
	bool floating = dock && dock->isFloating();
	QRect r = option->rect;
	if (floating) {
		if (vertical) {
			r.adjust(-3, 3, 0, -3);
		} else {
			r.adjust(3, -3, -3, 0);
		}
	//	painter->fillRect(r.adjusted(1, 1, -1, -1), QColor(30, 40, 80));
	}
	QColor color = option->palette.color(QPalette::Window);
	paintThinFrame(painter, r, option->palette, 40, -20);
	paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 80);
	QLinearGradient gradient(r.topLeft(), vertical ? r.topRight() : r.bottomLeft());
	gradient.setColorAt(0.0, shaded_color(color, 50));
	gradient.setColorAt(0.2, shaded_color(color, 30));
	gradient.setColorAt(0.5, shaded_color(color, 0));
	gradient.setColorAt(0.51, shaded_color(color, -10));
	gradient.setColorAt(1.0, shaded_color(color, -20));
	painter->fillRect(r.adjusted(1, 1, -1, -1), gradient);
#if 0
	QRadialGradient dialogGradient2(r.left() + r.width() / 2, r.top(), r.height());
	dialogGradient2.setColorAt(0.0, QColor(255, 255, 255, 50));
	dialogGradient2.setColorAt(1.0, QColor(0, 0, 0, 0));
	painter->save();
	painter->translate(r.center());
	painter->scale(r.width() / 2.0 / r.height(), 1);
	painter->translate(-r.center());
	painter->fillRect(r.adjusted(1, 1, -1, -1), dialogGradient2);
	painter->restore();
#endif
	QFont font = painter->font();
	font.setBold(true);
	font.setPointSizeF(font.pointSizeF() / 1.19);
	painter->save();
	painter->setFont(font);
	r = style->subElementRect(QStyle::SE_DockWidgetTitleBarText, option, widget);
	// ### fix for Plastique centering
	if (vertical && option->rect.height() & 1) {
		if (!floating) {
			r.adjust(0, 1, 0, 1);
		} else {
			r.adjust(0, -1, 0, -1);
		}
	}
	if (floating) {
		if (vertical) {
			r.adjust(-1, 12, 3, -10);
		} else {
			r.adjust(2, 3, -3, -7);
		}
	} else {
		if (vertical) {
			r.adjust(0, 8, 4, -8);
		} else {
			r.adjust(0, 5, 0, -7);
		}
	}
	if (vertical) {
		QMatrix mat;
		QPointF c = r.center();
		mat.translate(c.x(), c.y());
		mat.rotate(-90);
		mat.translate(-c.x(), -c.y());
		r = mat.mapRect(r);
		painter->setMatrix(mat, true);
	}
//	painter->fillRect(r, Qt::red);
	painter->setClipRect(r);
	style->drawItemText(painter, r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextHideMnemonic, option->palette, true, option->title, QPalette::Text);
	painter->restore();
}


/*
 * skulpture_frames.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QAbstractSpinBox>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QPainter>
#include <cmath>
#include <cstdio>


/*-----------------------------------------------------------------------*/

static void paintThinFrame(QPainter *painter, const QRect &rect, const QBrush &brush1, const QBrush &brush2)
{
	painter->fillRect(QRect(rect.left() + 1, rect.top(), rect.width() - 1, 1), brush2);
	painter->fillRect(QRect(rect.left(), rect.top(), 1, rect.height()), brush2);
	painter->fillRect(QRect(rect.left(), rect.bottom(), rect.width() - 1, 1), brush1);
	painter->fillRect(QRect(rect.right(), rect.top(), 1, rect.height()), brush1);
}


static const QBrush shaded_brush(const QPalette &palette, int shade, QPalette::ColorRole bgrole)
{
	return (shaded_color(palette.color(bgrole), shade));
}


/*-----------------------------------------------------------------------*/
/**
 * paintThinFrame - paint a single pixel wide frame
 *
 * Paints a frame _inside_ the specified rectangle, using
 * a single pixel wide pen. The frame is rendered by darkening
 * or brightening the pixels in that area; no specific color
 * can be selected.
 *
 * dark and light specify how dark or bright the frame should
 * be rendered. They are either negative (meaning darkening),
 * or positive (meaning brigthening).
 *
 * TODO:
 * dark and light are arbitrary values; they need adjustment.
 *
 */

void paintThinFrame(QPainter *painter, const QRect &rect, const QPalette &palette, int dark, int light, QPalette::ColorRole bgrole)
{
	paintThinFrame(painter, rect, shaded_brush(palette, dark, bgrole), shaded_brush(palette, light, bgrole));
}


void paintRecessedFrame(QPainter *painter, const QRect &rect, const QPalette &palette, enum RecessedFrame rf, QPalette::ColorRole bgrole)
{
	paintThinFrame(painter, rect, palette, 30, -20, bgrole);
	paintThinFrame(painter, rect.adjusted(1, 1, -1, -1), palette, -20, -70, bgrole);
	paintRecessedFrameShadow(painter, rect.adjusted(2, 2, -2, -2), rf);
}


/*-----------------------------------------------------------------------*/

void paintFrameGroupBox(QPainter *painter, const QStyleOptionFrame *option)
{
	QRect r = option->rect;
	r.setHeight(/*r.height() +*/ 2);
	paintThinFrame(painter, r, option->palette, 60, -20);
//	paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 60);
}


void paintStyledFrame(QPainter *painter, const QStyleOptionFrame *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle */*style*/)
{
	if (option->state & QStyle::State_Sunken) {
		if (widget && widget->parentWidget() && widget->inherits("QFrame") && widget->parentWidget()->inherits("KFontRequester")) {
			paintThinFrame(painter, option->rect, option->palette, 60, -20);
			paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
			QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
			panelGradient.setColorAt(0.6, QColor(0, 0, 0, 0));
			panelGradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), 70));
			painter->fillRect(option->rect.adjusted(2, 2, -2, -2), panelGradient);
		} else {
			if (option->palette.color(QPalette::Base) == QColor(220, 230, 210)) {
				painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.color(QPalette::Base));
				paintRecessedFrame(painter, option->rect, option->palette, RF_Small);
			} else {
				RecessedFrame rf = RF_Large;
				if (!(option->state & QStyle::State_Enabled)
				 || (widget && !widget->isEnabled())) {
					rf = RF_Small;
				}
				if (widget && (widget->inherits("QAbstractItemView") || widget->inherits("Q3ScrollView"))) {
					const QList<QObject *> children = widget->children();
					foreach (QObject *child, children) {
						if (child->inherits("FrameShadow")) {
							rf = RF_None;
							break;
						}
					}
				}
				paintRecessedFrame(painter, option->rect, option->palette, rf);
			}
		}
	} else if (option->state & QStyle::State_Raised) {
		QRect r = option->rect;
		if (option->lineWidth == 0) {
			paintThinFrame(painter, r, option->palette, -20, 60);
		} else {
			paintThinFrame(painter, r, option->palette, -10, -20);
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -30, 80, bgrole);
		//	painter->fillRect(option->rect, Qt::red);
		}
	} else {
		// Plain
		if (widget && widget->parentWidget() && widget->inherits("QFrame") && widget->parentWidget()->inherits("KTitleWidget")) {
			QRect r = option->rect;
			bgrole = QPalette::Window;
//			bgrole = QPalette::Base;
			QColor bgcolor = option->palette.color(bgrole);
			painter->fillRect(r, bgcolor);
			paintThinFrame(painter, r, option->palette, -10, -20);
		//	painter->fillRect(r.adjusted(1, 1, -1, -1), QColor(200, 190, 160));
		//	painter->fillRect(r.adjusted(1, 1, -1, -1), QColor(240, 240, 240));
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -30, 80, bgrole);
			QLinearGradient gradient(r.topLeft(), r.bottomLeft());
			gradient.setColorAt(0.0, shaded_color(bgcolor, 90));
			gradient.setColorAt(0.2, shaded_color(bgcolor, 60));
			gradient.setColorAt(0.5, shaded_color(bgcolor, 0));
			gradient.setColorAt(0.51, shaded_color(bgcolor, -10));
			gradient.setColorAt(1.0, shaded_color(bgcolor, -20));
			painter->fillRect(r.adjusted(1, 1, -1, -1), gradient);
#if 0
			QRadialGradient dialogGradient2(r.left() + r.width() / 2, r.top(), r.height());
			dialogGradient2.setColorAt(0.0, QColor(255, 255, 255, 50));
			dialogGradient2.setColorAt(1.0, QColor(0, 0, 0, 0));
			painter->save();
			painter->translate(r.center());
			painter->scale(r.width() / 2.0 / r.height(), 1);
			painter->translate(-r.center());
			painter->fillRect(r.adjusted(1, 1, -1, -1), dialogGradient2);
			painter->restore();
#endif
		} else {
			QRect r = option->rect;
			paintThinFrame(painter, r, option->palette, 60, -20);
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 60, bgrole);
		}
	}
}


void paintFrameLineEdit(QPainter *painter, const QStyleOptionFrame *option)
{
	paintRecessedFrame(painter, option->rect, option->palette, RF_Small);
}


void paintPanelLineEdit(QPainter *painter, const QStyleOptionFrame *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget)
{
	bool focus = (option->state & QStyle::State_HasFocus) && !(option->state & QStyle::State_ReadOnly);
	if (option->palette.brush(bgrole).style() == Qt::SolidPattern) {
		QColor color = option->palette.color(bgrole);
//		printf("style=%d, bgrole=%d, panel color: r=%d, g=%d, b=%d, a=%d\n", option->palette.brush(bgrole).style(), bgrole, color.red(), color.green(), color.blue(), color.alpha());
		if (focus) {
#if 0
			color.setHsvF(option->palette.color(QPalette::Highlight).hueF(), 0.05, 1.0);
#else
			color = blend_color(color, option->palette.color(QPalette::Highlight), 0.15);
#endif
		} else if (option->state & QStyle::State_ReadOnly) {
		//	color = color.light(102);
	/*	} else if ((option->state & QStyle::State_Enabled)
		 && (option->state & QStyle::State_MouseOver)) {
			color = color.light(120);
	*/	}
		painter->fillRect(option->rect.adjusted(2, 2, -2, -2), color);
		if (option->state & QStyle::State_Enabled && option->rect.height() <= 64) {
			QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
			if (color.valueF() > 0.9) {
				panelGradient.setColorAt(0.0, shaded_color(color, -20));
			}
			panelGradient.setColorAt(0.6, shaded_color(color, 0));
			panelGradient.setColorAt(1.0, shaded_color(color, 10));
			painter->fillRect(option->rect.adjusted(2, 2, -2, -2), panelGradient);
		}
	}
	if (focus && option->state & QStyle::State_KeyboardFocusChange) {
		QColor color = option->palette.color(QPalette::Highlight).dark(120);
		color.setAlpha(120);
		painter->fillRect(option->rect.adjusted(4, 4 + option->rect.height() - 9, -4, -4), color);
	}
	if (option->lineWidth) {
		if (option->state & QStyle::State_ReadOnly) {
			paintThinFrame(painter, option->rect, option->palette, 60, -20);
			paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
		} else {
			paintRecessedFrame(painter, option->rect, option->palette, option->rect.height() <= 64 ? RF_Small : RF_Small);
		}
	} else if (widget && widget->parent() && (qobject_cast<QAbstractSpinBox *>(widget->parent()) || qobject_cast<QComboBox *>(widget->parent()))) {
		if (option->palette.brush(bgrole).style() != Qt::SolidPattern) {
			/* Fix Qt stylesheet demo */
			return;
		}
		int icon = 0;
		QComboBox *combo = qobject_cast<QComboBox *>(widget->parent());
		if (combo) {
			icon = widget->geometry().left();
			painter->save();
			painter->setClipRect(option->rect.adjusted(2, 0, -2, 0));
		}
		paintRecessedFrameShadow(painter, option->rect.adjusted(2 - icon, 2, -2, -2), option->rect.height() <= 64 ? RF_Small : RF_Small);
		if (combo) {
			painter->restore();
		}
	}
}


void paintFrameFocusRect(QPainter *painter, const QStyleOptionFocusRect *option)
{
	if (!(option->state & QStyle::State_KeyboardFocusChange)) {
		return;
	}
	QColor color = option->palette.color(QPalette::Highlight);
	color.setAlpha(20);
	painter->fillRect(option->rect, color);
//	painter->fillRect(option->rect.adjusted(1, 1, -1, -1), color);
	painter->fillRect(option->rect.adjusted(2, 2, -2, -2), color);
	color = color.dark(120);
	color.setAlpha(230);
	painter->fillRect(option->rect.adjusted(0, option->rect.height() - 1, 0, 0), color);
}


/*-----------------------------------------------------------------------*/
/*
 * create a gradient to draw the edge of the given path.
 *
 * the path must be convex.
 *
 * there are eight colors, one for each of the four edges
 * and the four corners.
 *
 * these are as follows:
 *
 *		c0	c1	c2
 *		c7		c3
 *		c6	c5	c4
 *
 * currently, these are created from two colors only (c0, c4).
 *
 * ### currently this function DOES NOT WORK WELL
 * because of oddities in QConicalGradient.
 *
 * maybe don't use QConicalGradient at all
 *
 */

QGradient path_edge_gradient(const QRectF &rect, const QStyleOption *option, const QPainterPath &path, const QColor &color2, const QColor &color1)
{
	QPointF c = rect.center();
	QColor color[8];
#if 1
	if (/*true ||*/ option->direction == Qt::LeftToRight) {
		color[0] = blend_color(QColor(255, 255, 255, 255), color1, 0.5);
		color[1] = color1;
		color[2] = blend_color(color1, color2, 0.5);
		color[3] = color2;
	//	color[3] = color1;
		color[4] = blend_color(QColor(0, 0, 0, 255), color2, 0.5);
		color[5] = color2;
		color[6] = blend_color(color2, color1, 0.5);
		color[7] = color1;
	//	color[7] = color2;
	} else {
		color[2] = blend_color(QColor(255, 255, 255, 255), color1, 0.5);
		color[1] = color1;
		color[0] = blend_color(color1, color2, 0.5);
		color[7] = color2;
	//	color[7] = color1;
		color[6] = blend_color(QColor(0, 0, 0, 255), color2, 0.5);
		color[5] = color2;
		color[4] = blend_color(color2, color1, 0.5);
		color[3] = color1;
	//	color[3] = color2;
	}
#else
	color[0] = QColor(255, 0, 0);
	color[2] = QColor(255, 0, 100);
	color[4] = QColor(255, 100, 0);
	color[6] = QColor(255, 100, 100);
	color[1] = QColor(0, 255, 0);
	color[3] = QColor(0, 255, 100);
	color[5] = QColor(100, 255, 0);
	color[7] = QColor(100, 255, 100);
#endif
	QConicalGradient gradient(c, 0);
#if 0 // does not work well
	QPolygonF poly = path.toFillPolygon();
	QPainterPath pp;
	pp.addPolygon(poly);
	for (int i = 0; i < pp.elementCount() - 1; ++i) {
		const QPainterPath::Element e1 = pp.elementAt(i);
		const QPainterPath::Element e2 = pp.elementAt(i + 1);
		if (e2.isLineTo()) {
		//	printf("\nat segment %d (%3g/%3g) > (%3g/%3g)\n", i, e1.x, e1.y, e2.x, e2.y);
			QColor col;
			qreal angle = atan2(e2.y - e1.y, e2.x - e1.x);
			/*
			 * angle == 0: c5
			 * angle == pi/2: c3
			 * angle == pi: c1
			 * angle == 3pi/2: c7
			 */
			const qreal e = (M_PI / 4.0);
			if (angle < 0) {
				angle += M_PI * 2.0;
			}
			if (angle >= 0*e && angle <= 1*e) {
				col = blend_color(color[5], color[4], (angle - 0*e) / e);
			} else if (angle >= 1*e && angle <= 2*e) {
				col = blend_color(color[4], color[3], (angle - 1*e) / e);
			} else if (angle >= 2*e && angle <= 3*e) {
				col = blend_color(color[3], color[2], (angle - 2*e) / e);
			} else if (angle >= 3*e && angle <= 4*e) {
				col = blend_color(color[2], color[1], (angle - 3*e) / e);
			} else if (angle >= 4*e && angle <= 5*e) {
				col = blend_color(color[1], color[0], (angle - 4*e) / e);
			} else if (angle >= 5*e && angle <= 6*e) {
				col = blend_color(color[0], color[7], (angle - 5*e) / e);
			} else if (angle >= 6*e && angle <= 7*e) {
				col = blend_color(color[7], color[6], (angle - 6*e) / e);
			} else if (angle >= 7*e && angle <= 8.00001*e) {
				col = blend_color(color[6], color[5], (angle - 7*e) / e);
			} else {
				qDebug("should not get here");
			}
		//	printf(" color is %g/%g/%g\n", col.redF() * 255, col.greenF() * 255, col.blueF() * 255);
			qreal angle1 = atan2(e1.y - c.y(), e1.x - c.x());
			qreal angle2 = atan2(e2.y - c.y(), e2.x - c.x());
			if (angle1 < 0) {
				angle1 += M_PI * 2.0;
			}
			if (angle2 < 0) {
				angle2 += M_PI * 2.0;
			}
			if (angle1 > angle2) {
				gradient.setColorAt(0.0, color[3]);
				gradient.setColorAt(angle2 / 2.0 / M_PI - 0.00001, col);
				gradient.setColorAt(angle1 / 2.0 / M_PI + 0.00001, col);
				gradient.setColorAt(1.0, color[3]);
			} else {
				// FIXME: broken QConicalGradient in 4.3.0 ?
				// this is a workaround
				if (qAbs(e1.x - e2.x) < 0.1 || qAbs(e1.y - e2.y) < 0.1) {
					gradient.setColorAt(angle1 / 2.0 / M_PI + 0.00001, col);
					gradient.setColorAt(angle2 / 2.0 / M_PI - 0.00001, col);
				} else {
				//	gradient.setColorAt((angle1 + angle2) / 4.0 / M_PI, col);
				}
			}
		//	printf(" gradient range: %3g > %3g\n angle is %3g\n", angle1 / 2.0 / M_PI, angle2 / 2.0 / M_PI, angle);
		}
	}
#else // works a BIT better for this special case
	Q_UNUSED(path);
	qreal angle;
	qreal d = 1;
	QRectF r = rect.adjusted(1, 1, -1, -1);
	{
		QRectF rect = r;
		gradient.setColorAt(0.0, color[3]);

		angle = atan2(rect.top() + d - c.y(), rect.right() - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[3]);

		angle = atan2(rect.top() - c.y(), rect.right() - d - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[5]);

		angle = atan2(rect.top() - c.y(), rect.left() + d - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[5]);

		angle = atan2(rect.top() + d - c.y(), rect.left() - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[7]);

		angle = atan2(rect.bottom() - d - c.y(), rect.left() - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[7]);

		angle = atan2(rect.bottom() - c.y(), rect.left() + d - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[1]);

		angle = atan2(rect.bottom() - c.y(), rect.right() - d - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[1]);

		angle = atan2(rect.bottom() - d - c.y(), rect.right() - c.x());
		if (angle < 0) angle += M_PI * 2.0;
		gradient.setColorAt(angle / 2.0 / M_PI, color[3]);

		gradient.setColorAt(1.0, color[3]);
	}
#endif
	return gradient;
}


/*
 * skulpture_header.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QHeaderView>


/*-----------------------------------------------------------------------*/

void paintHeaderEmptyArea(QPainter *painter, const QStyleOption *option)
{
	if (option->state & QStyle::State_Enabled) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).light(107));
	} else {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).dark(104));
	}
	if (option->state & QStyle::State_Horizontal) {
		paintThinFrame(painter, option->rect.adjusted(0, -2, 32000, -1), option->palette, -20, 60);
//		painter->fillRect(option->rect.adjusted(0, option->rect.height() - 1, 0, 0), QColor(255, 255, 255, 160));
	} else {
		paintThinFrame(painter, option->rect.adjusted(-2, 0, -1, 32000), option->palette, -20, 60);
//		painter->fillRect(option->rect.adjusted(option->rect.width() - 1, 0, 0, 0), QColor(255, 255, 255, 160));
	}
}


void paintHeaderSection(QPainter *painter, const QStyleOptionHeader *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);

	if (!(option->state & (QStyle::State_Raised | QStyle::State_Sunken))) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).dark(104));
		paintRecessedFrame(painter, option->rect.adjusted(-9, -9, 3, 3), option->palette, RF_Small);
		painter->fillRect(QRect(option->rect.right(), option->rect.bottom(), 1, 1), option->palette.color(QPalette::Window));
	} else {
		bool enabled = true;
		if (!(option->state & QStyle::State_Enabled)) {
			enabled = false;
			if (widget && widget->inherits("Q3Header")) {
				enabled = widget->isEnabled();
			}
		}
		if (enabled) {
			bool hover = false;
			const QHeaderView *view = qobject_cast<const QHeaderView *>(widget);
			if (view && (view->isClickable() || view->isMovable())) {
				hover = option->state & QStyle::State_MouseOver;
			}
		//	painter->fillRect(option->rect, option->palette.color(QPalette::Window).light(107));
			painter->fillRect(option->rect, option->palette.color(QPalette::Base).dark(hover ? 104 : (option->state & QStyle::State_On ? 120 : 106)));
		} else {
			painter->fillRect(option->rect, option->palette.color(QPalette::Window).dark(104));
		}
		if (true || !(option->state & QStyle::State_On)) {
			if (option->orientation == Qt::Horizontal) {
				paintThinFrame(painter, option->rect.adjusted(0, -2, 0, -1), option->palette, -20, 60);
			} else {
				if (option->direction == Qt::LeftToRight) {
					paintThinFrame(painter, option->rect.adjusted(-2, 0, -1, 0), option->palette, -20, 60);
				} else {
					paintThinFrame(painter, option->rect.adjusted(1, 0, 2, 0), option->palette, -20, 60);
				}
			}
		}
#if 0
		if (option->orientation == Qt::Horizontal) {
			painter->fillRect(option->rect.adjusted(0, option->rect.height() - 1, 0, 0), QColor(255, 255, 255, 160));
		} else {
			painter->fillRect(option->rect.adjusted(option->rect.width() - 1, 0, 0, 0), QColor(255, 255, 255, 160));
		}
#endif
	}
}


void paintHeaderLabel(QPainter *painter, const QStyleOptionHeader *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	painter->save();
	if (widget) {
		painter->setFont(widget->font());
	}
//	if (true || !painter->font().bold()) {
//		painter->setOpacity(painter->opacity() * 0.7);
//	}
//	QFont font(painter->font());
//	font.setPointSizeF(font.pointSizeF() / 1.19);
//	font.setBold(true);
//	painter->setFont(font);
	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_HeaderLabel, option, painter, widget);
	painter->restore();
}


void paintHeaderSortIndicator(QPainter *painter, const QStyleOptionHeader *option)
{
	int h = option->fontMetrics.height() / 2 + 2;
	int w = option->fontMetrics.height() / 4 + 2;
	QPainterPath path;

	h /= 2; w /= 2;
	if (option->sortIndicator == QStyleOptionHeader::SortDown) {
		h = -h;
	}
	path.moveTo(-w, h);
	path.lineTo(w, h);
	path.lineTo(0, -h);
	path.lineTo(-w, h);
	qreal opacity = painter->opacity();
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->translate(option->rect.center());
	painter->translate(0.5, 1.5);
	painter->setPen(Qt::NoPen);
	painter->setBrush(option->palette.color(QPalette::Text));
	painter->setOpacity(0.6 * opacity);
	painter->drawPath(path);
	painter->restore();
}

/*
 * skulpture_icons.cpp
 *
 */

#include "skulpture_p.h"
#include <QtCore/QSettings>
#include <QtGui/QStyleOption>
#include <QtGui/QDockWidget>
#include <QtGui/QFrame>
#include <QtGui/QPainter>
#include <cstdlib>
#include <cstdio>


/*-----------------------------------------------------------------------*/
#if 0 // not yet
static QStringList xdg_data_dirs()
{
	QString dirs = QString::fromLocal8Bit(getenv("XDG_DATA_DIRS"));

	if (dirs.isEmpty()) {
		dirs = QString::fromUtf8("/usr/local/share:/usr/share:/opt/kde3/share");
	}
	return dirs.split(QChar(':', 0));
}


static QStringList icon_themes()
{
	return QString::fromUtf8(
		"default.kde" "\n"
		"hicolor" "\n"
		"oxygen" "\n"
		"crystalsvg"
	).split(QChar('\n', 0));
}

#define OXYGEN "/media/hdb1/svn/kde/kdelibs/pics/oxygen"

struct IconSetting
{
	enum Size { ToolIconSize = -1 };

	const char * const label;

	int id;
	const char * const icon;
	int size;
};

static const struct IconSetting iconSettings[] =
{
 { "MessageBox/InformationIcon", QStyle::SP_MessageBoxInformation, "dialog-information", 32 },
 { "MessageBox/WarningIcon", QStyle::SP_MessageBoxWarning, "dialog-warning", 32 },
 { "MessageBox/ErrorIcon", QStyle::SP_MessageBoxWarning, "dialog-error", 32 },
 { "MessageBox/QuestionIcon", QStyle::SP_MessageBoxWarning, "KEduca", 32 },
 { 0, -1, 0, 0 },
};
#endif

QPixmap SkulptureStyle::standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const
{
#if 0
	switch (standardPixmap) {
		case SP_MessageBoxInformation:	return QPixmap(OXYGEN "/32x32/actions/dialog-information.png");
		case SP_MessageBoxWarning:		return QPixmap(OXYGEN "/32x32/actions/dialog-warning.png");
		case SP_MessageBoxCritical:		return QPixmap(OXYGEN "/32x32/actions/dialog-error.png");
		case SP_MessageBoxQuestion:		return QPixmap(OXYGEN "/32x32/apps/KEduca.png");

		case SP_DesktopIcon:			return QPixmap(OXYGEN "/16x16/actions/about-kde.png");
		case SP_TrashIcon:				return QPixmap(OXYGEN "/16x16/actions/edit-trash.png");
		case SP_ComputerIcon:			return QPixmap(OXYGEN "/16x16/devices/system.png");
		case SP_DriveFDIcon:			return QPixmap(OXYGEN "/16x16/devices/3floppy-unmount.png");
		case SP_DriveHDIcon:			return QPixmap(OXYGEN "/16x16/devices/hdd-unmount.png");
		case SP_DriveCDIcon:			return QPixmap(OXYGEN "/16x16/devices/drive-optical.png");
		case SP_DriveDVDIcon:			return QPixmap(OXYGEN "/16x16/devices/dvd-unmount.png");
		case SP_DriveNetIcon:			return QPixmap(OXYGEN "/16x16/devices/nfs-unmount.png");

		case SP_DirHomeIcon:			return QPixmap(OXYGEN "/16x16/places/folder-home.png");
		case SP_DirClosedIcon:
		case SP_DirIcon:
		case SP_DirLinkIcon:				return QPixmap(OXYGEN "/16x16/places/folder.png");
		case SP_DirOpenIcon:			return QPixmap(OXYGEN "/16x16/actions/folder-open.png");

		case SP_FileLinkIcon:
		case SP_FileIcon:				return QPixmap(OXYGEN "/16x16/mimetypes/document.png");

		case SP_FileDialogStart:			return QPixmap(OXYGEN "/16x16/actions/go-top.png");
		case SP_FileDialogEnd:			return QPixmap(OXYGEN "/16x16/actions/go-next.png");
		case SP_FileDialogToParent:		return QPixmap(OXYGEN "/16x16/actions/go-up.png");
		case SP_FileDialogNewFolder:		return QPixmap(OXYGEN "/16x16/actions/folder-new.png");
		case SP_FileDialogDetailedView:	return QPixmap(OXYGEN "/16x16/actions/fileview-detailed.png");
		case SP_FileDialogInfoView:		return QPixmap(OXYGEN "/16x16/actions/documentinfo-koffice.png");
		case SP_FileDialogContentsView:	return QPixmap(OXYGEN "/16x16/actions/file-find.png");
		case SP_FileDialogListView:		return QPixmap(OXYGEN "/16x16/actions/fileview-multicolumn.png");
		case SP_FileDialogBack:			return QPixmap(OXYGEN "/16x16/actions/go-previous.png");

		case SP_DialogOkButton:			return QPixmap(OXYGEN "/16x16/actions/dialog-ok.png");
		case SP_DialogCancelButton:		return QPixmap(OXYGEN "/16x16/actions/dialog-cancel.png");
		case SP_DialogHelpButton:		return QPixmap(OXYGEN "/16x16/actions/help-contents.png");
		case SP_DialogOpenButton:		return QPixmap(OXYGEN "/16x16/actions/document-open.png");
		case SP_DialogSaveButton:		return QPixmap(OXYGEN "/16x16/actions/document-save.png");
		case SP_DialogCloseButton:		return QPixmap(OXYGEN "/16x16/actions/dialog-close.png");
		case SP_DialogApplyButton:		return QPixmap(OXYGEN "/16x16/actions/dialog-apply.png");
		case SP_DialogResetButton:		return QPixmap(OXYGEN "/16x16/actions/edit-undo.png");
		case SP_DialogDiscardButton:		return QPixmap(OXYGEN "/16x16/actions/emptytrash.png");
		case SP_DialogYesButton:			return QPixmap(OXYGEN "/16x16/actions/dialog-apply.png");
		case SP_DialogNoButton:			return QPixmap(OXYGEN "/16x16/actions/dialog-cancel.png");

		case SP_ArrowUp:				return QPixmap(OXYGEN "/16x16/actions/arrow-up.png");
		case SP_ArrowDown:				return QPixmap(OXYGEN "/16x16/actions/arrow-down.png");
		case SP_ArrowLeft:				return QPixmap(OXYGEN "/16x16/actions/arrow-left.png");
		case SP_ArrowRight:				return QPixmap(OXYGEN "/16x16/actions/arrow-right.png");
		case SP_ArrowBack:				return QPixmap(OXYGEN "/16x16/actions/go-previous.png");
		case SP_ArrowForward:			return QPixmap(OXYGEN "/16x16/actions/go-next.png");
	}
#endif
	return ParentStyle::standardPixmap(standardPixmap, option, widget);
}


/*-----------------------------------------------------------------------*/
/**
 * decorationShape - get shape for window decoration button
 *
 * The coordinate system is -1 ... 1 for each dimension, with
 * (0, 0) being at the center, and positive coordinates pointing
 * down and to the right.
 *
 */

QPainterPath decorationShape(QStyle::StandardPixmap sp)
{
	static const qreal k = 0.3;
	static const qreal kx1 = 0.8;
	static const qreal kx2 = 0.55;
	QPainterPath path;

	switch (sp) {
		case QStyle::SP_TitleBarCloseButton:
			path.moveTo(-1, -1);
			path.lineTo(0, -k);
			path.lineTo(1, -1);
			path.lineTo(k, 0);
			path.lineTo(1, 1);
			path.lineTo(0, k);
			path.lineTo(-1, 1);
			path.lineTo(-k, 0);
			path.lineTo(-1, -1);
			path.closeSubpath();
			break;
		case QStyle::SP_TitleBarNormalButton:
		case QStyle::SP_TitleBarMaxButton:
			path.moveTo(0, -1);
			path.lineTo(1, 0);
			path.lineTo(0, 1);
			path.lineTo(-1, 0);
			path.lineTo(0, -1);
			path.moveTo(0, -kx2);
			path.lineTo(-kx1, 0);
			path.lineTo(0, kx2);
			path.lineTo(kx1, 0);
			path.lineTo(0, -kx2);
			path.closeSubpath();
			break;
		case QStyle::SP_TitleBarShadeButton:
			path.moveTo(-1, -0.4);
			path.lineTo(0, -0.6);
			path.lineTo(1, -0.4);
			path.lineTo(0, -1);
			path.lineTo(-1, -0.4);
			path.closeSubpath();
			break;
		case QStyle::SP_TitleBarUnshadeButton:
			path.moveTo(-1, -1);
			path.lineTo(0, -0.8);
			path.lineTo(1, -1);
			path.lineTo(0, -0.4);
			path.lineTo(-1, -1);
			path.closeSubpath();
			break;
		case QStyle::SP_TitleBarMinButton:
			path.moveTo(-1, 0.4);
			path.lineTo(0, 0.6);
			path.lineTo(1, 0.4);
			path.lineTo(0, 1);
			path.lineTo(-1, 0.4);
			path.closeSubpath();
			break;
		case QStyle::SP_TitleBarContextHelpButton:
			path.moveTo(0.0305, 0.513);
			path.lineTo(-0.0539, 0.513);
			path.lineTo(0.0117, 0.227);
			path.lineTo(0.22, -0.0859);
			path.lineTo(0.38, -0.323);
			path.lineTo(0.417, -0.491);
			path.lineTo(0.279, -0.767);
			path.lineTo(-0.0609, -0.87);
			path.lineTo(-0.342, -0.814);
			path.lineTo(-0.445, -0.692);
			path.lineTo(-0.383, -0.568);
			path.lineTo(-0.321, -0.456);
			path.lineTo(-0.368, -0.373);
			path.lineTo(-0.483, -0.339);
			path.lineTo(-0.64, -0.396);
			path.lineTo(-0.71, -0.555);
			path.lineTo(-0.512, -0.827);
			path.lineTo(0.0281, -0.947);
			path.lineTo(0.649, -0.783);
			path.lineTo(0.797, -0.516);
			path.lineTo(0.73, -0.31);
			path.lineTo(0.476, -0.0625);
			path.lineTo(0.111, 0.255);
			path.lineTo(0.0305, 0.513);
			path.moveTo(0.00234, 0.681);
			path.lineTo(0.165, 0.726);
			path.lineTo(0.232, 0.834);
			path.lineTo(0.164, 0.943);
			path.lineTo(0.00234, 0.988);
			path.lineTo(-0.158, 0.943);
			path.lineTo(-0.225, 0.834);
			path.lineTo(-0.158, 0.726);
			path.lineTo(0.00234, 0.681);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 1:
			path.moveTo(0, -1);
			path.lineTo(0.2, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0.2, 0.2);
			path.lineTo(0, 1);
			path.lineTo(-0.2, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(-0.2, -0.2);
			path.lineTo(0, -1);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 2:
			path.moveTo(0, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(0, -0.2);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 4:
			path.moveTo(0, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(0, -0.2);
			path.closeSubpath();
			path.moveTo(-1, -0.4);
			path.lineTo(0, -0.6);
			path.lineTo(1, -0.4);
			path.lineTo(0, -1);
			path.lineTo(-1, -0.4);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 5:
			path.moveTo(0, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(0, -0.2);
			path.closeSubpath();
			path.moveTo(-1, 0.4);
			path.lineTo(0, 0.6);
			path.lineTo(1, 0.4);
			path.lineTo(0, 1);
			path.lineTo(-1, 0.4);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 6:
			path.moveTo(0, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(0, -0.2);
			path.closeSubpath();
			path.moveTo(-1, -1);
			path.lineTo(0, -0.8);
			path.lineTo(1, -1);
			path.lineTo(0, -0.4);
			path.lineTo(-1, -1);
			path.closeSubpath();
			break;
		case QStyle::SP_CustomBase + 7:
			path.moveTo(0, -0.2);
			path.lineTo(1, 0);
			path.lineTo(0, 0.2);
			path.lineTo(-1, 0);
			path.lineTo(0, -0.2);
			path.closeSubpath();
			path.moveTo(-1, 1);
			path.lineTo(0, 0.8);
			path.lineTo(1, 1);
			path.lineTo(0, 0.4);
			path.lineTo(-1, 1);
			path.closeSubpath();
			break;
		case QStyle::SP_ToolBarHorizontalExtensionButton:
			path.moveTo(-1, -1);
			path.lineTo(0, 0);
			path.lineTo(-1, 1);
			path.lineTo(-0.5, 0);
			path.lineTo(-1, -1);
			path.closeSubpath();
			path.moveTo(0, -1);
			path.lineTo(1, 0);
			path.lineTo(0, 1);
			path.lineTo(0.5, 0);
			path.lineTo(0, -1);
			path.closeSubpath();
			break;
		case QStyle::SP_ToolBarVerticalExtensionButton:
			path.moveTo(-1, -1);
			path.lineTo(0, -0.5);
			path.lineTo(1, -1);
			path.lineTo(0, 0);
			path.lineTo(-1, -1);
			path.closeSubpath();
			path.moveTo(-1, 0);
			path.lineTo(0, 0.5);
			path.lineTo(1, 0);
			path.lineTo(0, 1);
			path.lineTo(-1, 0);
			path.closeSubpath();
			break;
		default:
			break;
	}
	return path;
}


QIcon SkulptureStyle::standardIconImplementation(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
#if 0
	QIcon icon;

	switch (standardIcon) {
		default:
			icon.addPixmap(standardPixmap(standardIcon, option, widget));
			break;
	}
	return icon;
#else
	if (standardIcon > QStyle::SP_CustomBase
	 || (standardIcon >= QStyle::SP_TitleBarMinButton && standardIcon <= QStyle::SP_TitleBarContextHelpButton)
	 || (standardIcon == QStyle::SP_ToolBarHorizontalExtensionButton)
	 || (standardIcon == QStyle::SP_ToolBarVerticalExtensionButton)
	) {
		bool dock = qobject_cast<const QDockWidget *>(widget) != 0;
		bool ext = (standardIcon == QStyle::SP_ToolBarHorizontalExtensionButton)
		 || (standardIcon == QStyle::SP_ToolBarVerticalExtensionButton);
		if (dock && standardIcon == QStyle::SP_TitleBarNormalButton) {
			standardIcon = QStyle::SP_TitleBarMaxButton;
		}
		QPainterPath path = decorationShape(standardIcon);
		const int size = ext ? 8 : (dock ? 14 : 10);
		const qreal s = size / 2.0;
		QPixmap pixmap(size, size);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.translate(s, s);
		if (dock) {
			painter.scale(s - 2, s - 2);
		} else {
			painter.scale(s, s);
		}
		painter.setPen(Qt::NoPen);
		if (option) {
			painter.setBrush(option->palette.color(QPalette::Text));
		} else {
			painter.setBrush(Qt::black);
		}
		painter.drawPath(path);
		return QIcon(pixmap);
	}
	return ParentStyle::standardIconImplementation(standardIcon, option, widget);
#endif
}


QPixmap SkulptureStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
	return ParentStyle::generatedIconPixmap(iconMode, pixmap, option);
}


QRect SkulptureStyle::itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap & pixmap) const
{
	return ParentStyle::itemPixmapRect(rectangle, alignment, pixmap);
}


void SkulptureStyle::drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const
{
	ParentStyle::drawItemPixmap(painter, rectangle, alignment, pixmap);
}


/*
 * skulpture_mdi.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QSettings>
#include <cmath>


/*-----------------------------------------------------------------------*/

void paintFrameWindow(QPainter *painter, const QStyleOptionFrame *option)
{
//	painter->fillRect(option->rect, option->palette.color(QPalette::Window));
#if 0
	paintThinFrame(painter, option->rect.adjusted(0, 0, 0, 0), option->palette, -60, 160);
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
#else
	paintThinFrame(painter, option->rect.adjusted(0, 0, 0, 0), option->palette, -90, 355);
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -40, 100);
#endif
//	paintThinFrame(painter, option->rect.adjusted(4, 7 + option->fontMetrics.height(), -4, -4), option->palette, 60, -20);
#if 0
	painter->setPen(Qt::red);
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
#endif
}


static void getTitleBarPalette(QPalette &palette)
{
	QSettings settings(QLatin1String("Trolltech"));
	settings.beginGroup(QLatin1String("Qt"));

	if (settings.contains(QLatin1String("KWinPalette/activeBackground"))) {
		palette.setColor(QPalette::Window, QColor(settings.value(QLatin1String("KWinPalette/inactiveBackground")).toString()));
		palette.setColor(QPalette::WindowText, QColor(settings.value(QLatin1String("KWinPalette/inactiveForeground")).toString()));
		palette.setColor(QPalette::Highlight, QColor(settings.value(QLatin1String("KWinPalette/activeBackground")).toString()));
		palette.setColor(QPalette::HighlightedText, QColor(settings.value(QLatin1String("KWinPalette/activeForeground")).toString()));
	} else {
		palette.setColor(QPalette::Window, QColor(0, 0, 0, 20));
		palette.setColor(QPalette::WindowText, QColor(0, 0, 0, 255));
		QColor barColor = palette.color(QPalette::Highlight);
		barColor.setHsvF(barColor.hueF(), barColor.saturationF() * 0.9, 0.25);
		palette.setColor(QPalette::Highlight, barColor);
		palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255, 240));
	}
}


void paintTitleBar(QPainter *painter, const QStyleOptionTitleBar *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	QColor barColor;
	QColor textColor;

	painter->save();
	qreal opacity = painter->opacity();

	QPalette palette = option->palette;

	if (widget && widget->inherits("QMdiSubWindow")) {
		if (widget->objectName() != QString::fromAscii("SkulpturePreviewWindow")) {
			getTitleBarPalette(palette);
		}
	}
	if (option->state & QStyle::State_Active) {
		barColor = palette.color(QPalette::Highlight);
		textColor = palette.color(QPalette::HighlightedText);
	} else {
		barColor = palette.color(QPalette::Window);
		textColor = palette.color(QPalette::WindowText);
	}

	QLinearGradient barGradient(option->rect.topLeft() + QPoint(-1, -1), option->rect.bottomLeft() + QPoint(-1, -2));
//	barGradient.setColorAt(0.0, option->palette.color(QPalette::Window));
	barGradient.setColorAt(0.0, barColor.dark(105));
//	barGradient.setColorAt(0.3, barColor);
//	barGradient.setColorAt(0.7, barColor);
	barGradient.setColorAt(1.0, barColor.light(120));
//	barGradient.setColorAt(1.0, option->palette.color(QPalette::Window));
//	painter->fillRect(option->rect.adjusted(-1, -1, 1, -2), barGradient);
//	painter->fillRect(option->rect.adjusted(-1, -1, 1, -2), barColor);

#if 1
	{
		QRect r = option->rect.adjusted(-4, -7, 4, 0);
		QRect lr = r.adjusted(6, 2, -6/* - 55*/, -1);
	//	QRect lr = r.adjusted(6, 2, -70, -1);

		if (true || option->state & QStyle::State_Active) {
			painter->fillRect(lr, barColor);
		}

		QStyleOptionTitleBar buttons = *option;
	//	buttons.subControls &= ~QStyle::SC_TitleBarLabel;
		buttons.subControls = QStyle::SC_TitleBarSysMenu;
		buttons.rect.adjust(3, -2, -4, -1);
		painter->setOpacity(option->state & QStyle::State_Active ? opacity : 0.7 * opacity);
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_TitleBar, &buttons, painter, widget);
		buttons = *option;
#if 0
		buttons.subControls &= ~(QStyle::SC_TitleBarLabel | QStyle::SC_TitleBarSysMenu);
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_TitleBar, &buttons, painter, widget);
#else
		QStyleOption opt = *option;
		QIcon icon;

		for (int i = 1; i <= 7; ++i) {
			QStyle::SubControl sc = (QStyle::SubControl) (1 << i);
			if (option->subControls & sc & ~(QStyle::SC_TitleBarContextHelpButton)) {
				QRect rect = style->subControlRect(QStyle::CC_TitleBar, option, sc, widget);
				if (option->activeSubControls & sc) {
					if (sc == QStyle::SC_TitleBarCloseButton) {
						painter->fillRect(rect, QColor(255, 0, 0, 120));
					} else {
						painter->fillRect(rect, QColor(255, 255, 255, 70));
					}
				}
				opt.palette.setColor(QPalette::Text, Qt::black);
				icon = style->standardIcon((QStyle::StandardPixmap)(QStyle::SP_TitleBarMenuButton + i), &opt, widget);
				painter->setOpacity(0.1 * opacity);
				icon.paint(painter, rect.adjusted(1, 1, 1, 1));
				opt.palette.setColor(QPalette::Text, textColor);
				icon = style->standardIcon((QStyle::StandardPixmap)(QStyle::SP_TitleBarMenuButton + i), &opt, widget);
				painter->setOpacity(option->state & QStyle::State_Active ? opacity : 0.7 * opacity);
				icon.paint(painter, rect);
			}
		}
#endif
		painter->setOpacity(opacity);

#if 0
		QRect buttonRect = option->rect.adjusted(300, 1, -90, -6);
		paintThinFrame(painter, buttonRect, option->palette, -180, 40);
		paintThinFrame(painter, buttonRect.adjusted(-1, -1, 1, 1), option->palette, 40, -180);
#endif

		{
			QLinearGradient labelGradient(lr.topLeft(), lr.bottomLeft());
#if 0
			labelGradient.setColorAt(0.0, QColor(0, 0, 0, 50));
			labelGradient.setColorAt(0.5, QColor(0, 0, 0, 0));
			labelGradient.setColorAt(0.55, QColor(0, 0, 0, 20));
			labelGradient.setColorAt(1.0, QColor(0, 0, 0, 0));
#elif 1
			labelGradient.setColorAt(0.0, QColor(255, 255, 255, 10));
			labelGradient.setColorAt(0.5, QColor(255, 255, 255, 40));
			labelGradient.setColorAt(0.55, QColor(0, 0, 0, 0));
			labelGradient.setColorAt(1.0, QColor(255, 255, 255, 20));
#else
			labelGradient.setColorAt(0.0, QColor(0, 0, 0, 30));
			labelGradient.setColorAt(1.0, QColor(255, 255, 255, 60));
#endif
			painter->fillRect(lr, labelGradient);
		}

		QLinearGradient barGradient(r.topLeft(), r.bottomLeft());
		barGradient.setColorAt(0.0, QColor(255, 255, 255, 200));
		barGradient.setColorAt(0.2, QColor(255, 255, 255, 80));
		barGradient.setColorAt(0.5, QColor(255, 255, 255, 30));
		barGradient.setColorAt(1.0, QColor(255, 255, 255, 0));
		painter->fillRect(r, barGradient);
#if 0
		QRadialGradient dialogGradient2(r.left() + r.width() / 2, r.top(), r.height());
		dialogGradient2.setColorAt(0.0, QColor(255, 255, 225, 70));
		dialogGradient2.setColorAt(1.0, QColor(0, 0, 0, 0));
		painter->save();
		painter->translate(r.center());
		painter->scale(r.width() / 2.0 / r.height(), 1);
		painter->translate(-r.center());
		painter->fillRect(r.adjusted(1, 1, -1, -1), dialogGradient2);
		painter->restore();
#endif
		paintThinFrame(painter, lr, option->palette, -30, 90);
		paintThinFrame(painter, lr.adjusted(-1, -1, 1, 1), option->palette, 90, -30);

	}
#endif


#if 0
//	paintThinFrame(painter, option->rect.adjusted(0, 0, 0, -1), option->palette, -30, 80);
	paintThinFrame(painter, option->rect.adjusted(-1, -1, 1, 0), option->palette, 80, -30);
	painter->fillRect(option->rect.adjusted(0, 0, 0, -1), barColor);

	// FIXME: adjust rect for new shadow // paintRecessedFrameShadow(painter, option->rect.adjusted(-1, -1, 1, 0), RF_Large);
	{
		QRect labelRect = option->rect.adjusted(20, 0, -250, 0);
		painter->fillRect(labelRect, option->palette.color(QPalette::Window));
		paintThinFrame(painter, labelRect.adjusted(0, 0, 0, 1), option->palette, -30, 80);
	}
#endif

	if (option->subControls & QStyle::SC_TitleBarLabel) {
		QRect labelRect;

		if (widget && widget->inherits("QMdiSubWindow")) {
			QFont font = painter->font();
			font.setBold(true);
			labelRect = option->rect.adjusted(option->fontMetrics.height() + 10, -1, -2, -3);
		//	font.setPointSizeF(10);
			painter->setFont(font);
		} else {
			labelRect = style->subControlRect(QStyle::CC_TitleBar, option, QStyle::SC_TitleBarLabel, widget);
		}
		painter->setOpacity(opacity * 0.1);
		painter->setPen(Qt::black);
		painter->drawText(labelRect.adjusted(1, 1, 1, 1), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->text);
		painter->setOpacity(option->state & QStyle::State_Active ? opacity : 0.7 * opacity);
		painter->setPen(textColor);
		painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->text);
	}
/*
	if (!option->icon.isNull()) {
		labelRect.setWidth(16);
		labelRect.setHeight(16);
		painter->drawPixmap(labelRect.adjusted(0, -1, 0, -1), option->icon.pixmap(QSize(16, 16)));
	}
*/	painter->restore();
}


int getWindowFrameMask(QStyleHintReturnMask *mask, const QStyleOptionTitleBar *option, const QWidget *widget)
{
	Q_UNUSED(widget);
	mask->region = option->rect;

	// TODO get total dimensions of workspace and don't use masks on corners
	/*if (option->rect.topLeft() != QPoint(0, 0))*/ {
//		mask->region -= QRect(option->rect.topLeft(), QSize(1, 1));
	}
//	mask->region -= QRect(option->rect.topRight(), QSize(1, 1));
//	mask->region -= QRect(option->rect.bottomLeft(), QSize(1, 1));
//	mask->region -= QRect(option->rect.bottomRight(), QSize(1, 1));

	// try new style
//	mask->region -= QRect(option->rect.topLeft(), QSize(6, 1));
//	mask->region -= QRect(option->rect.topRight() - QPoint(5, 0), QSize(6, 1));
	return 1;
}


/*
 * skulpture_menu.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QShortcut>
#include <cstdio>


/*-----------------------------------------------------------------------*/

void paintFrameMenu(QPainter *painter, const QStyleOptionFrame *option)
{
	paintThinFrame(painter, option->rect, option->palette, -60, 160);
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
}


void paintPanelMenuBar(QPainter *painter, const QStyleOptionFrame *option)
{
	Q_UNUSED(painter); Q_UNUSED(option);
//	paintThinFrame(painter, option->rect, option->palette, -20, 60);
//	painter->fillRect(option->rect.adjusted(1, 1, -1, -1), option->palette.color(QPalette::Window));
}


void paintMenuBarEmptyArea(QPainter *painter, const QStyleOption *option)
{
	Q_UNUSED(painter); Q_UNUSED(option);
//	painter->fillRect(option->rect, option->palette.color(QPalette::Window));
}


/*-----------------------------------------------------------------------*/
#if 0
QSize sizeFromContentsMenuBarItem(const QStyleOptionMenuItem *option, const QSize &contentsSize)
{
}
#endif

extern void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole);

void paintMenuBarItem(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
#if 0
	if (option->state & QStyle::State_Selected) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Highlight));
		paintThinFrame(painter, option->rect, option->palette, -10, 20);
	}
#else
	QStyleOptionButton button;

	button.QStyleOption::operator=(*option);
	button.features = QStyleOptionButton::None;
	button.rect.adjust(-1, -1, 1, 1);
//	button.rect.adjust(0, -1, 0, -1);
	if (option->state & QStyle::State_Selected || option->state & QStyle::State_MouseOver) {
		button.state |= QStyle::State_MouseOver;
		paintCommandButtonPanel(painter, &button, QPalette::Button);
	}
#endif
	int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
	if (!style->styleHint(QStyle::SH_UnderlineShortcut, option, widget)) {
		alignment |= Qt::TextHideMnemonic;
	}
#if 0
	painter->save();
	painter->setPen(option->palette.color(option->state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
	style->drawItemText(painter, option->rect, alignment, option->palette, option->state & QStyle::State_Enabled, option->text, QPalette::NoRole);
	painter->restore();
#else
	QRect r = option->rect;
//	int x = option->state & QStyle::State_Sunken ? style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, option, widget) : 0;
//	int y = option->state & QStyle::State_Sunken ? style->pixelMetric(QStyle::PM_ButtonShiftVertical, option, widget) : 0;
//	r.adjust(x, y, x, y);
//	r.adjust(0, -1, 0, -1);
	// FIXME support icon
	style->drawItemText(painter, r, alignment, option->palette, option->state & QStyle::State_Enabled, option->text, QPalette::ButtonText);
#endif
//	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_MenuBarItem, option, painter, widget);
}


/*-----------------------------------------------------------------------*/
/*
 * this menu code work arounds some limitations of Qt menus.
 * new features are:
 *
 * - text column and shortcut column can overlap
 * - shortcut column can be hidden
 * - shortcut column can have a different font
 * - shortcut column can have actual keyboard glyphs
 * - shortcut keys can have frames
 * - the + connector for keys can be hidden
 *
 * keyboard glyphs are used for: Shift, Arrows, Return
 * german keyboards also have: "Bild <arrow up>", "Bild <arrow down>"
 * keyboards in other locales may have other glyphs, so
 * keep it configurable.
 *
 *
 * We must entirely replace Qt's layout algorithm.
 *
 */


/*-----------------------------------------------------------------------*/
/**
 * sizeFromContentsMenu
 *
 * following information is passed in option:
 *
 *
 */

QSize sizeFromContentsMenu(const QStyleOption *option, const QSize &contentsSize, const QWidget *widget, const QStyle */*style*/)
{
	Q_UNUSED(option); Q_UNUSED(widget);
	return contentsSize;
}


/*-----------------------------------------------------------------------*/
/**
 * sizeFromContentsMenuItem
 *
 * following information is passed in option:
 *
 *
 */

QSize sizeFromContentsMenuItem(const QStyleOptionMenuItem *option, const QSize &contentsSize)
{
	if (option->menuItemType == QStyleOptionMenuItem::Separator) {
		return QSize(4, 4);
	}

	// always make room for icon column
	int iconWidth = 16 + 4;
	if (option->maxIconWidth > iconWidth) {
		iconWidth = option->maxIconWidth;
	}

	QFont menu_font;

	int w = contentsSize.width(), h = contentsSize.height();
	w += iconWidth + 4 + 22;
	h += 4;
	return QSize(w, h);
}


/*-----------------------------------------------------------------------*/

void paintMenuItem(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	const bool buttonmode = true;

	if (qobject_cast<const QMenu *>(widget)) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Active, buttonmode ? QPalette::Window : QPalette::Base));
		QRect rect = option->rect;
		if (option->direction == Qt::RightToLeft) {
			rect.translate(rect.width() - 25, 0);
		}
		rect.setWidth(24/*option->maxIconWidth*/);
#if 0
		painter->fillRect(rect.adjusted(0, 0, -1, 0), option->palette.color(QPalette::Window).darker(107));
#else
		QLinearGradient gradient(rect.topLeft(), rect.topRight());
		gradient.setColorAt(0.0, QColor(0, 0, 0, 10));
		gradient.setColorAt(1.0, QColor(0, 0, 0, 10));
		if (buttonmode) {
			painter->fillRect(rect.adjusted(0, 0, -1, 0), gradient);
		} else {
			painter->fillRect(rect.adjusted(0, 0, -1, 0), option->palette.color(QPalette::Active, QPalette::Window));
		}
#endif
	}
//	painter->fillRect(option->rect.adjusted(option->maxIconWidth, 0, -1, 0), option->palette.color(QPalette::Window).lighter(102));
	if (option->state & QStyle::State_Selected) {
		if (buttonmode) {
			QStyleOptionButton button;

			button.QStyleOption::operator=(*option);
			button.features = QStyleOptionButton::None;
			button.state |= QStyle::State_MouseOver;
			button.rect.adjust(-1, -1, 0, 1);
			if (option->state & QStyle::State_Selected || option->state & QStyle::State_MouseOver) {
				paintCommandButtonPanel(painter, &button, QPalette::Button);
			}
		} else {
			QColor color = option->palette.color(QPalette::Active, QPalette::Highlight);
			if (!(option->state & QStyle::State_Enabled)) {
			//	color = blend_color(color, option->palette.color(QPalette::Window), 0.9);
				color.setAlpha(40);
			} else {
				color.setAlpha(180);
			}
			painter->fillRect(option->rect.adjusted(0, 0, -1, 0), color);
			if (option->state & QStyle::State_Enabled) {
				paintThinFrame(painter, option->rect.adjusted(0, 0, -1, 0), option->palette, -20, -20);
			}
		}
	} else {
		/* */
	}
	if (option->menuItemType == QStyleOptionMenuItem::Separator) {
		if (qobject_cast<const QMenu *>(widget)) {
		//	QRect rect = option->rect;
		//	rect.setWidth(22/*option->maxIconWidth*/);
		//	painter->fillRect(rect.adjusted(0, 0, -1, 0), option->palette.color(QPalette::Window).darker(110));
			if (option->direction == Qt::LeftToRight) {
				paintThinFrame(painter, option->rect.adjusted(23, 1, 3, -1), option->palette, 60, -20);
			} else {
				paintThinFrame(painter, option->rect.adjusted(0, 1, -25, -1), option->palette, 60, -20);
			}
		//	painter->fillRect(option->rect, Qt::red);
		} else {
			QRect rect = option->rect;
			rect.setHeight(2);
			rect.translate(0, (option->rect.height() - 2) >> 1);
			paintThinFrame(painter, rect, option->palette, 60, -20);
		}
		return;
	}
//	painter->save();
//	QFont font = painter->font();
//	font.setPointSizeF(font.pointSizeF() / (1.19));
//	font.setBold(true);
//	painter->setFont(font);
	QStyleOptionMenuItem menuOption = *option;
	if (buttonmode) {
		menuOption.palette.setColor(QPalette::HighlightedText, option->palette.color(QPalette::ButtonText));
		menuOption.palette.setColor(QPalette::ButtonText, option->palette.color(QPalette::WindowText));
	} else {
		menuOption.palette.setColor(QPalette::ButtonText, option->palette.color(QPalette::Text));
	}
	menuOption.palette.setColor(QPalette::Highlight, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Button, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Window, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Shadow, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Light, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Midlight, QColor(0, 0, 0, 0));
	menuOption.palette.setColor(QPalette::Dark, QColor(0, 0, 0, 0));
#if 0
	int pos;
	if ((pos = option->text.indexOf(QChar('\t', 0))) > 0) {
		QString shortcut = option->text.mid(pos + 1);
	//	shortcut = shortcut.replace(QString::fromUtf8("Strg+"), QChar('^', 0));
		shortcut = shortcut.left(shortcut.length() - 1).replace(QChar('+', 0), QChar(' ', 0)) + shortcut.right(1);
#if 0
		shortcut = shortcut.replace(QShortcut::tr("Left"), QChar(0x2190));
		shortcut = shortcut.replace(QShortcut::tr("Up"), QChar(0x2191));
		shortcut = shortcut.replace(QShortcut::tr("Right"), QChar(0x2192));
		shortcut = shortcut.replace(QShortcut::tr("Down"), QChar(0x2193));
		shortcut = shortcut.replace(QShortcut::tr("Shift"), QChar(0x21e7));
//		shortcut = shortcut.replace(QShortcut::tr("Ctrl"), QChar(0x2318));
		// german keyboard shows Arrows for PgUp / PgDown
		shortcut = shortcut.replace(QString::fromUtf8(" aufw\xc3\xa4rts"), QChar(0x2191));
		shortcut = shortcut.replace(QString::fromUtf8(" abw\xc3\xa4rts"), QChar(0x2193));
#endif
		menuOption.text = option->text.left(pos) + QChar('\t', 0) + shortcut;
	}
#endif
	if (menuOption.maxIconWidth < 16 + 4) {
		menuOption.maxIconWidth = 16 + 3;
		if (option->direction == Qt::LeftToRight) {
			menuOption.rect.adjust(2, 0, 2, 0);
		} else {
			menuOption.rect.adjust(-4, 0, -4, 0);
		}
	} else {
		if (option->direction == Qt::LeftToRight) {
			menuOption.rect.adjust(1, 0, 1, 0);
		} else {
			menuOption.rect.adjust(-3, 0, -3, 0);
		}
	}
	if (menuOption.checked) {
		menuOption.icon = QIcon();
	}
	// ### hack to reduce size of arrow
	QFont font;
	font.setPointSizeF(font.pointSizeF() / 1.19);
	menuOption.fontMetrics = QFontMetrics(font);
	((QWindowsStyle *) style)->QWindowsStyle::drawControl(QStyle::CE_MenuItem, &menuOption, painter, widget);
//	painter->restore();

	/* arrows: left up right down: 0x2190-0x2193
	 * shift?: 0x21E7
	 * command: 0x2318
	 * return: 0x23CE, 0x21B5
	 * backspace: 0x232B
	 * (scrollbar arrows: left up right down: 0x25C4,B2,BA,BC)
	 * dot: 0x25CF
	 * other arrow right: 0x2799
	 */
}


/*
 * skulpture_metrics.cpp
 *
 */

#include "skulpture_p.h"
#include <QtCore/QSettings>
#include <QtGui/QApplication>
#include <cstdio>


/*-----------------------------------------------------------------------*/
#if 0 // not yet
struct MetricSetting {
	const char * const label;
	int id;
	int value;
	unsigned char flags;
	unsigned char min; short max;
};


static const struct MetricSetting metricSettings[] =
{
 { "MDI/TitleBar/Height", QStyle::PM_TitleBarHeight, 25 },
 { "Button/Margin", QStyle::PM_ButtonMargin, 8 },
 { 0, -1, 0 }
};
#endif

static inline int fontSize(const QStyleOption *option, const QWidget *widget)
{
	if (option) {
		return option->fontMetrics.height();// - 1;
	} else if (widget) {
		return widget->fontMetrics().height();// - 1;
	}
	return 16;
}


static inline int appFontSize(const QStyleOption *option, const QWidget *widget)
{
	Q_UNUSED(option); Q_UNUSED(widget);
	return QApplication::fontMetrics().height();
}


static inline int iconSize(int fontSize)
{
	int iconSize;

	if (fontSize < 22) {
		iconSize = 16;
	} else if (fontSize < 32) {
		iconSize = 22;
	} else {
		iconSize = 32;
	}
	return iconSize;
}


static inline Qt::LayoutDirection direction(const QStyleOption *option, const QWidget *widget)
{
	if (option) {
		return option->direction;
	} else if (widget) {
		return widget->layoutDirection();
	}
	return Qt::LeftToRight;
}


/*-----------------------------------------------------------------------*/

int SkulptureStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
//	return ParentStyle::pixelMetric(metric, option, widget);
	switch (metric) {
	/* Window */
		case PM_TitleBarHeight: {
				if (option) {
#if 0
					printf("fontleading: %d\n", option->fontMetrics.leading());
					printf("fontlinespacing: %d\n", option->fontMetrics.lineSpacing());
					printf("fontheight: %d\n", option->fontMetrics.height());
					printf("xheight: %d\n", option->fontMetrics.xHeight());
					printf("x-bheight: %d\n", option->fontMetrics.boundingRect(QChar('x', 0)).height());
					printf("X-bheight: %d\n", option->fontMetrics.boundingRect(QChar('X', 0)).height());
					printf("g-bheight: %d\n", option->fontMetrics.boundingRect(QChar('g', 0)).height());
					printf("§-bheight: %d\n", option->fontMetrics.boundingRect(QString::fromUtf8("§").at(0)).height());
					printf("Ä-bheight: %d\n", option->fontMetrics.boundingRect(QString::fromUtf8("Ä").at(0)).height());
#endif
					return 3 + fontSize(option, widget);
				} else {
					return 21;
				}
			}
			break;
		case PM_MDIFrameWidth:			return 3;
		/* MenuBar & Menu */
		case PM_MenuBarVMargin:		return 1;
		case PM_MenuBarHMargin:		return 1;
		case PM_MenuBarPanelWidth:		return 0;
		case PM_MenuBarItemSpacing:		return 0;
		// ### anything other than 0 f*cks up Qt's menu positioning code ...
		case PM_MenuVMargin:			return 0;
		case PM_MenuHMargin:			return 0;
		case PM_MenuPanelWidth:		return 1;
		/* ToolBar & ToolButton */
		case PM_ToolBarSeparatorExtent:	return 6;
		case PM_ToolBarHandleExtent:		return 9; //fontSize(option, widget) / 2;
		case PM_ToolBarFrameWidth:		return 1;
		case PM_ToolBarItemMargin:		return 0;
		case PM_ToolBarIconSize:			return iconSize(fontSize(option, widget));
		case PM_ToolBarItemSpacing:		return 0;
		case PM_ToolBarExtensionExtent:	return 12;
		/* DockWidget */
		case PM_DockWidgetSeparatorExtent:	return (fontSize(option, widget) / 4) * 2 - 1;
		case PM_DockWidgetHandleExtent:	return 160;	// TODO
		case PM_DockWidgetFrameWidth:	return 2;
		case PM_DockWidgetTitleMargin:	return 2;
		case PM_DockWidgetTitleBarButtonMargin:
									return 0;
		/* ItemView */
		case PM_HeaderGripMargin:		return 4;
		case PM_HeaderMargin:			return 3;
		case PM_HeaderMarkSize:			return 5;
	/* Controls */
		/* Button */
		case PM_ButtonDefaultIndicator:	return 0;
		case PM_ButtonShiftHorizontal:		return direction(option, widget) == Qt::LeftToRight ? 1 : -1;
		case PM_ButtonShiftVertical:		return 1;
		case PM_ButtonMargin:			return fontSize(option, widget) / 2 - 4;
		case PM_MenuButtonIndicator:		return fontSize(option, widget);
		case PM_ExclusiveIndicatorWidth:
		case PM_ExclusiveIndicatorHeight:
		case PM_IndicatorWidth:
		case PM_IndicatorHeight:			return fontSize(option, widget);// * 6 / 5;
		case PM_CheckBoxLabelSpacing:	return fontSize(option, widget) / 3;
		case PM_RadioButtonLabelSpacing:	return fontSize(option, widget) / 3;
		/* Slider */
		case PM_SliderSpaceAvailable:
			return ((QCommonStyle *) this)->QCommonStyle::pixelMetric(metric, option, widget);
		case PM_SliderThickness:
		case PM_SliderControlThickness:	return (fontSize(option, widget) / 4) * 2 + 9 + 4;
		case PM_SliderLength:			return 2 * fontSize(option, widget) - 4;
		case PM_SliderTickmarkOffset: {
			if (const QStyleOptionSlider *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
				if (sliderOption->tickPosition == QSlider::TicksAbove) {
					return 5;
				} else if (sliderOption->tickPosition == QSlider::TicksBothSides) {
					return 5;
				} else if (sliderOption->tickPosition == QSlider::TicksBelow) {
					return 0;
				}
			}
			return 0;
		}
		/* ScrollBar */
		case PM_ScrollBarExtent:			return (appFontSize(option, widget) / 4) * 2 + 9;
		case PM_ScrollBarSliderMin:		return 2 * appFontSize(option, widget) + 4;
		/* ComboBox */
		case PM_ComboBoxFrameWidth:	return 2;
		/* SpinBox & QDial */
		case PM_SpinBoxFrameWidth:		return 2;
	/* Misc */
		/* GroupBox */
		/* TabBar & TabWidget */
		case PM_TabBarIconSize:			return iconSize(fontSize(option, widget));
		case PM_TabBarTabHSpace:		return 0;
		case PM_TabBarTabVSpace:		return 0;
		case PM_TabBarBaseOverlap:		return 2;
		case PM_TabBarScrollButtonWidth:	return 2 * (fontSize(option, widget) / 2) + 1;
		case PM_TabBar_ScrollButtonOverlap:	return 0;
		case PM_TabBarTabShiftVertical:	return 0;
		case PM_TabBarTabShiftHorizontal:	return 0;
//		case PM_TabBarBaseHeight:		return 22;
// ???	case PM_TabBarTabOverlap:		return 4;
		/* ProgressBar */
		/* Other */
		case PM_SmallIconSize:			return iconSize(fontSize(option, widget));
		case PM_SplitterWidth:			return (fontSize(option, widget) / 4) * 2 + 1;
		/* Layout */
		case PM_LayoutHorizontalSpacing:
		case PM_LayoutVerticalSpacing:
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
			return -1;
#else
			return fontSize(option, widget) / 3;
#endif

		case PM_DefaultChildMargin:		return fontSize(option, widget) / 2;
		case PM_DefaultTopLevelMargin:	return 1 + fontSize(option, widget) * 5 / 8;

		case PM_LayoutLeftMargin:
		case PM_LayoutTopMargin:
		case PM_LayoutRightMargin:
		case PM_LayoutBottomMargin:
			return pixelMetric((option && (option->state & State_Window)) || (widget && widget->isWindow()) ? PM_DefaultTopLevelMargin : PM_DefaultChildMargin, option, widget);
		case PM_DefaultFrameWidth:		return 2;
		case PM_TextCursorWidth:			return qMax(1, (fontSize(option, widget) + 8) / 12);
		default:
			return ParentStyle::pixelMetric(metric, option, widget);
	}
}


/*-----------------------------------------------------------------------*/
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
int SkulptureStyle::layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
#if 0
	return ParentStyle::layoutSpacingImplementation(control1, control2, orientation, option, widget);
#else
	if (orientation == Qt::Vertical && (control1 == QSizePolicy::CheckBox || control1 ==QSizePolicy::RadioButton || control2 ==QSizePolicy::CheckBox || control2 ==QSizePolicy::RadioButton)) {
		return fontSize(option, widget) / 3;
	}
	int space = ParentStyle::layoutSpacingImplementation(control1, control2, orientation, option, widget);
	if (space <= 0) return space;
	return int(0.5 + fontSize(option, widget) / 16.0 * ((space > 6 ? space - 2 : space) & -2));
#endif
}
#endif

/*
 * skulpture_misc.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QTableView>


/*-----------------------------------------------------------------------*/

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void paintPanelItemViewItem(QPainter *painter, const QStyleOptionViewItemV4 *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);

	QColor color = option->palette.color(QPalette::Highlight);
	bool mouse = option->state & QStyle::State_MouseOver && option->state & QStyle::State_Enabled;

	if (option->version >= 2 && option->features & QStyleOptionViewItemV2::Alternate) {
		painter->fillRect(option->rect, option->palette.color(QPalette::AlternateBase));
	} else {
		painter->fillRect(option->rect, option->backgroundBrush);
	}
	if (option->state & QStyle::State_Selected) {
		if (mouse) {
			color = color.lighter(110);
		}
	} else if (mouse) {
		color.setAlpha(40);
	} else {
		return;
	}
	painter->save();
	// ### work around KDE widgets that turn on antialiasing
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setBrush(Qt::NoBrush);
//	QColor shine(255, 255, 255, option->rect.height() > 20 ? 25 : 10);
	QColor shadow(0, 0, 0, option->rect.height() > 20 ? 50 : 20);
	painter->setPen(shadow);
//	painter->setPen(QPen(color.darker(option->rect.height() > 20 ? 150 : 120), 1));
	painter->fillRect(option->rect, color);
	const QTableView *table = qobject_cast<const QTableView *>(widget);
#if 0
	if (!widget) {
		if (widget->objectName() == QLatin1String("qt_scrollarea_viewport")) {
			if (widget->parentWidget()) {
				table = qobject_cast<const QTableView *>(widget->parentWidget());
			}
		}
	}
#endif
	if (table && table->showGrid()) {
		painter->restore();
		return;
	}
	if (option->version >= 4) {
#if 0
		// ### Qt doesn't initialize option->widget, so do not use it
		if (qobject_cast<const QTableView *>(option->widget)) {
			painter->restore();
			return;
		}
#endif
		switch (option->viewItemPosition) {
			case QStyleOptionViewItemV4::Beginning:
				painter->drawLine(option->rect.topLeft() + QPoint(0, 1), option->rect.bottomLeft() - QPoint(0, 1));
				painter->drawLine(option->rect.topLeft(), option->rect.topRight());
				painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
				break;
			case QStyleOptionViewItemV4::End:
				painter->drawLine(option->rect.topRight() + QPoint(0, 1), option->rect.bottomRight() - QPoint(0, 1));
				painter->drawLine(option->rect.topLeft(), option->rect.topRight());
				painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
				break;
			case QStyleOptionViewItemV4::Middle:
				painter->drawLine(option->rect.topLeft(), option->rect.topRight());
				painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
				break;
			case QStyleOptionViewItemV4::Invalid:
			case QStyleOptionViewItemV4::OnlyOne:
				painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
				break;
		}
	} else {
		painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
	}
	painter->restore();
}
#endif

/*-----------------------------------------------------------------------*/

extern void paintCachedIndicatorBranchChildren(QPainter *painter, const QStyleOption *option);

void paintIndicatorBranch(QPainter *painter, const QStyleOption *option)
{
	QPoint center = option->rect.center() + (option->direction == Qt::LeftToRight ? QPoint(2, 0) : QPoint(-1, 0));

	if (option->state & (QStyle::State_Item | QStyle::State_Sibling)) {
		QColor lineColor = option->palette.color(QPalette::Text);
		lineColor.setAlpha(50);
		painter->fillRect(QRect(center.x(), option->rect.y(), 1, center.y() - option->rect.y()), lineColor);
		if (option->state & QStyle::State_Sibling) {
			painter->fillRect(QRect(center.x(), center.y(), 1, option->rect.bottom() - center.y() + 1), lineColor);
		}
		if (option->state & QStyle::State_Item) {
			if (option->direction == Qt::RightToLeft) {
				painter->fillRect(QRect(option->rect.left(), center.y(), center.x() - option->rect.left(), 1), lineColor);
			} else {
				painter->fillRect(QRect(center.x() + 1, center.y(), option->rect.right() - center.x(), 1), lineColor);
			}
			if (!(option->state & QStyle::State_Sibling)) {
				lineColor.setAlpha(25);
				painter->fillRect(QRect(center.x(), center.y(), 1, 1), lineColor);
			}
		}
	}
	if (option->state & QStyle::State_Children && !(option->state & QStyle::State_Open)) {
		QStyleOption opt = *option;
		static const int d = 5;
		opt.rect = QRect(center.x() - d / 2, center.y() - d / 2, d, d);
		paintCachedIndicatorBranchChildren(painter, &opt);
	}
}


/*-----------------------------------------------------------------------*/

void paintSizeGrip(QPainter *painter, const QStyleOptionSizeGrip *option)
{
	QRect r;

	switch (option->corner) {
		case Qt::TopLeftCorner:		r = option->rect.adjusted(0, 0, 2, 2);	break;
		case Qt::TopRightCorner:		r = option->rect.adjusted(-2, 0, 0, 2);	break;
		case Qt::BottomLeftCorner:	r = option->rect.adjusted(0, -2, 2, 0);	break;
		case Qt::BottomRightCorner:	r = option->rect.adjusted(-2, -2, 0, 0);	break;
	}
	paintThinFrame(painter, r, option->palette, 60, -20);
	paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 60);
	switch (option->corner) {
		case Qt::TopRightCorner:	// for Kickoff 4.1
			painter->save();
			painter->setPen(QPen(shaded_color(option->palette.color(QPalette::Window), 60), 1.0));
			painter->drawLine(r.topLeft(), r.bottomRight());
			painter->setPen(QPen(shaded_color(option->palette.color(QPalette::Window), -20), 1.0));
			painter->drawLine(r.topLeft() + QPoint(1, -1), r.bottomRight() + QPoint(1, -1));
			painter->restore();
			break;
		case Qt::BottomRightCorner:
			painter->save();
			painter->setPen(QPen(shaded_color(option->palette.color(QPalette::Window), -20), 1.0));
			painter->drawLine(r.topRight(), r.bottomLeft());
			painter->setPen(QPen(shaded_color(option->palette.color(QPalette::Window), 60), 1.0));
			painter->drawLine(r.topRight() + QPoint(1, 1), r.bottomLeft() + QPoint(1, 1));
			painter->restore();
			break;
		case Qt::TopLeftCorner:
			// TODO
			break;
		case Qt::BottomLeftCorner:
			// TODO
			break;
	}
}


/*-----------------------------------------------------------------------*/

extern void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole);

void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBoxV2 *option)
{
	QRect r = option->rect;
	if (option->state & QStyle::State_Selected) {
		QColor color = option->palette.color(QPalette::Window);
		paintThinFrame(painter, r, option->palette, 40, -20);
		paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 80);
		QLinearGradient gradient(r.topLeft(), r.bottomLeft());
		gradient.setColorAt(0.0, shaded_color(color, 50));
		gradient.setColorAt(0.2, shaded_color(color, 30));
		gradient.setColorAt(0.5, shaded_color(color, 0));
		gradient.setColorAt(0.51, shaded_color(color, -10));
		gradient.setColorAt(1.0, shaded_color(color, -20));
		painter->fillRect(r.adjusted(1, 1, -1, -1), gradient);
	} else if (option->state & (QStyle::State_Sunken | QStyle::State_MouseOver)) {
		QStyleOptionButton button;
		button.QStyleOption::operator=(*option);
		button.features = QStyleOptionButton::None;
	//	button.rect.adjust(-1, -1, 1, 1);
		paintCommandButtonPanel(painter, &button, QPalette::Window);
	} else if (option->version >= 2 && option->selectedPosition == QStyleOptionToolBoxV2::PreviousIsSelected) {
		r.setHeight(2);
		paintThinFrame(painter, r, option->palette, 60, -20);
	} else {
	//	r.setHeight(2);
	//	painter->fillRect(r, option->palette.color(QPalette::Window));
	}
	QStyleOption indicator;
	indicator = *option;
	indicator.rect.setSize(QSize(11, 11));
	indicator.rect.translate(0, (option->rect.height() - 11) >> 1);
	indicator.state = QStyle::State_Children;
	if (option->state & QStyle::State_Selected) {
		indicator.state |= QStyle::State_Open;
	}
	paintIndicatorBranch(painter, &indicator);
}


/*-----------------------------------------------------------------------*/

extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole);

void paintSplitter(QPainter *painter, const QStyleOption *option)
{
	if (option->state & QStyle::State_Enabled && option->state & QStyle::State_MouseOver) {
		painter->fillRect(option->rect, QColor(255, 255, 255, 60));
	}
	int d = 5;
	QRect rect(QRect(option->rect).center() - QPoint(d / 2, d / 2), QSize(d, d));
	QStyleOption iOption = *option;
	iOption.rect = rect;
	iOption.palette.setCurrentColorGroup(QPalette::Disabled);
//	iOption.state &= ~QStyle::State_Enabled;
	iOption.palette.setColor(QPalette::Button, iOption.palette.color(QPalette::Window));
	paintCachedGrip(painter, &iOption, QPalette::Window);
}


/*-----------------------------------------------------------------------*/

void paintRubberBand(QPainter *painter, const QStyleOptionRubberBand *option)
{
	painter->save();
	if (true || option->shape == QRubberBand::Rectangle) {
		qreal opacity = painter->opacity();
		painter->setOpacity(opacity * 0.2);
		painter->fillRect(option->rect, option->palette.color(QPalette::Highlight));
		painter->setOpacity(opacity * 0.8);
		painter->setPen(QPen(option->palette.color(QPalette::Highlight) /*, 1.0, Qt::DotLine*/));
		painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
		painter->setOpacity(opacity);
	} else {
	//	painter->fillRect(option->rect, Qt::green);
	}
	painter->restore();
}


int getRubberBandMask(QStyleHintReturnMask *mask, const QStyleOption *option, const QWidget *widget)
{
	static const int rubber_width = 4;
	int r = rubber_width;

	Q_UNUSED(widget);
	mask->region = option->rect;
	if (option->rect.width() > 2 * r && option->rect.height() > 2 * r) {
		mask->region -= option->rect.adjusted(r, r, -r, -r);
	}
	return 1;
}


/*
 * skulpture_progressbar.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QProgressBar>
#include <QtCore/QTime>


/*-----------------------------------------------------------------------*/

void paintProgressBarGroove(QPainter *painter, const QStyleOptionProgressBar *option)
{
//	paintThinFrame(painter, option->rect, option->palette, 60, -20);
	painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.color(QPalette::Base));
//	painter->fillRect(option->rect.adjusted(1, 1, -1, -1), QColor(230, 230, 230));
}


void paintProgressBarLabel(QPainter *painter, const QStyleOptionProgressBarV2 *option)
{
#if 0
	QRectF rect = option->rect.adjusted(6, 0, -(option->rect.width() - 40), 0);
#endif
	QFont font(painter->font());
	font.setBold(true);
	painter->save();
	painter->setFont(font);
	qreal opacity = painter->opacity();
#if 1
	painter->setOpacity(0.3 * opacity);
	painter->setPen(Qt::black);
	if (option->version >= 2) {
		if (option->orientation == Qt::Vertical) {
			painter->translate(option->rect.center());
			painter->rotate(option->bottomToTop ? -90 : 90);
			painter->translate(- option->rect.center());
		}
	}
	painter->drawText(option->rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, option->text);
	painter->drawText(option->rect.adjusted(-1, 1, -1, 1), Qt::AlignCenter, option->text);
	painter->drawText(option->rect.adjusted(1, -1, 1, -1), Qt::AlignCenter, option->text);
	painter->drawText(option->rect.adjusted(-1, -1, -1, -1), Qt::AlignCenter, option->text);
#endif
	painter->setOpacity(0.9 * opacity);
//	painter->setPen(option->palette.color(QPalette::HighlightedText));
	painter->setPen(Qt::white);
	painter->drawText(option->rect, Qt::AlignCenter, option->text);
	//painter->setOpacity(opacity);
	painter->restore();
}


void paintProgressBarContents(QPainter *painter, const QStyleOptionProgressBar *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle */*style*/)
{
	const QProgressBar *bar = qobject_cast<const QProgressBar *>(widget);
	bool norange = (option->minimum == 0 && option->maximum == 0);
//	norange = 1;
	int percentage = 0;
	int m = QTime(0, 0).msecsTo(QTime::currentTime()) / 30;

	if (option->minimum < option->maximum) {
		// decide if we should animate
		// FIXME: this test should be done in timerEvent, to stop/restart timer.
		if (option->progress == option->maximum || option->text == QString()) {
			m = 0;
		} else if (bar) {
			if (bar->format() != QString::fromAscii("%p%")) {
				m = 0;
			}
		}
		percentage = 100 * (option->progress - option->minimum) / (option->maximum - option->minimum);
	}
	QRectF rect = option->rect.adjusted(2, 2, -2, -2);

	qreal r = (qMax(rect.width() - 2, rect.height() - 2) - 8.0) * 100.0 / 100.0 + 8.0;
	QPointF center = rect.center();
	QRadialGradient r_grad(center, r / 2.0);
	QLinearGradient l_grad(rect.topLeft(), rect.topRight());
	QGradient *gradient = &l_grad;
	QColor color = option->palette.color(QPalette::Highlight).dark(150);

	if (widget && widget->parentWidget() && widget->parentWidget()->parentWidget()
	 && widget->parentWidget()->parentWidget()->inherits("KNewPasswordDialog")) {
		color.setHsv(percentage * 85 / 100, 200, 240 - percentage);
	//	if (percentage > 75) m = 0;
	} else if (widget && widget->inherits("StatusBarSpaceInfo")) {
		int p = percentage;
		if (p < 75) p = 100; else p = (100 - p) * 4;
		color.setHsv(p * 85 / 100, 200, 240 - p);
	} else {
		gradient = &r_grad;
	}
	if (norange) {
		percentage = 100;
	}
	if (percentage == 0 && option->progress == option->minimum) {
		int v = m & 15;
		if (m & 16) v = 15 - v;
		color.setAlpha(100 + v * 5);
	}
	gradient->setColorAt(0.0, color);
	gradient->setColorAt(0.04, color);
	int i;
	for (i = 0; i < percentage; ++i) {
		int v;
		if (norange) {
			v = i + m;
		} else {
			v = i - m;
		}
		int k = v & 7;
		if (v & 8) k = 7 - k;
		if (norange) {
			color.setAlpha(2 * (100 - i) + 7 * k);
		} else {
			color.setAlpha(100 + (100 - i * 0) + 7 * k);
		}
		gradient->setColorAt(0.1 + i * 0.009, color);
	}
	gradient->setColorAt(0.1 + i * 0.009, QColor(0, 0, 0, 0));
	gradient->setColorAt(1.0, QColor(0, 0, 0, 0));
	painter->fillRect(option->rect.adjusted(2, 2, -2, -2), *gradient);
	QLinearGradient glassyGradient(option->rect.topLeft(), option->rect.bottomLeft());
#if 0 // really glassy
	glassyGradient.setColorAt(0.0, QColor(255, 255, 255, 210));
	glassyGradient.setColorAt(0.45, QColor(0, 0, 0, 2));
	glassyGradient.setColorAt(0.455, QColor(0, 0, 0, 11));
	glassyGradient.setColorAt(1.0, QColor(255, 255, 255, 60));
#else // like Skandale 0.0.2
	glassyGradient.setColorAt(0.0, QColor(255, 255, 255, 60));
	glassyGradient.setColorAt(0.45, QColor(0, 0, 0, 2));
	glassyGradient.setColorAt(0.455, QColor(0, 0, 0, 11));
	glassyGradient.setColorAt(1.0, QColor(255, 255, 255, 60));
#endif
	painter->fillRect(option->rect.adjusted(2, 2, -2, -2), glassyGradient);
	paintRecessedFrame(painter, option->rect, option->palette, RF_Small);
}


/*
 * skulpture_scrollbar.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QApplication>
#include <climits>


/*-----------------------------------------------------------------------*/

extern void paintScrollArrow(QPainter *painter, const QStyleOption *option, Qt::ArrowType arrow, bool spin);

void paintScrollArea(QPainter *painter, const QStyleOption *option)
{
	QColor color = option->palette.color(QPalette::Window);
	if (option->state & QStyle::State_Enabled || option->type != QStyleOption::SO_Slider) {
#if 0
		if (option->state & QStyle::State_Sunken) {
			color = color.lighter(107);
		} else {
			color = color.darker(107);
		}
#elif 0
		color = option->palette.color(QPalette::Base);
#elif 1
		if (option->state & QStyle::State_Sunken) {
			color = color.darker(107);
		} else {
			// ###
			if (false && option->state & QStyle::State_MouseOver) {
				color = color.lighter(110);
			} else {
				color = color.lighter(107);
			}
		}
#endif
	}
	painter->fillRect(option->rect, color);
//	painter->fillRect(option->rect, Qt::red);
}


void paintScrollAreaCorner(QPainter *painter, const QStyleOption *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle */*style*/)
{
	QStyleOption opt;
	opt = *option;
	opt.type = QStyleOption::SO_Default;
	if (widget && widget->inherits("QAbstractScrollArea")) {
		opt.type = QStyleOption::SO_Slider;
		opt.state &= ~QStyle::State_Enabled;
		if (widget->isEnabled()) {
			opt.state |= QStyle::State_Enabled;
		}
	}
	paintScrollArea(painter, &opt);
}


extern void paintSliderGroove(QPainter *painter, QRect &rect, const QStyleOptionSlider *option);

void paintScrollBarPage(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/)
{
	paintScrollArea(painter, option);
	QRect rect = option->rect.adjusted(1, 1, -1, -1);
	paintSliderGroove(painter, rect, option);
}


void paintScrollBarAddLine(QPainter *painter, const QStyleOptionSlider *option)
{
	paintScrollArea(painter, option);
//	paintThinFrame(painter, option->rect, option->palette, -40, 120);
	if (option->minimum != option->maximum) {
		QStyleOptionSlider opt = *option;
		opt.fontMetrics = QApplication::fontMetrics();
		paintScrollArrow(painter, &opt, option->orientation == Qt::Horizontal ? (option->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow) : Qt::DownArrow, false);
	}
}


void paintScrollBarSubLine(QPainter *painter, const QStyleOptionSlider *option)
{
	paintScrollArea(painter, option);
//	paintThinFrame(painter, option->rect, option->palette, -40, 120);
	if (option->minimum != option->maximum) {
		QStyleOptionSlider opt = *option;
		opt.fontMetrics = QApplication::fontMetrics();
		paintScrollArrow(painter, &opt, option->orientation == Qt::Horizontal ? (option->direction == Qt::RightToLeft ? Qt::RightArrow : Qt::LeftArrow) : Qt::UpArrow, false);
	}
}


/*-----------------------------------------------------------------------*/

extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole = QPalette::Window);
extern void paintSliderHandle(QPainter *painter, const QRect &rect, const QStyleOptionSlider *option);


void paintScrollBarSlider(QPainter *painter, const QStyleOptionSlider *option)
{
	if (option->minimum == option->maximum) {
		paintScrollArea(painter, option);
		return;
	}
#if 0
	// ### Qt behaviour differs between Slider and ScrollBar
	QStyleOptionSlider aOption = *option;
	if (!(option->activeSubControls & QStyle::SC_SliderHandle)) {
		aOption.state &= ~QStyle::State_MouseOver;
	}
	paintSliderHandle(painter, option->rect, &aOption);
#else
	paintSliderHandle(painter, option->rect, option);
#endif

#if 0 // slider grip
	QStyleOptionSlider grip = *option;
	int d = (qMin(option->rect.width(), option->rect.height()) / 6) * 2 + 1;
	grip.rect = QRect(option->rect.center() - QPoint(d / 2, d / 2), QSize(d, d));
	paintCachedGrip(painter, &grip);
#if 0
	d += (d + 1) / 2;
	if (option->orientation == Qt::Horizontal) {
		grip.rect.adjust(d, 0, d, 0);
		paintCachedGrip(painter, &grip);
		grip.rect.adjust(- 2 * d, 0, - 2 * d, 0);
		paintCachedGrip(painter, &grip);
	} else {
		grip.rect.adjust(0, d, 0, d);
		paintCachedGrip(painter, &grip);
		grip.rect.adjust(0, - 2 * d, 0, - 2 * d);
		paintCachedGrip(painter, &grip);
	}
#endif
#endif
}


/*-----------------------------------------------------------------------*/

void paintScrollBar(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ScrollBar, option, painter, widget);

	QFrame *frame = 0;
	if (widget && widget->parentWidget()) {
		QWidget *container = widget->parentWidget();
		if (container->inherits("Q3ListView")) {
			if (option->orientation == Qt::Vertical) {
				frame = qobject_cast<QFrame *>(container);
			}
		} else if (container->inherits("Q3Table")) {
			frame = qobject_cast<QFrame *>(container);
		} else if (container->parentWidget()) {
			frame = qobject_cast<QAbstractScrollArea *>(container->parentWidget());
		}
	}
	if (frame && frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken)) {
		QRect rect = option->rect;
		if (option->orientation == Qt::Vertical) {
			if (option->direction == Qt::LeftToRight) {
				rect.adjust(-3, 0, 1, 0);
			} else {
				rect.adjust(-1, 0, 2, 0);
			}
			// ### temporary hack to detect corner widget
			if (widget->height() == frame->height() - 4) {
				rect.adjust(0, -1, 0, 1);
			} else {
				rect.adjust(0, -1, 0, 4);
			}
		} else {
			rect.adjust(0, -3, 0, 1);
			// ### temporary hack to detect corner widget
			if (widget->width() == frame->width() - 4) {
				rect.adjust(-1, 0, 1, 0);
			} else {
				if (option->direction == Qt::LeftToRight) {
					rect.adjust(-1, 0, 4, 0);
				} else {
					rect.adjust(-4, 0, 1, 0);
				}
			}
		}
//		painter->fillRect(option->rect.adjusted(0, 0, 100, 100), Qt::red);
//		return;
#if 1
		paintRecessedFrameShadow(painter, rect.adjusted(1, 1, -1, -1), RF_Small);
#else
		paintRecessedFrameShadow(painter, rect.adjusted(-1, 1, -1, -1), RF_Large);
#endif
	}
}


/*-----------------------------------------------------------------------*/

QRect subControlRectScrollBar(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style)
{
	int maxsize;
	if (option->orientation == Qt::Horizontal) {
		maxsize = option->rect.width();
	} else {
		maxsize = option->rect.height();
	}

	int slidersize;
	if (option->maximum != option->minimum) {
		uint range = option->maximum - option->minimum;
		slidersize = (qint64(option->pageStep) * maxsize) / (range + option->pageStep);

		int slidermin = style->pixelMetric(QStyle::PM_ScrollBarSliderMin, option, widget);
		if (slidermin > maxsize / 2) {
			slidermin = maxsize / 2;
		}
		if (slidersize < slidermin || range > INT_MAX / 2) {
			slidersize = slidermin;
		}
		if (slidersize > maxsize) {
			slidersize = maxsize;
		}
	} else {
		slidersize = maxsize;
	}

	int sliderpos = style->sliderPositionFromValue(option->minimum, option->maximum, option->sliderPosition, maxsize - slidersize, option->upsideDown);
	int buttonsize = style->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget);
	buttonsize = qMin(maxsize / 2, buttonsize);
	int pos, size;
	switch (subControl) {
		case QStyle::SC_ScrollBarGroove:
			pos = 0;
			size = maxsize;
			break;
		case QStyle::SC_ScrollBarSlider:
			pos = sliderpos;
			size = slidersize;
			break;
		case QStyle::SC_ScrollBarSubPage:
			pos = buttonsize;
			size = sliderpos - buttonsize;
			break;
		case QStyle::SC_ScrollBarAddPage:
			pos = sliderpos + slidersize;
			size = maxsize - sliderpos - slidersize - buttonsize;
			break;
		case QStyle::SC_ScrollBarSubLine:
			pos = 0;
			size = buttonsize;
			//size = qMin(buttonsize, sliderpos);
			break;
		case QStyle::SC_ScrollBarAddLine:
			size = buttonsize;
			//size = qMin(buttonsize, maxsize - sliderpos - slidersize);
			pos = maxsize - size;
			break;
		default:
			pos = 0;
			size = 0;
			break;
	}

	QRect rect;
	if (option->orientation == Qt::Horizontal) {
		rect = QRect(pos, 0, size, option->rect.height());
	} else {
		rect = QRect(0, pos, option->rect.width(), size);
	}
	return style->visualRect(option->direction, option->rect, rect);
}


/*-----------------------------------------------------------------------*/
/*
 * Arrow positions:
 *	*    <*>    <*<>    *<>    <>*<>    <>*
 *
 */

/*
 * skulpture_shadows.cpp
 *
 */

#include "skulpture_p.h"
//#include <QtGui/QFrame>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QMdiArea>
//#include <q3scrollview.h>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <cstdio>


/*-----------------------------------------------------------------------*/
/**
 * FrameShadow - overlay a shadow inside a frame
 *
 * This class is used to give the impression of sunken
 * frames that cast shadows over the contents inside.
 *
 * To create the shadow, call installFrameShadow()
 * on the sunken QFrame that you want the shadow
 * to be added to.
 *
 * Side-effects:
 *
 * Adds transparent widgets to the frame, which may
 * affect performance.
 *
 */

FrameShadow::FrameShadow(QWidget *parent)
	: QWidget(parent)
{
	init();
}


FrameShadow::FrameShadow(ShadowArea area, QWidget *parent)
	: QWidget(parent)
{
	init();
	area_ = area;
}


FrameShadow::~FrameShadow()
{
	/* */
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::installFrameShadow(QWidget *widget)
{
//	printf("adding shadow to %s\n", widget->metaObject()->className());
	widget->installEventFilter(this);
	removeFrameShadow(widget);
	for (int i = 0; i < 4; ++i) {
		FrameShadow *shadow = new FrameShadow(FrameShadow::ShadowArea(i));
		shadow->hide();
		shadow->setParent(widget);
		shadow->updateGeometry();
		shadow->show();
	}
}


void SkulptureStyle::Private::removeFrameShadow(QWidget *widget)
{
	const QList<QObject *> shadows = widget->children();
	foreach (QObject *child, shadows) {
		FrameShadow *shadow;
		if ((shadow = qobject_cast<FrameShadow *>(child))) {
			shadow->hide();
			shadow->setParent(0);
			shadow->deleteLater();
		}
	}
}


void SkulptureStyle::Private::updateFrameShadow(QWidget *widget)
{
	const QList<QObject *> shadows = widget->children();
	foreach (QObject *child, shadows) {
		FrameShadow *shadow;
		if ((shadow = qobject_cast<FrameShadow *>(child))) {
			if (shadow->isVisible()) {
				shadow->updateGeometry();
			}
		}
	}
}


/*-----------------------------------------------------------------------*/

#define SHADOW_SIZE_TOP		4
#define SHADOW_SIZE_BOTTOM	2
#define SHADOW_SIZE_LEFT		4
#define SHADOW_SIZE_RIGHT		4

void FrameShadow::updateGeometry()
{
	QWidget *widget = parentWidget();
	QRect cr = widget->contentsRect();

//	printf("cr-top: %d in class %s\n", cr.top(), widget->metaObject()->className());
	switch (shadowArea()) {
		case FrameShadow::Top:
			cr.setHeight(SHADOW_SIZE_TOP);
			break;
		case FrameShadow::Left:
			cr.setWidth(SHADOW_SIZE_LEFT);
			cr.adjust(0, SHADOW_SIZE_TOP, 0, -SHADOW_SIZE_BOTTOM);
			break;
		case FrameShadow::Bottom:
			cr.setTop(cr.bottom() - SHADOW_SIZE_BOTTOM + 1);
			break;
		case FrameShadow::Right:
			cr.setLeft(cr.right() - SHADOW_SIZE_RIGHT + 1);
			cr.adjust(0, SHADOW_SIZE_TOP, 0, -SHADOW_SIZE_BOTTOM);
			break;
	}
	setGeometry(cr);
}


/*-----------------------------------------------------------------------*/

bool FrameShadow::event(QEvent *e)
{
	if (e->type() == QEvent::Paint) {
		return QWidget::event(e);
	}
	QWidget *viewport = 0;
	if (parentWidget()) {
		if (QAbstractScrollArea *widget = qobject_cast<QAbstractScrollArea *>(parentWidget())) {
			viewport = widget->viewport();
		} else if (parentWidget()->inherits("Q3ScrollView")) {
			// FIXME: get viewport? needs Qt3Support linkage!
			viewport = 0;
		} else {
			viewport = 0;
		}
	}
	if (!viewport) {
		return false;
	}

#if 1
	switch (e->type()) {
		case QEvent::DragEnter:
		case QEvent::DragMove:
		case QEvent::DragLeave:
			case QEvent::Drop: {
				setAcceptDrops(viewport->acceptDrops());
				QObject *o = viewport;
				return o->event(e);
			}
			break;
		case QEvent::Enter:
			setCursor(viewport->cursor());
			setAcceptDrops(viewport->acceptDrops());
			break;
			case QEvent::ContextMenu: {
				QContextMenuEvent *me = reinterpret_cast<QContextMenuEvent *>(e);
				QContextMenuEvent *ne = new QContextMenuEvent(me->reason(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos());
				QApplication::sendEvent(viewport, ne);
				e->accept();
				return true;
			}
			break;
		case QEvent::MouseButtonPress:
			releaseMouse();
		case QEvent::MouseMove:
			case QEvent::MouseButtonRelease: {
				QMouseEvent *me = reinterpret_cast<QMouseEvent *>(e);
				QMouseEvent *ne = new QMouseEvent(e->type(), parentWidget()->mapFromGlobal(me->globalPos()), me->globalPos(), me->button(), me->buttons(), me->modifiers());
				QApplication::sendEvent(viewport, ne);
				e->accept();
				return true;
			}
			break;
		case QEvent::Paint:
			return QWidget::event(e);
		default:
			break;
	}
	e->ignore();
	return false;
#else
	return QWidget::event(e);
#endif
}


/*-----------------------------------------------------------------------*/

void FrameShadow::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	QWidget *parent = parentWidget();
	QRect r = parent->contentsRect();
	r.translate(mapFromParent(QPoint(0, 0)));
//	painter.fillRect(QRect(-100, -100, 1000, 1000), Qt::red);
	paintRecessedFrameShadow(&painter, r, RF_Large);
}


/*-----------------------------------------------------------------------*/

void FrameShadow::init()
{
//	setAttribute(Qt::WA_NoSystemBackground, true);
//	setAttribute(Qt::WA_NoBackground, true);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
	setFocusPolicy(Qt::NoFocus);
	// TODO: check if this is private
	setAttribute(Qt::WA_TransparentForMouseEvents, true);
	setContextMenuPolicy(Qt::NoContextMenu);

	QWidget *viewport;
	if (parentWidget()) {
		if (QAbstractScrollArea *widget = qobject_cast<QAbstractScrollArea *>(parentWidget())) {
			setAcceptDrops(true);
			viewport = widget->viewport();
		} else if (parentWidget()->inherits("Q3ScrollView")) {
			// FIXME: get viewport? needs Qt3Support linkage!
			viewport = parentWidget();
		} else {
			viewport = 0;
		}
		if (viewport) {
			setCursor(viewport->cursor());
		}
	}
}


/*-----------------------------------------------------------------------*/

void paintRecessedFrameShadow(QPainter *painter, const QRect &rect, enum RecessedFrame rf)
{
	if (rf == RF_None) return;
#if 1
	int c1 = (rf == RF_Small) ? 10 : 10;
	int c2 = (rf == RF_Small) ? 20 : 36;
#else
	int c1 = 0;
	int c2 = 0;
#endif
#if 0
	c1 += c1 >> 1;
	c2 += c2 >> 1;
	int intensityTop = c2;
	int intensityLeft = c2;
	int intensityBottom = c1;
	int intensityRight = c1;
#endif
	QRect r = rect;
	while (c1 > 3 || c2 > 3) {
		QBrush brush1(QColor(0, 0, 0, c1));
		QBrush brush2(QColor(0, 0, 0, c2));

		painter->fillRect(QRect(rect.left(), r.top(), rect.width(), 1), brush2);
		painter->fillRect(QRect(r.left(), rect.top(), 1, rect.height()), brush2);
		painter->fillRect(QRect(rect.left(), r.bottom(), rect.width(), 1), brush1);
		painter->fillRect(QRect(r.right(), rect.top(), 1, rect.height()), brush1);
		c1 >>= 1; c2 >>= 1;
//		c1 = int(c1 * 0.7); c2 = int(c2 * 0.7);
		r.adjust(1, 1, -1, -1);
	}
}


/*-----------------------------------------------------------------------*/
/**
 * WidgetShadow - add a drop shadow to a widget
 *
 * This class renders a shadow below a widget, such
 * as a QMdiSubWindow.
 *
 */

WidgetShadow::WidgetShadow(QWidget *parent)
	: QWidget(parent)
{
	init();
}


/*-----------------------------------------------------------------------*/

void WidgetShadow::init()
{
	setObjectName(QString::fromUtf8("WidgetShadow"));
//	setAttribute(Qt::WA_NoSystemBackground, true);
//	setAttribute(Qt::WA_NoBackground, true);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
//	setAutoFillBackground(false);
	setFocusPolicy(Qt::NoFocus);
	// TODO: check if this is private
	setAttribute(Qt::WA_TransparentForMouseEvents, true);
//	setContextMenuPolicy(Qt::NoContextMenu);
	widget_ = 0;
}


/*-----------------------------------------------------------------------*/

bool WidgetShadow::event(QEvent *e)
{
	switch (e->type())
	{
		case QEvent::Paint: if (widget_) {
			QRect r(- 10, - 5, widget_->frameGeometry().width() + 20, widget_->frameGeometry().height() + 15);
			r.translate(qMin(widget_->x(), 10), qMin(widget_->y(), 5));
			QPainter p(this);
			QRegion region(r);
			region -= QRect(r.adjusted(10, 5, -10, -10));
			p.setClipRegion(region);
#if 0
			for (int i = 0; i < 10; ++i) {
			int k = 9 - i;
			p.fillRect(r.adjusted(k, i, -k, -i), QColor(0, 0, 0, i));
			p.fillRect(r.adjusted(i, k, -i, -k), QColor(0, 0, 0, i));
		}
#else
			for (int i = 0; i < 10; ++i) {
				p.fillRect(r, QColor(0, 0, 0, 2 + i));
				r.adjust(1, 1, -1, -1);
			}
#endif
			e->ignore();
			return (true);
		}
		default:
			break;
	}
	return QWidget::event(e);
}


void WidgetShadow::updateGeometry()
{
	if (widget_) {
		if (widget_->isHidden()) {
			hide();
		} else {
			QWidget *parent = parentWidget();
			if (parent && !qobject_cast<QMdiArea *>(parent) && qobject_cast<QMdiArea *>(parent->parentWidget())) {
				parent = parent->parentWidget();
			}
			if (parent) {
				QRect geo(widget_->x() - 10, widget_->y() - 5, widget_->frameGeometry().width() + 20, widget_->frameGeometry().height() + 15);
				setGeometry(geo & parent->rect());
			}
			show();
		}
	}
}


void WidgetShadow::updateZOrder()
{
	if (widget_) {
		if (widget_->isHidden()) {
			hide();
		} else {
			stackUnder(widget_);
			QWidget *parent = parentWidget();
			if (parent && !qobject_cast<QMdiArea *>(parent) && qobject_cast<QMdiArea *>(parent->parentWidget())) {
				parent = parent->parentWidget();
			}
			if (parent) {
				QRect geo(widget_->x() - 10, widget_->y() - 5, widget_->frameGeometry().width() + 20, widget_->frameGeometry().height() + 15);
				setGeometry(geo & parent->rect());
			}
			show();
		}
	}
}


/*
 * skulpture_shortcuts.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAbstractButton>
#include <QtGui/QLabel>
#include <QtGui/QDockWidget>
#include <QtGui/QToolBox>
#include <QtGui/QGroupBox>
#include <QtGui/QTabBar>


/*-----------------------------------------------------------------------*/
/**
 * Class to manage the applications keyboard shortcuts
 *
 * It acts as an eventfilter for the application, and does the following:
 *
 *	* the shortcuts are only underlined when the Alt key is pressed, and
 *		the underline is removed once it is released.
 *	* on many widgets the focus frame is not displayed if the focus was
 *		received using the mouse. It is, however, still displayed if
 *		you use the Tab key.
 *
 * Any other keyboard shortcut and focus issues should also be handled here.
 *
 */


/*-----------------------------------------------------------------------*/

bool ShortcutHandler::underlineShortcut(const QWidget *widget) const
{
	if (widget && widget->isEnabled()) {
		if (alt_pressed.contains(widget->window())) {
			return true;
		}
		if (qobject_cast<const QMenuBar *>(widget)) {
			if (widget->hasFocus()) {
				return true;
			}
			QList<QWidget *> children = qFindChildren<QWidget *>(widget);
			foreach (QWidget *child, children) {
				if (child->hasFocus()) {
					return true;
				}
			}
		}
		if (qobject_cast<const QMenu *>(widget)) {
			return true;
		}
	}
	return false;
}


static inline bool hasShortcut(QWidget *widget)
{
	if (qobject_cast<const QAbstractButton *>(widget) // && (qobject_cast<const QAbstractButton *>(widget))->text.contains(QChar('&', 0))
	 || qobject_cast<const QLabel *>(widget)
	 || qobject_cast<const QDockWidget *>(widget)
	 || qobject_cast<const QToolBox *>(widget)
//	 || qobject_cast<const QMenu *>(widget)
	 || qobject_cast<const QMenuBar *>(widget)
	 || qobject_cast<const QGroupBox *>(widget) // && (qobject_cast<const QGroupBox *>(widget))->isCheckable()
	 || qobject_cast<const QTabBar *>(widget)) {
		return true;
	}
	return false;
}


static inline void updateShortcuts(QWidget *widget)
{
	QList<QWidget *> children = qFindChildren<QWidget *>(widget);
	foreach (QWidget *child, children) {
		if (child->isVisible() && hasShortcut(child)) {
			child->update();
		}
	}
}


bool ShortcutHandler::eventFilter(QObject *watched, QEvent *event)
{
	// event filter may be called on the application, abort in this case
	if (!watched->isWidgetType()) {
		return QObject::eventFilter(watched, event);
	}

	QWidget *widget = reinterpret_cast<QWidget *>(watched);

	switch (event->type()) {
		case QEvent::FocusIn: {
				Qt::FocusReason reason = ((QFocusEvent *) event)->reason();
				if (reason != Qt::TabFocusReason && reason != Qt::BacktabFocusReason) {
					QWidget *window = widget->window();
					window->setAttribute(Qt::WA_KeyboardFocusChange, false);
				}
			}
			break;
		case QEvent::KeyPress:
			if (((QKeyEvent *) event)->key() == Qt::Key_Alt) {
				QWidget *window = widget->window();
				if (!alt_pressed.contains(window)) {
					alt_pressed.append(window);
					window->installEventFilter(this);
					updateShortcuts(window);
				}
			}
			break;
		case QEvent::Close:
			if (widget->isWindow()) {
				alt_pressed.removeAll(widget);
				widget->removeEventFilter(this);
			}
			break;
		case QEvent::WindowDeactivate:
			if (widget->isWindow()) {
				alt_pressed.removeAll(widget);
				widget->removeEventFilter(this);
				updateShortcuts(widget);
			}
			break;
		case QEvent::KeyRelease:
			if (((QKeyEvent *) event)->key() == Qt::Key_Alt) {
				QWidget *window = widget->window();
				if (alt_pressed.contains(window)) {
					alt_pressed.removeAll(window);
					window->removeEventFilter(this);
					updateShortcuts(window);
				}
			}
			break;
		default:
			break;
	}
	return QObject::eventFilter(watched, event);
}


/*-----------------------------------------------------------------------*/

ShortcutHandler::ShortcutHandler(QObject *parent)
	: QObject(parent)
{
	init();
}


ShortcutHandler::~ShortcutHandler()
{

}


/*
 * skulpture_slider.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QAbstractSlider>
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/

void paintSliderGroove(QPainter *painter, QRect &rect, const QStyleOptionSlider *option)
{
	if (option->orientation == Qt::Horizontal) {
		int d = rect.height() / 2;
		rect.adjust(0, d, 0, -d);
	} else {
		int d = rect.width() / 2;
		rect.adjust(d, 0, -d, 0);
	}
	QColor color = option->palette.color(QPalette::Window);
	if (option->state & QStyle::State_Enabled) {
		color = color.dark(120);
		painter->fillRect(rect, color);
		paintThinFrame(painter, rect.adjusted(-1, -1, 1, 1), option->palette, -30, -90);
	} else {
		painter->fillRect(rect, color);
		paintThinFrame(painter, rect.adjusted(-1, -1, 1, 1), option->palette, -20, -60);
	}
}


void paintSliderHandle(QPainter *painter, const QRect &rect, const QStyleOptionSlider *option)
{
	// shadow
	painter->fillRect(rect.adjusted(2, 2, 2, 2), QColor(0, 0, 0, 5));
	painter->fillRect(rect.adjusted(1, 1, 1, 1), QColor(0, 0, 0, 8));
	// slider color
	QColor color = option->palette.color(QPalette::Button);
	if (option->state & QStyle::State_Enabled) {
		if (option->state & QStyle::State_Sunken) {
			color = color.light(102);
		} else if (option->state & QStyle::State_MouseOver) {
			color = color.light(104);
		}
	} else {
		color = option->palette.color(QPalette::Window);
	}
	painter->fillRect(rect, color);

#if 1 // slider gradient
	if ((option->state & QStyle::State_Enabled) && !(option->state & QStyle::State_Sunken)) {
		QLinearGradient gradient;
		gradient.setStart(rect.topLeft());
		if (option->orientation == Qt::Horizontal) {
			gradient.setFinalStop(rect.bottomLeft());
		} else {
			gradient.setFinalStop(rect.topRight());
		}
#if 1
		// SkandaleStyle 0.0.2
		gradient.setColorAt(0.0, shaded_color(color, 40));
		gradient.setColorAt(0.5, shaded_color(color, -5));
		gradient.setColorAt(1.0, shaded_color(color, 70));
#else
		// glassy
		gradient.setColorAt(0.0, shaded_color(color, 40));
		gradient.setColorAt(0.4, shaded_color(color, -5));
		gradient.setColorAt(0.405, shaded_color(color, -15));
		gradient.setColorAt(1.0, shaded_color(color, 70));
#endif
		painter->fillRect(rect, gradient);
	}
#endif
	// slider frame
	paintThinFrame(painter, rect, option->palette, -70, -20, QPalette::Button);
	paintThinFrame(painter, rect.adjusted(1, 1, -1, -1), option->palette, -30, 130, QPalette::Button);
}


/*-----------------------------------------------------------------------*/

void paintSlider(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	// groove
	if (option->subControls & QStyle::SC_SliderGroove) {
	//	painter->fillRect(option->rect, option->palette.color(QPalette::Window).dark(105));
	//	paintThinFrame(painter, option->rect, option->palette, 130, -30);
#if 0
		int handlesize = style->pixelMetric(QStyle::PM_SliderLength, option, widget);
		int e = handlesize / 2 - 1;
#else
		int e = 1;
#endif
#if 0
		QRect rect = option->rect;
	//	QRect rect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderGroove, widget);
		if (option->orientation == Qt::Horizontal) {
			rect.adjust(e, 0, -e, 0);
		} else {
			rect.adjust(0, e, 0, -e);
		}
		paintSliderGroove(painter, rect, option);
#else
		QRect rect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderGroove, widget);
		//QRect rect = option->rect;
		QRect handle_rect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderHandle, widget);
		QStyleOptionSlider aOption = *option;
		aOption.palette.setColor(QPalette::Window, aOption.palette.color(QPalette::Highlight));
		if (option->orientation == Qt::Horizontal) {
			handle_rect.adjust(0, 2, 0, -2);
			rect.adjust(e, 0, -e, 0);
			rect.setWidth(handle_rect.left() - rect.left() - 1);
			if (rect.width() > -3) {
				paintSliderGroove(painter, rect, option->upsideDown ? option : &aOption);
			}
			rect.setLeft(handle_rect.right() + 2);
			rect.setWidth(option->rect.right() - handle_rect.right() - 1 - e);
			if (rect.width() > -3) {
				paintSliderGroove(painter, rect, option->upsideDown ? &aOption : option);
			}
		} else {
			handle_rect.adjust(2, 0, -2, 0);
			rect.adjust(0, e, 0, -e);
			rect.setHeight(handle_rect.top() - rect.top() - 1);
			if (rect.height() > -3) {
				paintSliderGroove(painter, rect, option->upsideDown ? option : &aOption);
			}
			rect.setTop(handle_rect.bottom() + 2);
			rect.setHeight(option->rect.bottom() - handle_rect.bottom() - e);
			if (rect.height() > -3) {
				paintSliderGroove(painter, rect, option->upsideDown ? &aOption : option);
			}
		}
#endif
	}

#if 1	// tickmarks
	if (option->subControls & QStyle::SC_SliderTickmarks) {
		qreal opacity = painter->opacity();
		QStyleOptionSlider slider = *option;
		slider.subControls = QStyle::SC_SliderTickmarks;
		// ### for now, just use common tickmarks
		painter->setOpacity(opacity * 0.2);
		if (option->orientation == Qt::Horizontal) {
			slider.rect.adjust(-1, 0, -1, 0);
		} else {
			slider.rect.adjust(0, -1, 0, -1);
		}
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_Slider, &slider, painter, widget);
		slider.rect = option->rect;
		QPalette palette = slider.palette;
		palette.setColor(QPalette::WindowText, Qt::white);
		slider.palette = palette;
		painter->setOpacity(opacity * 0.3);
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_Slider, &slider, painter, widget);
		painter->setOpacity(opacity);
	}
#endif
	// focus rect
	if (option->state & QStyle::State_HasFocus) {
		QStyleOptionFocusRect focus;
		focus.QStyleOption::operator=(*option);
		focus.rect = style->subElementRect(QStyle::SE_SliderFocusRect, option, widget);
		focus.state |= QStyle::State_FocusAtBorder;
		style->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, painter, widget);
	}

	// handle
	if (option->subControls & QStyle::SC_SliderHandle) {
		QStyleOptionSlider aOption = *option;
		if (!(option->activeSubControls & QStyle::SC_SliderHandle)) {
			aOption.state &= ~QStyle::State_MouseOver;
			aOption.state &= ~QStyle::State_Sunken;
		}
		QRect rect = style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderHandle, widget);
		if (option->orientation == Qt::Horizontal) {
			rect.adjust(0, 2, 0, -2);
		} else {
			rect.adjust(2, 0, -2, 0);
		}
#if 0
		if (option->orientation == Qt::Horizontal) {
			rect.setTop(option->rect.top());
			rect.setHeight(option->rect.height());
		//	rect.adjust(0, 1, 0, -1);
		} else {
			rect.setLeft(option->rect.left());
			rect.setWidth(option->rect.width());
		//	rect.adjust(1, 0, -1, 0);
		}
#endif
	//	rect.adjust(0, 0, -1, -1);
		paintSliderHandle(painter, rect, &aOption);
	//	rect.adjust(0, 0, 1, 1);
	//	paintThinFrame(painter, rect.adjusted(1, 1, 1, 1), option->palette, -20, 0);
#if 0
		// grip
		const int o = 5;
		const int s = 6;
		if (option->orientation == Qt::Horizontal) {
			int d = (rect.width() - 2) / 2;
			rect.adjust(d, 0, -d, 0);
			rect.translate(-s, 0);
		} else {
			int d = (rect.height() - 2) / 2;
			rect.adjust(0, d, 0, -d);
			rect.translate(0, -s);
		}
		for (int k = -1; k < 2; ++k) {
			if (option->orientation == Qt::Horizontal) {
				painter->fillRect(rect.adjusted(0, o, 0, -o), QColor(0, 0, 0, 30));
				painter->fillRect(rect.adjusted(1, o, 1, -o), QColor(255, 255, 255, 80));
				rect.translate(s, 0);
			} else {
				painter->fillRect(rect.adjusted(o, 0, -o, 0), QColor(0, 0, 0, 30));
				painter->fillRect(rect.adjusted(o, 1, -o, 1), QColor(255, 255, 255, 80));
				rect.translate(0, s);
			}
		}
#endif
	}
#if 0
	painter->fillRect(style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderGroove, widget), QColor(0, 0, 0, 70));
	if (option->subControls & QStyle::SC_SliderGroove) {
		painter->fillRect(style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderGroove, widget), QColor(255, 0, 0, 70));
	}
	if (option->subControls & QStyle::SC_SliderHandle) {
		painter->fillRect(style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderHandle, widget), QColor(0, 100, 255, 70));
	}
	if (option->subControls & QStyle::SC_SliderTickmarks) {
		painter->fillRect(style->subControlRect(QStyle::CC_Slider, option, QStyle::SC_SliderTickmarks, widget), QColor(0, 255, 0, 170));
	}
#endif
}


/*-----------------------------------------------------------------------*/

QRect subControlRectSlider(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style)
{
	QRect rect = ((QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_Slider, option, subControl, widget);
	switch (subControl) {
		case QStyle::SC_SliderGroove:
		case QStyle::SC_SliderTickmarks:
		case QStyle::SC_SliderHandle:
		default:
			break;
	}
	return rect;
#if 0
	/*
	option->orientation
	option->tickPosition: QSlider::TicksAbove | QSlider::TicksBelow
	option->state
	option->tickInterval
	*/
	switch (subControl) {
		case SC_SliderGrove:
		case SC_SliderTickmarks:
		case SC_SliderHandle:
	}
#endif
}


/*
 * skulpture_spinbox.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QAbstractSpinBox>
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/

extern void paintScrollArrow(QPainter *painter, const QStyleOption *option, Qt::ArrowType arrow, bool spin);

void paintIndicatorSpinDown(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::DownArrow, true);
}


void paintIndicatorSpinUp(QPainter *painter, const QStyleOption *option)
{
	paintScrollArrow(painter, option, Qt::UpArrow, true);
}


void paintSpinBox(QPainter *painter, const QStyleOptionSpinBox *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	QStyleOption arrowOpt;

	// arrow background
	if (option->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) {
		QRect r  = style->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxUp, widget);
		r |= style->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxDown, widget);
		QColor color = option->palette.color(QPalette::Base);
		if (option->state & QStyle::State_Enabled) {
			color = option->palette.color(QPalette::Window).lighter(107);
		}
		if (option->direction == Qt::LeftToRight) {
			painter->fillRect(r.adjusted(0, 2, -2, -2), color);
		} else {
			painter->fillRect(r.adjusted(2, 2, 0, -2), color);
		}
	}

	// arrows
	if (true /* || option->state & QStyle::State_MouseOver && option->state & QStyle::State_Enabled*/) {
		if (option->subControls & QStyle::SC_SpinBoxUp) {
			arrowOpt.QStyleOption::operator=(*option);
			arrowOpt.rect = style->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxUp, widget).adjusted(-1, 2, -2, -1);
			if (!(option->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
				arrowOpt.state &= ~(QStyle::State_Enabled | QStyle::State_MouseOver);
				arrowOpt.palette.setCurrentColorGroup(QPalette::Disabled);
			}
			if (!(option->activeSubControls & QStyle::SC_SpinBoxUp)) {
				arrowOpt.state &= ~(QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver);
			}
			if (option->direction == Qt::LeftToRight) {
				arrowOpt.rect.adjust(1, 3, -2, 0);
			} else {
				arrowOpt.rect.adjust(4, 3, 1, 0);
			}
			style->drawPrimitive(option->buttonSymbols == QAbstractSpinBox::PlusMinus ? QStyle::PE_IndicatorSpinPlus : QStyle::PE_IndicatorSpinUp, &arrowOpt, painter, widget);
		}
		if (option->subControls & QStyle::SC_SpinBoxDown) {
			arrowOpt.QStyleOption::operator=(*option);
			arrowOpt.rect = style->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxDown, widget).adjusted(-1, 0, -2, -2);
			if (!(option->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
				arrowOpt.state &= ~(QStyle::State_Enabled | QStyle::State_MouseOver);
				arrowOpt.palette.setCurrentColorGroup(QPalette::Disabled);
			}
			if (!(option->activeSubControls & QStyle::SC_SpinBoxDown)) {
				arrowOpt.state &= ~(QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver);
			}
			if (option->direction == Qt::LeftToRight) {
				arrowOpt.rect.adjust(1, 0, -2, -2);
			} else {
				arrowOpt.rect.adjust(4, 0, 1, -2);
			}
			style->drawPrimitive(option->buttonSymbols == QAbstractSpinBox::PlusMinus ? QStyle::PE_IndicatorSpinMinus : QStyle::PE_IndicatorSpinDown, &arrowOpt, painter, widget);
		}
	}

	// frame
	if (option->frame && (option->subControls & QStyle::SC_SpinBoxFrame)) {
		QStyleOptionFrame frameOpt;
		frameOpt.QStyleOption::operator=(*option);
		frameOpt.rect = style->subControlRect(QStyle::CC_SpinBox, option, QStyle::SC_SpinBoxFrame, widget);
		frameOpt.state |= QStyle::State_Sunken;
		frameOpt.lineWidth = style->pixelMetric(QStyle::PM_SpinBoxFrameWidth, &frameOpt, widget);
		frameOpt.midLineWidth = 0;
		style->drawPrimitive(QStyle::PE_FrameLineEdit, &frameOpt, painter, widget);
	}
}


/*
 * skulpture_tabs.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <cstdio>


/*-----------------------------------------------------------------------*/

enum Pos { North, South, West, East };

static inline Pos tabPos(QTabBar::Shape shape)
{
	return Pos(int(shape) & 3);
}

static inline bool isVertical(QTabBar::Shape shape)
{
	return (int(shape) & 2);
}

struct Affinity {
	int x1, y1, x2, y2;
};

static inline void tabAffinity(QTabBar::Shape shape, Affinity &affinity, int amount)
{
	affinity.x1 = affinity.y1 = affinity.x2 = affinity.y2 = 0;
	switch (tabPos(shape)) {
		case North:	affinity.y1 = amount;	break;
		case South:	affinity.y2 = -amount;	break;
		case West:	affinity.x1 = amount;	break;
		case East:		affinity.x2 = -amount;	break;
	}
}


/*-----------------------------------------------------------------------*/

static void paintTabBase(QPainter *painter, const QRect &rect, const QStyleOption *option, QTabBar::Shape shape)
{
	if (true /*option->state & QStyle::State_Enabled*/) {
		QLinearGradient tabGradient(rect.topLeft(), isVertical(shape) ? rect.topRight() : rect.bottomLeft());
		tabGradient.setColorAt(0.0, option->palette.color(QPalette::Window).darker(118));
		tabGradient.setColorAt(1.0, option->palette.color(QPalette::Window).darker(105));
		painter->fillRect(rect.adjusted(1, 1, -1, -1), tabGradient);
	} else {
		painter->fillRect(rect.adjusted(1, 1, -1, -1), option->palette.color(QPalette::Window).darker(106));
	}
	paintThinFrame(painter, rect.adjusted(1, 1, -1, -1), option->palette, -20, -40);
	paintRecessedFrameShadow(painter, rect.adjusted(2, 2, -2, -2), RF_Small);
}


void paintFrameTabBarBase(QPainter *painter, const QStyleOptionTabBarBase *option)
{
	// ### remove clipping
	painter->save();
	QRect r = option->rect;
	r = r.united(option->tabBarRect);
	QRegion region(r);
	region -= option->tabBarRect;
	painter->setClipRegion(region);
	paintTabBase(painter, r, option, option->shape);
	QRect rect = r;
#if 0
	Affinity affinity;
	tabAffinity(option->shape, affinity, 1);
	rect.adjust(affinity.x2, affinity.y2, affinity.x1, affinity.y1);
#endif
	paintThinFrame(painter, rect, option->palette, 60, -20);
	painter->restore();
}


void paintTabWidgetFrame(QPainter *painter, const QStyleOptionTabWidgetFrame *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget)
{
	Q_UNUSED(widget);

	QRect base = option->rect;
	int s = (isVertical(option->shape) ? option->tabBarSize.width() : option->tabBarSize.height());
	if (s < 2) s = 2;
	if (isVertical(option->shape)) {
		base.setWidth(s);
	} else {
		base.setHeight(s);
	}
	const int overlap = 2;
	switch (tabPos(option->shape)) {
		case North:	base.translate(0, -(s - overlap));	break;
		case West:	base.translate(-(s - overlap), 0);	break;
		case South:	base.translate(0, option->rect.height() - overlap);	break;
		case East:		base.translate(option->rect.width() - overlap, 0);	break;
	}
	if (s != 2) {
		paintTabBase(painter, base, option, option->shape);
	}

	Affinity affinity;
	tabAffinity(option->shape, affinity, -(s - overlap));
//	painter->save();
//	painter->setClipRect(base);
	paintThinFrame(painter, option->rect.adjusted(affinity.x1, affinity.y1, affinity.x2, affinity.y2), option->palette, 60, -20);
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -50, 160);
#if 0
	QRect r = option->rect.adjusted(2, 2, -2, -2);
	painter->fillRect(r, option->palette.color(QPalette::Window));
	QLinearGradient gradient(r.topLeft(), r.bottomLeft());
	gradient.setColorAt(0.0, QColor(255, 255, 255, 0));
	gradient.setColorAt(1.0, QColor(0, 0, 0, 20));
	painter->fillRect(r, gradient);
#endif
//	painter->restore();
}


/*-----------------------------------------------------------------------*/

#define TAB_SHIFT 1

void paintTabBarTabShape(QPainter *painter, const QStyleOptionTab *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);
	bool mouse = (option->state & QStyle::State_MouseOver) && !(option->state & QStyle::State_Selected) && (option->state & QStyle::State_Enabled);
	QRect c_rect = option->rect;
	bool konq = false;

	if (widget && widget->parentWidget()) {
		if (!qstrcmp(widget->metaObject()->className(), "KTabBar") && !qstrcmp(widget->parentWidget()->metaObject()->className(), "KonqFrameTabs")) {
			konq = true;
		}
	}
	if (konq || (widget && (!(widget->parentWidget()) || !(widget->parentWidget()->inherits("QTabWidget"))))) {
		// ### remove clipping
		painter->save();
		painter->setClipRect(option->rect);
		QRect rect = widget->rect();
		if (konq) {
			rect.adjust(-10, 0, 10, 0);
		}
		paintTabBase(painter, rect, option, option->shape);
#if 0
		Affinity affinity;
		tabAffinity(option->shape, affinity, 1);
		rect.adjust(affinity.x2, affinity.y2, affinity.x1, affinity.y1);
#endif
		paintThinFrame(painter, rect, option->palette, 60, -20);
		painter->restore();
	}

	switch (tabPos(option->shape)) {
	case North:
		c_rect.adjust(1, 1, -2, 0);
		if (option->position != QStyleOptionTab::Beginning
		&& option->position != QStyleOptionTab::OnlyOneTab) {
			c_rect.adjust(-1, 0, 0, 0);
		}
		if (option->state & QStyle::State_Selected) {
			painter->fillRect(c_rect.adjusted(0, 0, -1, 0), option->palette.color(QPalette::Window));
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.bottomLeft());
#if 0
				QColor c = option->palette.color(QPalette::Highlight);
				gradient.setColorAt(0.0, c);
				gradient.setColorAt(0.2, QColor(255, 255, 255, 20));
				gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
#else
				gradient.setColorAt(0.0, shaded_color(option->palette.color(QPalette::Window), 20));
				gradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), 0));
#endif
				painter->fillRect(c_rect.adjusted(0, 0, -1, 0), gradient);
			}
			// ### flat tabs: 50->20, 180->80
		//	painter->fillRect(c_rect, QColor(255, 0, 0, 100));
			Affinity affinity;
			tabAffinity(option->shape, affinity, 1);
			paintThinFrame(painter, c_rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1), option->palette, -50, 180);
			// shadows
			painter->fillRect(QRect(c_rect.right() + 1, c_rect.top(), 1, c_rect.height()), QColor(0, 0, 0, 25));
			painter->fillRect(QRect(c_rect.right() + 2, c_rect.top(), 1, c_rect.height()), QColor(0, 0, 0, 10));
		} else {
			// ### install clip
			painter->save();
			Affinity affinity;
			tabAffinity(option->shape, affinity, -1);
			painter->setClipRect(option->rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1));

			painter->fillRect(c_rect.adjusted(1, mouse ? 1 : 2, -1, -1), mouse ? option->palette.color(QPalette::Window).darker(104) : option->palette.color(QPalette::Window).darker(108));
			paintThinFrame(painter, c_rect.adjusted(1, mouse ? 1 : 2, -1, 1), option->palette, -40, 90);
			// shadows
			painter->fillRect(QRect(c_rect.right(), c_rect.top() + 3, 1, c_rect.height() - 4), QColor(0, 0, 0, 15));
			painter->fillRect(QRect(c_rect.right() + 1, c_rect.top() + 3, 1, c_rect.height() - 4), QColor(0, 0, 0, 5));

			painter->restore();
			// shadow below base
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.bottom() - 1, c_rect.width() - 2, 1), QColor(0, 0, 0, 10));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.bottom() - 2, c_rect.width() - 2, 1), QColor(0, 0, 0, 4));
		}
		break;
	case South:
		c_rect.adjust(1, 0, -2, -1);
		if (option->position != QStyleOptionTab::Beginning
		 && option->position != QStyleOptionTab::OnlyOneTab) {
			c_rect.adjust(-1, 0, 0, 0);
		}
		if (option->state & QStyle::State_Selected) {
			painter->fillRect(c_rect.adjusted(0, 0, -1, 0), option->palette.color(QPalette::Window));
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.bottomLeft());
				gradient.setColorAt(0.0, shaded_color(option->palette.color(QPalette::Window), 0));
				gradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), -5));
				painter->fillRect(c_rect.adjusted(0, 0, -1, 0), gradient);
			}
			Affinity affinity;
			tabAffinity(option->shape, affinity, 1);
			paintThinFrame(painter, c_rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1), option->palette, -50, 180);
			//paintThinFrame(painter, c_rect.adjusted(0, -1, 0, 0), option->palette, -50, 180);
			// shadows
			painter->fillRect(QRect(c_rect.right() + 1, c_rect.top() + 1, 1, c_rect.height() - 1), QColor(0, 0, 0, 25));
			painter->fillRect(QRect(c_rect.right() + 2, c_rect.top() + 1, 1, c_rect.height() - 1), QColor(0, 0, 0, 10));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.bottom() + 1, c_rect.width(), 1), QColor(0, 0, 0, 20));
		} else {
			// ### install clip
			painter->save();
			Affinity affinity;
			tabAffinity(option->shape, affinity, -1);
			painter->setClipRect(option->rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1));

			painter->fillRect(c_rect.adjusted(1, 1, -1, mouse ? -1 : -2), mouse ? option->palette.color(QPalette::Window).darker(104) : option->palette.color(QPalette::Window).darker(108));
			paintThinFrame(painter, c_rect.adjusted(1, -1, -1, mouse ? -1 : -2), option->palette, -40, 90);
			// shadows
			painter->fillRect(QRect(c_rect.right(), c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 15));
			painter->fillRect(QRect(c_rect.right() + 1, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 5));
			if (!mouse) {
				painter->fillRect(QRect(c_rect.left() + 2, c_rect.bottom() - 1, c_rect.width() - 3, 1), QColor(0, 0, 0, 10));
			}

			painter->restore();
			// shadow below base
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.top() + 1, c_rect.width() - 2, 1), QColor(0, 0, 0, 30));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.top() + 2, c_rect.width() - 2, 1), QColor(0, 0, 0, 15));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.top() + 3, c_rect.width() - 2, 1), QColor(0, 0, 0, 8));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.top() + 4, c_rect.width() - 2, 1), QColor(0, 0, 0, 4));
		}
		break;
	case West:
		c_rect.adjust(1, 1, 0, -2);
		if (option->state & QStyle::State_Selected) {
			painter->fillRect(c_rect.adjusted(0, 0, 0, -1), option->palette.color(QPalette::Window));
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.topRight());
				gradient.setColorAt(0.0, shaded_color(option->palette.color(QPalette::Window), 20));
				gradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), 0));
				painter->fillRect(c_rect.adjusted(0, 0, 0, -1), gradient);
			}
			Affinity affinity;
			tabAffinity(option->shape, affinity, 1);
			paintThinFrame(painter, c_rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1), option->palette, -50, 180);
			// shadows
			painter->fillRect(QRect(c_rect.left(), c_rect.bottom() + 1, c_rect.width(), 1), QColor(0, 0, 0, 25));
			painter->fillRect(QRect(c_rect.left(), c_rect.bottom() + 2, c_rect.width(), 1), QColor(0, 0, 0, 10));
		} else {
			// ### install clip
			painter->save();
			Affinity affinity;
			tabAffinity(option->shape, affinity, -1);
			painter->setClipRect(option->rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1));
			painter->fillRect(c_rect.adjusted(mouse ? 1 : 2, 1, 1, -1), mouse ? option->palette.color(QPalette::Window).darker(104) : option->palette.color(QPalette::Window).darker(108));
			paintThinFrame(painter, c_rect.adjusted(mouse ? 1 : 2, 1, 1, -1), option->palette, -40, 90);
			// shadows
			painter->fillRect(QRect(c_rect.left() + 2, c_rect.bottom() + 0, c_rect.width() - 3, 1), QColor(0, 0, 0, 15));
			painter->fillRect(QRect(c_rect.left() + 2, c_rect.bottom() + 1, c_rect.width() - 3, 1), QColor(0, 0, 0, 5));

			painter->restore();
			// shadow below base
			painter->fillRect(QRect(c_rect.right() - 1, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 10));
			painter->fillRect(QRect(c_rect.right() - 2, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 4));
		}
		break;
	case East:
		c_rect.adjust(0, 1, -1, -2);
		if (option->state & QStyle::State_Selected) {
			painter->fillRect(c_rect.adjusted(0, 0, 0, -1), option->palette.color(QPalette::Window));
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.topRight());
				gradient.setColorAt(0.0, shaded_color(option->palette.color(QPalette::Window), 0));
				gradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), 10));
				painter->fillRect(c_rect.adjusted(0, 0, 0, -1), gradient);
			}
			Affinity affinity;
			tabAffinity(option->shape, affinity, 1);
			paintThinFrame(painter, c_rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1), option->palette, -50, 180);
			// shadows
			painter->fillRect(QRect(c_rect.left(), c_rect.bottom() + 1, c_rect.width(), 1), QColor(0, 0, 0, 25));
			painter->fillRect(QRect(c_rect.left(), c_rect.bottom() + 2, c_rect.width(), 1), QColor(0, 0, 0, 10));
			painter->fillRect(QRect(c_rect.right() + 1, c_rect.top() + 1, 1, c_rect.height()), QColor(0, 0, 0, 20));
		} else {
			// ### install clip
			painter->save();
			Affinity affinity;
			tabAffinity(option->shape, affinity, -1);
			painter->setClipRect(option->rect.adjusted(affinity.x2, affinity.y2, affinity.x1, affinity.y1));
			painter->fillRect(c_rect.adjusted(-2, 1, mouse ? -1 : -2, -1), mouse ? option->palette.color(QPalette::Window).darker(104) : option->palette.color(QPalette::Window).darker(108));
			paintThinFrame(painter, c_rect.adjusted(-2, 1, mouse ? -1 : -2, -1), option->palette, -40, 90);
			// shadows
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.bottom() + 0, c_rect.width() - 3, 1), QColor(0, 0, 0, 15));
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.bottom() + 1, c_rect.width() - 3, 1), QColor(0, 0, 0, 5));

			painter->restore();
			// shadow below base
			painter->fillRect(QRect(c_rect.left() + 1, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 20));
			painter->fillRect(QRect(c_rect.left() + 2, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 10));
			painter->fillRect(QRect(c_rect.left() + 3, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 5));
		//	painter->fillRect(QRect(c_rect.left() + 4, c_rect.top() + 1, 1, c_rect.height() - 2), QColor(0, 0, 0, 4));
		}
		break;
	}
}


void paintTabBarTabLabel(QPainter *painter, const QStyleOptionTab *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
//	bool mouse = (option->state & QStyle::State_MouseOver) && !(option->state & QStyle::State_Selected) && (option->state & QStyle::State_Enabled);
//	painter->save();
#if 0
	if (!(option->state & QStyle::State_Selected)) {
		if (mouse) {
			painter->setOpacity(0.7);
		} else {
			painter->setOpacity(0.5);
		}
	} else {
		painter->setOpacity(0.8);
	}
//	QFont font(painter->font());
//	font.setBold(true);
//	painter->setFont(font);
#endif
	QStyleOptionTabV2 opt;

	int offset = TAB_SHIFT;
	if (option->state & QStyle::State_Selected || (option->state & QStyle::State_MouseOver && option->state & QStyle::State_Enabled)) {
		offset = 0;
	}

	if (option->version > 1) {
		opt = *((QStyleOptionTabV2 *) option);
	} else {
		opt = *option;
	}
	Affinity affinity;
	tabAffinity(option->shape, affinity, offset);
	opt.rect.translate(affinity.x1 + affinity.x2, affinity.y1 + affinity.y2);
	switch (tabPos(option->shape)) {
	case North:
		opt.rect.adjust(3, 1, -6, 1);
		break;
	case South:
		opt.rect.adjust(3, 0, -6, 0);
		break;
	case West:
	case East:
		painter->save();
		QMatrix mat;
		if (tabPos(option->shape) == West) {
			opt.rect.adjust(3, 4, 3, -4);
		} else {
			opt.rect.adjust(-1, 4, -1, -4);
		}
		QPointF c = opt.rect.center();
		mat.translate(c.x(), c.y());
		mat.rotate(tabPos(option->shape) == West ? -90 : 90);
		mat.translate(-c.x(), -c.y());
		opt.rect = mat.mapRect(opt.rect);
		painter->setMatrix(mat, true);
		opt.shape = (QTabBar::Shape) 0;
		break;
	}
	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_TabBarTabLabel, &opt, painter, widget);
	if (isVertical(option->shape)) {
		painter->restore();
	}
}

/*-----------------------------------------------------------------------*/
#if 0
class IconTextLabel
{
	QString text;
	QIcon icon;
	QSize iconSize;
	Qt::Orientation orientation;
};


static void paintIconTextLabel(QPainter *painter, const QRect &rect, const IconTextLabel &label)
{

}
#endif

/*
 * skulpture_text.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QTextEdit>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtGui/QPlainTextEdit>
#endif
#include <QtGui/QTextBrowser>
#include <QtGui/QTextFrame>
#include <QtGui/QScrollBar>
#include <cstdio>


/*-----------------------------------------------------------------------*/

#define HIGHLIGHT_VMARGIN	2

void SkulptureStyle::Private::highlightCurrentEditLine(QAbstractScrollArea *edit, const QRect &cursorRect)
{
	QRect cursorLine = cursorRect;
	cursorLine.setLeft(0);
	cursorLine.setWidth(edit->viewport()->width());
	cursorLine.adjust(0, - HIGHLIGHT_VMARGIN, 0, HIGHLIGHT_VMARGIN);
	cursorLine.translate(edit->horizontalScrollBar()->value(), edit->verticalScrollBar()->value());
	cursorLine.moveTop(cursorLine.top() % edit->height());
	if (cursorRect.top() < HIGHLIGHT_VMARGIN) {
		cursorLine.setTop(cursorLine.top() + HIGHLIGHT_VMARGIN - cursorRect.top());
	}
	if (cursorRect.bottom() + HIGHLIGHT_VMARGIN > edit->rect().bottom()) {
		cursorLine.setBottom(cursorLine.bottom() - cursorRect.bottom() - HIGHLIGHT_VMARGIN + edit->rect().bottom());
	}
	if (edit != oldEdit || cursorLine.top() != oldCursorTop
		   || cursorLine.width() != oldCursorWidth
		   || cursorLine.height() != oldCursorHeight
		   || edit->viewport()->height() != oldHeight
		   || edit->palette() != oldPalette) {
		oldEdit = edit;
		oldCursorTop = cursorLine.top();
		oldCursorWidth = cursorLine.width();
		oldCursorHeight = cursorLine.height();
		oldHeight = edit->viewport()->height();
		oldPalette = edit->palette();
		QPixmap pixmap(edit->size());
		QPalette palette = edit->palette();
		pixmap.fill(palette.color(QPalette::Base));
		QPainter painter(&pixmap);
		QColor color;
		bool wrap = false;
		do {
			color = palette.color(QPalette::Highlight);
			color.setAlpha(40);
		//	color.setHsvF(palette.color(QPalette::Highlight).hueF(), 0.05, 0.97);
			painter.fillRect(cursorLine, color);
			if (edit->window()->testAttribute(Qt::WA_KeyboardFocusChange)) {
				color = palette.color(QPalette::Highlight).dark(120);
				color.setAlpha(120);
				painter.fillRect(cursorLine.adjusted(0, cursorLine.height() - 3, 0, -2), color);
			}
			if (!wrap) {
				if (cursorLine.bottom() > edit->rect().bottom()) {
					cursorLine.translate(0, -edit->height());
					wrap = true;
				}
			} else {
				wrap = false;
			}
		} while (wrap);
		QBrush brush(pixmap);
		palette.setBrush(QPalette::Base, brush);
		edit->viewport()->setPalette(palette);
	}
}

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void SkulptureStyle::Private::highlightCurrentEditLine(QPlainTextEdit *edit)
{
	if (edit->hasFocus() && !edit->isReadOnly() && edit->cursorRect().intersects(edit->rect())) {
		QStyleOption option;
		option.initFrom(edit);
		int cursorWidth = q->SkulptureStyle::pixelMetric(PM_TextCursorWidth, &option, edit);
		if (edit->cursorWidth() != cursorWidth) {
			edit->setCursorWidth(cursorWidth);
		}
		highlightCurrentEditLine(edit, edit->cursorRect());
	} else {
		QPalette palette = edit->palette();
		edit->viewport()->setPalette(palette);
		oldEdit = 0;
	}
}
#endif

void SkulptureStyle::Private::highlightCurrentEditLine(QTextEdit *edit)
{
	if (edit->hasFocus() && !edit->isReadOnly() && edit->cursorRect().intersects(edit->rect())) {
		QStyleOption option;
		option.initFrom(edit);
		int cursorWidth = q->SkulptureStyle::pixelMetric(PM_TextCursorWidth, &option, edit);
		if (edit->cursorWidth() != cursorWidth) {
			edit->setCursorWidth(cursorWidth);
		}
		highlightCurrentEditLine(edit, edit->cursorRect());
	} else {
		QPalette palette = edit->palette();
		edit->viewport()->setPalette(palette);
		oldEdit = 0;
	}
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::updateTextEditMargins(QTextEdit *edit)
{
	int margin = 1 + edit->fontMetrics().height() / 5;
	if (margin > 4) margin = 4;
	if (qobject_cast<QTextBrowser *>(edit)) {
		margin = edit->fontMetrics().height();
		if (margin < 4 || edit->height() < 4 * edit->fontMetrics().height()) {
			margin = 4;
		}
	}
	if (margin < 2 || edit->height() < 2 * edit->fontMetrics().height()) {
		margin = 2;
	}
	QTextDocument *doc = edit->document();
//	printf("doc: %p\n", doc);
	if (!doc) return;
	if (doc->isEmpty()) {
		// create valid root frame
		QTextCursor cursor(doc);
	}

	QTextFrame *root = doc->rootFrame();
//	printf("root: %p\n", root);
	if (!root) return;

	QTextFrameFormat format = root->frameFormat();
	if (!format.isValid()) return;

	if (format.margin() == 2.0 && margin != 2) {
	//	printf("set margin %d\n", margin);
		// ### crash on setText(), disable signals
		disconnect(edit, SIGNAL(textChanged()), &mapper, SLOT(map()));
		doc->blockSignals(true);
		format.setMargin(margin);
		root->setFrameFormat(format);
	//	edit->insertPlainText(QString::fromUtf8(""));
	//	edit->update();
		doc->blockSignals(false);
		connect(edit, SIGNAL(textChanged()), &mapper, SLOT(map()));
		// clear undo buffer
		bool undo = edit->isUndoRedoEnabled();
		edit->setUndoRedoEnabled(false);
		doc->setModified(false);
	//	emit doc->contentsChanged();
		edit->setUndoRedoEnabled(undo);
		// force relayout
		edit->resize(edit->size() + QSize(-1, 0));
		edit->resize(edit->size() + QSize(1, 0));
	}
}


void SkulptureStyle::Private::textEditSourceChanged(QWidget *widget)
{
	if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
		updateTextEditMargins(edit);
	}
}


/*-----------------------------------------------------------------------*/

QRect SkulptureStyle::itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const
{
	Q_UNUSED(enabled);
	return ParentStyle::itemTextRect(metrics, rectangle, alignment, true, text);
//	return ParentStyle::itemTextRect(metrics, rectangle, alignment, enabled, text);
}


void SkulptureStyle::drawItemText(QPainter * painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
	if (1 || enabled) {
	//	painter->save();
	//	QFont font = painter->font();
	//	font.setStretch(600);
	//	painter->setFont(font);
	//	painter->scale(1/24.0, 1);
#if 0
		if (enabled) {
			painter->save();
			painter->setPen(QColor(255, 255, 255, 100));
			ParentStyle::drawItemText(painter, rectangle.adjusted(1, 1, 1, 1), alignment, palette, enabled, text, QPalette::NoRole);
			painter->restore();
		}
#endif
		ParentStyle::drawItemText(painter, rectangle, alignment, palette, enabled, text, textRole);
	//	painter->restore();
	} else {
		QImage image(rectangle.size() + QSize(4, 4), QImage::Format_ARGB32_Premultiplied);
		image.fill(0);
		QPainter p(&image);
		p.setFont(painter->font());
		p.setPen(painter->pen());

		ParentStyle::drawItemText(&p, QRect(QPoint(3, 3), rectangle.size()), alignment, palette, enabled, text, QPalette::Light);
//		ParentStyle::drawItemText(&p, QRect(QPoint(1, 3), rectangle.size()), alignment, palette, enabled, text, QPalette::Light);
//		ParentStyle::drawItemText(&p, QRect(QPoint(3, 1), rectangle.size()), alignment, palette, enabled, text, QPalette::Light);
//		ParentStyle::drawItemText(&p, QRect(QPoint(1, 1), rectangle.size()), alignment, palette, enabled, text, QPalette::Light);
//		filterImage(image, 0.5);
		ParentStyle::drawItemText(&p, QRect(QPoint(2, 2), rectangle.size()), alignment, palette, enabled, text, textRole);
//		filterImage(image, 0.75);
		painter->drawImage(rectangle.topLeft() - QPoint(2, 2), image);
	}
}


/*
 * skulpture_toolbar.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>
#include <QtGui/QTabBar>
#include <cstdio>


/*-----------------------------------------------------------------------*/

#define PAINT_SEPARATOR 0


/*-----------------------------------------------------------------------*/

void paintToolBarSeparator(QPainter *painter, const QStyleOptionToolBar *option)
{
#if PAINT_SEPARATOR
	QRect rect = option->rect;

	if (option->state & QStyle::State_Horizontal) {
	//	rect.adjust(2, 3, -2, -3);
		rect.adjust(2, -1, -2, 1);
	} else {
	//	rect.adjust(3, 2, -3, -2);
		rect.adjust(-1, 2, 1, -2);
	}
	paintThinFrame(painter, rect, option->palette, 60, -20);
#else
	Q_UNUSED(painter);
	Q_UNUSED(option);
#endif
}

/*-----------------------------------------------------------------------*/

extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole);

void paintToolBarHandle(QPainter *painter, const QStyleOptionToolBar *option)
{
#if 1
	int d = 5;
	QRect rect(QRect(option->rect).center() - QPoint(d / 2, d / 2), QSize(d, d));
	QStyleOption iOption;
	iOption.QStyleOption::operator=(*option);
	iOption.rect = rect;
	iOption.palette.setCurrentColorGroup(QPalette::Disabled);
//	iOption.state &= ~QStyle::State_Enabled;
	iOption.palette.setColor(QPalette::Button, iOption.palette.color(QPalette::Window));
	paintCachedGrip(painter, &iOption, QPalette::Window);
#else
	QRect rect = option->rect;

	if (option->state & QStyle::State_Horizontal) {
		rect.adjust(2, 2, -2, -2);
#if PAINT_SEPARATOR
		rect.adjust(0, 1, 0, 1);
#endif
	} else {
		rect.adjust(2, 2, -2, -2);
#if PAINT_SEPARATOR
		rect.adjust(1, 0, 1, 0);
#endif
	}
	paintThinFrame(painter, rect.adjusted(-1, -1, 1, 1), option->palette, 60, -20);
	paintThinFrame(painter, rect, option->palette, -30, 80);
#endif
}


/*-----------------------------------------------------------------------*/

void paintPanelToolBar(QPainter *painter, const QStyleOptionToolBar *option)
{
//	painter->fillRect(option->rect, option->palette.color(QPalette::Window));
//	paintThinFrame(painter, option->rect, option->palette, -20, 60);
#if PAINT_SEPARATOR
	QRect r = option->rect;
	if (option->state & QStyle::State_Horizontal) {
		r.setHeight(2);
	} else {
		r.setWidth(2);
	}
	paintThinFrame(painter, r, option->palette, 60, -20);
#else
	Q_UNUSED(painter);
	Q_UNUSED(option);
#endif
}


/*-----------------------------------------------------------------------*/

static inline bool inVerticalToolBar(const QStyleOption *option, const QWidget *widget)
{
	// ### option->state does not reflect orientation
	Q_UNUSED(option);
	bool verticalBar = false;

	if (widget && widget->parentWidget()) {
		const QToolBar *toolBar = qobject_cast<const QToolBar *>(widget->parentWidget());
		//	printf("toolbar: %p\n", toolBar);
		if (toolBar && toolBar->orientation() == Qt::Vertical) {
			verticalBar = true;
		}
	}
	return verticalBar;
}


/*-----------------------------------------------------------------------*/

void paintToolButton(QPainter *painter, const QStyleOptionToolButton *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(bgrole);

	if (widget) {
		QTabBar *bar = qobject_cast<QTabBar *>(widget->parentWidget());
		if (bar) {
			// tabbar scroll button
			QStyleOptionToolButton opt = *option;
			if (int(bar->shape()) & 2) {
				opt.rect.adjust(4, 0, -4, -1);
			} else {
				opt.rect.adjust(0, 4, 0, -3);
			}
			painter->save();
			painter->setClipRect(opt.rect);
			painter->fillRect(opt.rect, option->palette.color(QPalette::Window));
			((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, &opt, painter, widget);
			painter->restore();
			return;
		} else if (widget->objectName() == QLatin1String("qt_menubar_ext_button") || widget->objectName() == QLatin1String("qt_toolbar_ext_button")) {
			QStyleOptionToolButton opt = *option;
			/* do not render menu arrow, because extension buttons already have an arrow */
			opt.features &= ~(QStyleOptionToolButton::Menu | QStyleOptionToolButton::HasMenu);
			((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, &opt, painter, widget);
			return;
		}
	}
	if (option->features & QStyleOptionToolButton::HasMenu) {
		if (option->features & QStyleOptionToolButton::Menu) {
			if (option->subControls & QStyle::SC_ToolButton) {
				painter->save();
				QStyleOptionToolButton opt = *option;
				opt.rect = style->subControlRect(QStyle::CC_ToolButton, option, QStyle::SC_ToolButton, widget);
				// opt.features &= ~(QStyleOptionToolButton::Menu | QStyleOptionToolButton::HasMenu | QStyleOptionToolButton::Arrow);
				opt.arrowType = Qt::NoArrow;
				opt.features = 0;
				opt.subControls &= ~(QStyle::SC_ToolButtonMenu);
				opt.activeSubControls &= ~(QStyle::SC_ToolButtonMenu);
				if (inVerticalToolBar(option, widget)) {
					painter->setClipRect(opt.rect.adjusted(0, 0, 0, -1));
				} else {
					painter->setClipRect(opt.rect.adjusted(option->direction == Qt::LeftToRight ? 0 : 1, 0, option->direction == Qt::LeftToRight ? -1 : 0, 0));
				}
				((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, &opt, painter, widget);
				painter->restore();
			}
			if (option->subControls & QStyle::SC_ToolButtonMenu) {
				painter->save();
				QStyleOptionToolButton opt = *option;
				opt.rect = style->subControlRect(QStyle::CC_ToolButton, option, QStyle::SC_ToolButtonMenu, widget);
				QStyle::State state = option->state;
				state &= ~(QStyle::State_Sunken | QStyle::State_Raised);
				if (!(state & QStyle::State_AutoRaise) || (state & QStyle::State_MouseOver)) {
					state |= QStyle::State_Raised;
				}
				if (option->activeSubControls & QStyle::SC_ToolButtonMenu) {
					state |= QStyle::State_Sunken;
				}
				opt.state = state;
				if (inVerticalToolBar(option, widget)) {
					painter->setClipRect(opt.rect.adjusted(0, 1, 0, 0));
				} else {
					painter->setClipRect(opt.rect.adjusted(option->direction == Qt::LeftToRight ? 1 : 0, 0, option->direction == Qt::LeftToRight ? 0 : -1, 0));
				}
				if (state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {
					style->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, painter, widget);
				}
				painter->restore();
				QRect r;
				if (inVerticalToolBar(option, widget)) {
					if (option->direction == Qt::LeftToRight) {
						r = QRect(opt.rect.right() - 9, opt.rect.top(), 7, opt.rect.height());
					} else {
						r = QRect(3, opt.rect.top(), 7, opt.rect.height());
					}
				} else {
					r = QRect(opt.rect.left(), opt.rect.bottom() - 9, opt.rect.width(), 7);
				}
				if (option->state & QStyle::State_Sunken) {
					if (option->direction == Qt::LeftToRight) {
						r.adjust(1, 1, 1, 1);
					} else {
						r.adjust(-1, 1, -1, 1);
					}
				}
				QFont font;
				font.setPixelSize(9);
				opt.fontMetrics = QFontMetrics(font);
				opt.rect = r;
				style->drawPrimitive(inVerticalToolBar(option, widget) ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
			}
		//	painter->fillRect(opt.rect.adjusted(3, 3, -3, -3), Qt::red);
		} else {
			QStyleOptionToolButton opt = *option;
			opt.features &= ~QStyleOptionToolButton::HasMenu;
			((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, &opt, painter, widget);
			QRect r;
			if (option->direction == Qt::LeftToRight) {
				r = QRect(option->rect.right() - 6, option->rect.bottom() - 6, 5, 5);
				if (option->state & QStyle::State_Sunken) {
					r.adjust(1, 1, 1, 1);
				}
			} else {
				r = QRect(2, option->rect.bottom() - 6, 5, 5);
				if (option->state & QStyle::State_Sunken) {
					r.adjust(-1, 1, -1, 1);
				}
			}
			QFont font;
			font.setPixelSize(7);
			opt.fontMetrics = QFontMetrics(font);
			opt.rect = r;
			style->drawPrimitive(inVerticalToolBar(option, widget) ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
		}
	} else {
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, option, painter, widget);
	}
}


/*-----------------------------------------------------------------------*/

QRect subControlRectToolButton(const QStyleOptionToolButton *option, QStyle::SubControl subControl, const QWidget *widget, void */*data*/, int /*id*/, const QStyle *style)
{
	QRect r = option->rect;

	if (option->features & QStyleOptionToolButton::Menu) {
		int pm = style->pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget) - 2;
		bool verticalBar = inVerticalToolBar(option, widget);
		switch (subControl) {
			case QStyle::SC_ToolButton:
				if (verticalBar) {
					r.adjust(0, 0, 0, -pm);
				} else {
					r.adjust(0, 0, -pm, 0);
				}
				break;
			case QStyle::SC_ToolButtonMenu:
				if (verticalBar) {
					r.adjust(0, r.height() - pm - 2, 0, 0);
				} else {
					r.adjust(r.width() - pm - 2, 0, 0, 0);
				}
				break;
			default:
				break;
		}
		return style->visualRect(option->direction, option->rect, r);
	}
	return ((QPlastiqueStyle *) style)->QPlastiqueStyle::subControlRect(QStyle::CC_ToolButton, option, subControl, widget);
}


extern QSize sizeFromContentsToolButton(const QStyleOptionToolButton *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style)
{
	QSize size = ((QPlastiqueStyle *) style)->QPlastiqueStyle::sizeFromContents(QStyle::CT_ToolButton, option, contentsSize, widget);

	if (widget && !qstrcmp(widget->metaObject()->className(), "KAnimatedButton")) {
		return size - QSize(4, 4);
	}
	if (option->features & QStyleOptionToolButton::Menu) {
		int pm = style->pixelMetric(QStyle::PM_MenuButtonIndicator, option, widget);
		size -= QSize(pm, 0);
		pm -= 2;
		bool verticalBar = inVerticalToolBar(option, widget);

		if (verticalBar) {
			size += QSize(0, pm);
		} else {
			size += QSize(pm, 0);
		}
	}
	return size;
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::updateToolBarOrientation(Qt::Orientation /*orientation */)
{
	QToolBar *toolbar = static_cast<QToolBar *>(sender());
	QList<QToolButton *> toolbuttons = toolbar->findChildren<QToolButton *>();
	bool changed = false;

	foreach (QToolButton *toolbutton, toolbuttons) {
		if (toolbutton->popupMode() == QToolButton::MenuButtonPopup) {
			// ### this hack forces Qt to invalidate the size hint
			Qt::ToolButtonStyle oldstyle = toolbutton->toolButtonStyle();
			Qt::ToolButtonStyle newstyle;
			if (oldstyle == Qt::ToolButtonIconOnly) {
				newstyle = Qt::ToolButtonTextOnly;
			} else {
				newstyle = Qt::ToolButtonIconOnly;
			}
			toolbutton->setToolButtonStyle(newstyle);
			toolbutton->setToolButtonStyle(oldstyle);
			changed = true;
		}
	}
	if (changed) {
		// ### Qt does not update dragged toolbars...
		toolbar->updateGeometry();
	}
}


/*
 * skulpture_p.cpp
 *
 */

#include "skulpture_p.h"
#include <QtCore/QSettings>
#include <QtCore/QLocale>
#include <QtGui/QFrame>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#include <QtGui/QToolBar>
#include <cstdio>


/*-----------------------------------------------------------------------*/

struct StyleSetting
{
	enum Type {
		Bool,
  		Char,
    		Frame,
       		Alignment,
	  	Orientation,
     		Pixels,
        	Points,
	  	Milliseconds,
     		Color,
       		Size,
	 	Parent,
   		Value
	};

	const char * const label;
	int id;
	int type;
	int value;
};


static const struct StyleSetting styleSettings[] =
{
// changed from defaults
// { "TabWidget/TabBarAlignment", QStyle::SH_TabBar_Alignment, StyleSetting::Alignment, Qt::AlignCenter },
// { "GroupBox/TextLabelColor", QStyle::SH_GroupBox_TextLabelColor, StyleSetting::Color, 0xFF000000 },
// { "ItemView/GridLineColor", QStyle::SH_Table_GridLineColor, StyleSetting::Color, 0xFFD0D0D0 },
 { "Menu/SubMenuPopupDelay", QStyle::SH_Menu_SubMenuPopupDelay, StyleSetting::Milliseconds, 80 },
// { "General/FullWidthSelection", QStyle::SH_RichText_FullWidthSelection, StyleSetting::Bool, 1 },
 { "LineEdit/PasswordCharacter", QStyle::SH_LineEdit_PasswordCharacter, StyleSetting::Char, 10039 },  /* other useful values: 9096; 10039; 9679; 8226; 9055; 9675; 9642; 9643; 9674; 9675; 9688; 9689; 9633; 9632; 9702; 9733; 9830; 9786; */
// { "Dialog/ButtonsHaveIcons", QStyle::SH_DialogButtonBox_ButtonsHaveIcons, StyleSetting::Bool, 0 },
// { "ItemView/ActivateItemOnSingleClick", QStyle::SH_ItemView_ActivateItemOnSingleClick, StyleSetting::Bool, 1 },
// { "ItemView/FrameOnlyAroundContents", QStyle::SH_ScrollView_FrameOnlyAroundContents, StyleSetting::Bool, 0 },
// { "General/UnderlineShortcut", QStyle::SH_UnderlineShortcut, StyleSetting::Bool, 0 },
// { "ToolBox/SelectedPageTitleBold", QStyle::SH_ToolBox_SelectedPageTitleBold, StyleSetting::Bool, false },
#if 0 // configurable
 { "General/EtchDisabledText", QStyle::SH_EtchDisabledText, StyleSetting::Parent, 0 },
 { "General/DitherDisabledText", QStyle::SH_DitherDisabledText, StyleSetting::Parent, 0 },
 { "ScrollBar/ContextMenu", QStyle::SH_ScrollBar_ContextMenu, StyleSetting::Parent, 0 },
 { "ScrollBar/MiddleClickAbsolutePosition", QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, StyleSetting::Parent, 0 },
 { "ScrollBar/LeftClickAbsolutePosition", QStyle::SH_ScrollBar_LeftClickAbsolutePosition, StyleSetting::Parent, 0 },
 { "ScrollBar/ScrollWhenPointerLeavesControl", QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl, StyleSetting::Parent, 0 },
 { "ScrollBar/RollBetweenButtons", QStyle::SH_ScrollBar_RollBetweenButtons, StyleSetting::Parent, 0 },
 { "TabWidget/TabBarAlignment", QStyle::SH_TabBar_Alignment, StyleSetting::Parent, 0 },
 { "ItemView/HeaderArrowAlignment", QStyle::SH_Header_ArrowAlignment, StyleSetting::Parent, 0 },
 { "Slider/SnapToValue", QStyle::SH_Slider_SnapToValue, StyleSetting::Parent, 0 },
 { "Slider/SloppyKeyEvents", QStyle::SH_Slider_SloppyKeyEvents, StyleSetting::Parent, 0 },
 { "ProgressDialog/CenterCancelButton", QStyle::SH_ProgressDialog_CenterCancelButton, StyleSetting::Parent, 0 },
 { "ProgressDialog/TextLabelAlignment", QStyle::SH_ProgressDialog_TextLabelAlignment, StyleSetting::Parent, 0 },
 { "PrintDialog/RightAlignButtons", QStyle::SH_PrintDialog_RightAlignButtons, StyleSetting::Parent, 0 },
 { "Window/SpaceBelowMenuBar", QStyle::SH_MainWindow_SpaceBelowMenuBar, StyleSetting::Parent, 0 },
 { "FontDialog/SelectAssociatedText", QStyle::SH_FontDialog_SelectAssociatedText, StyleSetting::Parent, 0 },
 { "Menu/KeyboardSearch", QStyle::SH_Menu_KeyboardSearch, StyleSetting::Parent, 0 },
 { "Menu/AllowActiveAndDisabled", QStyle::SH_Menu_AllowActiveAndDisabled, StyleSetting::Parent, 0 },
 { "Menu/SpaceActivatesItem", QStyle::SH_Menu_SpaceActivatesItem, StyleSetting::Parent, 0 },
 { "Menu/Scrollable", QStyle::SH_Menu_Scrollable, StyleSetting::Parent, 0 },
 { "Menu/SloppySubMenus", QStyle::SH_Menu_SloppySubMenus, StyleSetting::Parent, 0 },
 { "Menu/AltKeyNavigation", QStyle::SH_MenuBar_AltKeyNavigation, StyleSetting::Parent, 0 },
 { "ComboBox/ListMouseTracking", QStyle::SH_ComboBox_ListMouseTracking, StyleSetting::Parent, 0 },
 { "Menu/MouseTracking", QStyle::SH_Menu_MouseTracking, StyleSetting::Parent, 0 },
 { "Menu/BarMouseTracking", QStyle::SH_MenuBar_MouseTracking, StyleSetting::Parent, 0 },
 { "Menu/FillScreenWithScroll", QStyle::SH_Menu_FillScreenWithScroll, StyleSetting::Parent, 0 },
 { "Menu/SelectionWrap", QStyle::SH_Menu_SelectionWrap, StyleSetting::Parent, 0 },
 { "ItemView/ChangeHighlightOnFocus", QStyle::SH_ItemView_ChangeHighlightOnFocus, StyleSetting::Parent, 0 },
 { "Window/ShareActivation", QStyle::SH_Widget_ShareActivation, StyleSetting::Parent, 0 },
 { "TabWidget/SelectMouseType", QStyle::SH_TabBar_SelectMouseType, StyleSetting::Parent, 0 },
 { "ItemView/Compat/ExpansionSelectMouseType", QStyle::SH_Q3ListViewExpand_SelectMouseType, StyleSetting::Parent, 0 },
 { "TabWidget/TabBarPreferNoArrows", QStyle::SH_TabBar_PreferNoArrows, StyleSetting::Parent, 0 },
 { "ComboBox/Popup", QStyle::SH_ComboBox_Popup, StyleSetting::Parent, 0 },
 { "MDI/Workspace/FillSpaceOnMaximize", QStyle::SH_Workspace_FillSpaceOnMaximize, StyleSetting::Parent, 0 },
 { "Slider/StopMouseOverSlider", QStyle::SH_Slider_StopMouseOverSlider, StyleSetting::Parent, 0 },
 { "General/BlinkCursorWhenTextSelected", QStyle::SH_BlinkCursorWhenTextSelected, StyleSetting::Parent, 0 },
 { "GroupBox/TextLabelVerticalAlignment", QStyle::SH_GroupBox_TextLabelVerticalAlignment, StyleSetting::Parent, 0 },
 { "Dialog/DefaultButton", QStyle::SH_DialogButtons_DefaultButton, StyleSetting::Parent, 0 },
 { "General/SpellCheckUnderlineStyle", QStyle::SH_SpellCheckUnderlineStyle, StyleSetting::Parent, 0 },
 { "SpinBox/AnimateButton", QStyle::SH_SpinBox_AnimateButton, StyleSetting::Parent, 0 },
 { "SpinBox/KeyPressAutoRepeatRate", QStyle::SH_SpinBox_KeyPressAutoRepeatRate, StyleSetting::Parent, 0 },
 { "SpinBox/ClickAutoRepeatRate", QStyle::SH_SpinBox_ClickAutoRepeatRate, StyleSetting::Parent, 0 },
 { "ToolTip/Opacity", QStyle::SH_ToolTipLabel_Opacity, StyleSetting::Parent, 0 },
 { "Menu/DrawMenuBarSeparator", QStyle::SH_DrawMenuBarSeparator, StyleSetting::Parent, 0 },
 { "MDI/TitleBar/ModifyNotification", QStyle::SH_TitleBar_ModifyNotification, StyleSetting::Parent, 0 },
 { "Button/FocusPolicy", QStyle::SH_Button_FocusPolicy, StyleSetting::Parent, 0 },
 { "Menu/DismissOnSecondClick", QStyle::SH_MenuBar_DismissOnSecondClick, StyleSetting::Parent, 0 },
 { "MessageBox/UseBorderForButtonSpacing", QStyle::SH_MessageBox_UseBorderForButtonSpacing, StyleSetting::Parent, 0 },
 { "MessageBox/CenterButtons", QStyle::SH_MessageBox_CenterButtons, StyleSetting::Parent, 0 },
 { "MessageBox/AllowTextInteraction", QStyle::SH_MessageBox_TextInteractionFlags, StyleSetting::Parent, 0 },
 { "MDI/TitleBar/AutoRaise", QStyle::SH_TitleBar_AutoRaise, StyleSetting::Parent, 0 },
 { "ToolBar/PopupDelay", QStyle::SH_ToolButton_PopupDelay, StyleSetting::Parent, 0 },
 { "SpinBox/DisableControlsOnBounds", QStyle::SH_SpinControls_DisableOnBounds, StyleSetting::Parent, 0 },
 { "ComboBox/LayoutDirection", QStyle::SH_ComboBox_LayoutDirection, StyleSetting::Parent, 0 },
 { "ItemView/EllipsisLocation", QStyle::SH_ItemView_EllipsisLocation, StyleSetting::Parent, 0 },
 { "ItemView/ShowDecorationSelected", QStyle::SH_ItemView_ShowDecorationSelected, StyleSetting::Parent, 0 },
 { "Slider/AbsoluteSetButtons", QStyle::SH_Slider_AbsoluteSetButtons, StyleSetting::Parent, 0 },
 { "Slider/PageSetButtons", QStyle::SH_Slider_PageSetButtons, StyleSetting::Parent, 0 },
 { "TabWidget/ElideMode", QStyle::SH_TabBar_ElideMode, StyleSetting::Parent, 0 },
 { "Dialog/ButtonLayout", QStyle::SH_DialogButtonLayout, StyleSetting::Parent, 0 },
 { "ComboBox/PopupFrameStyle", QStyle::SH_ComboBox_PopupFrameStyle, StyleSetting::Parent, 0 },
 { "ItemView/MovementWithoutUpdatingSelection", QStyle::SH_ItemView_MovementWithoutUpdatingSelection, StyleSetting::Parent, 0 },
 { "General/FocusFrameAboveWidget", QStyle::SH_FocusFrame_AboveWidget, StyleSetting::Parent, 0 },
 { "General/FocusIndicatorTextCharFormat", QStyle::SH_TextControl_FocusIndicatorTextCharFormat, StyleSetting::Parent, 0 },
#endif
#if 0 // not configurable
// GUIStyle ???
 { "General/FocusFrameMask", QStyle::SH_FocusFrame_Mask, StyleSetting::Parent, 0 },
 { "General/RubberBandMask", QStyle::SH_RubberBand_Mask, StyleSetting::Parent, 0 },
 { "General/WindowFrameMask", QStyle::SH_WindowFrame_Mask, StyleSetting::Parent, 0 },
 { "MDI/TitleBar/NoBorder", QStyle::SH_TitleBar_NoBorder, StyleSetting::Parent, 0 },
 { "Dial/BackgroundRole", QStyle::SH_Dial_BackgroundRole, StyleSetting::Parent, 0 },
 { "ScrollBar/BackgroundMode", QStyle::SH_ScrollBar_BackgroundMode, StyleSetting::Parent, 0 },
 { "General/ToolTipMask", QStyle::SH_ToolTip_Mask, StyleSetting::Parent, 0 },
#endif
#if 0
 // New for SkulptureStyle (not supported yet)
 { "LineEdit/EnableClearButton", -1, StyleSetting::Bool, 1 },
 { "LineEdit/ClearButtonExtent", -1, StyleSetting::Pixels, 20 },
 { "LineEdit/ClearButtonIconSize", -1, StyleSetting::Size, 12 },
 { "General/HighlightCursorLine", -1, StyleSetting::Bool, 1 },
 { "General/HighlightCursorLineWithoutFocus", -1, StyleSetting::Bool, 1 },
 { "General/UnderlineShortcutWhenAltPressed", -1, StyleSetting::Bool, 1 },
#endif
 { 0, -1, 0, 0 }
};


extern int getRubberBandMask(QStyleHintReturnMask *mask, const QStyleOption *option, const QWidget *widget);
extern int getWindowFrameMask(QStyleHintReturnMask *mask, const QStyleOptionTitleBar *option, const QWidget *widget);

int SkulptureStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
//	return ParentStyle::styleHint(hint, option, widget, returnData);
	// TODO implement caching
	const StyleSetting *setting = &styleSettings[0];
	QVariant value;

	switch (hint) {
		case QStyle::SH_Table_GridLineColor: {
				QColor bgcolor;
				if (option) {
					bgcolor = option->palette.color(QPalette::Base);
				} else {
					bgcolor = QApplication::palette().color(QPalette::Base);
				}
				QColor color = bgcolor.dark(120);
				int r, g, b, a;
				color.getRgb(&r, &g, &b, &a);
				return (int) qRgba(r, g, b, a);
			}
/*		case QStyle::SH_ComboBox_ListMouseTracking:
			return true;
*/		case QStyle::SH_UnderlineShortcut:
			if (d->hideShortcutUnderlines) {
				return (d->shortcut_handler->underlineShortcut(widget));
			} else {
				return true;
			}
		case QStyle::SH_TitleBar_NoBorder:
			return 0;
		case QStyle::SH_RubberBand_Mask: {
				QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData);
				if (mask) {
					return getRubberBandMask(mask, option, widget);
				}
			}
			return 0;
		case QStyle::SH_WindowFrame_Mask: {
				QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData);
				const QStyleOptionTitleBar *titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
				if (mask && titleBarOption) {
					return getWindowFrameMask(mask, titleBarOption, widget);
				}
			}
			return 0;
		case QStyle::SH_MainWindow_SpaceBelowMenuBar: {
#if 0
				if (widget) {
					QMainWindow *window = qobject_cast<QMainWindow *>(widget->parentWidget());
					if (window) {
						QList<QToolBar *> toolBars = window->findChildren<QToolBar *>();
						foreach (QToolBar *toolBar, toolBars) {
							if (window->toolBarArea(toolBar) == Qt::TopToolBarArea) {
								if (!toolBar->isFloating()) {
									printf("has top toolbar\n");
									return 2;
								}
							}
						}
					}
				}
#endif
			}
			return 0;
		default:
			break;
	}

	while (setting->label) {
		if (setting->id == int(hint)) {
			break;
		}
		++setting;
	}
	if (setting->label) {
		value = setting->value;
		switch (setting->type) {
			case StyleSetting::Parent:
				value = ParentStyle::styleHint(hint, option, widget, returnData);
				break;
			case StyleSetting::Bool:
				value = setting->value != 0;
				break;
			case StyleSetting::Char:
				value = QString(QChar(setting->value));
				break;
			case StyleSetting::Color:
				value = QChar('#', 0) + QString::number(value.toInt() - qRgba(0, 0, 0, 255), 16);
				break;
		}
	} else {
		value = ParentStyle::styleHint(hint, option, widget, returnData);
		setting = 0;
	}
#if 1
	if (setting && d->settings && setting->type != StyleSetting::Parent && !d->settings->contains(QString::fromAscii(setting->label))) {
		d->settings->setValue(QString::fromAscii(setting->label), value);
	}
#endif
	if (setting) {
		if (d->settings) {
			value = d->settings->value(QString::fromAscii(setting->label), value);
		}
		switch (setting->type) {
			case StyleSetting::Color:
				value = qRgba(0, 0, 0, 255) + QLocale::c().toInt(value.toString().mid(1), 0, 16);
				break;
			case StyleSetting::Bool:
				value = value.toBool();
				break;
			case StyleSetting::Char:
				QString s = value.toString();
				if (s.size() == 1) {
					return s.at(0).unicode();
				}
				return setting->value;
		}
	}
	return value.toInt();
}


/*-----------------------------------------------------------------------*/

void paintNothing(QPainter */*painter*/, const QStyleOption */*option*/)
{
	//
}


void paintDefault(QPainter */*painter*/, const QStyleOption */*option*/)
{
	//
}


/*-----------------------------------------------------------------------*/

void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole);
void paintPushButtonBevel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole fgrole, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintTabWidgetFrame(QPainter *painter, const QStyleOptionTabWidgetFrame *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget);
void paintIndicatorCheckBox(QPainter *painter, const QStyleOptionButton *option);
void paintIndicatorRadioButton(QPainter *painter, const QStyleOptionButton *option);
void paintIndicatorSpinDown(QPainter *painter, const QStyleOption *option);
void paintIndicatorSpinUp(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowDown(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowLeft(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowRight(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowUp(QPainter *painter, const QStyleOption *option);
void paintHeaderSortIndicator(QPainter *painter, const QStyleOptionHeader *option);
void paintStyledFrame(QPainter *painter, const QStyleOptionFrame *frame, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintFrameLineEdit(QPainter *painter, const QStyleOptionFrame *frame);
void paintPanelLineEdit(QPainter *painter, const QStyleOptionFrame *frame, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget);
void paintFrameDockWidget(QPainter *painter, const QStyleOptionFrame *frame);
void paintFrameWindow(QPainter *painter, const QStyleOptionFrame *frame);
void paintToolBarSeparator(QPainter *painter, const QStyleOptionToolBar *option);
void paintToolBarHandle(QPainter *painter, const QStyleOptionToolBar *option);
void paintScrollArea(QPainter *painter, const QStyleOption *option);
void paintPanelToolBar(QPainter *painter, const QStyleOptionToolBar *option);
void paintIndicatorMenuCheckMark(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintFrameGroupBox(QPainter *painter, const QStyleOptionFrame *option);
void paintFrameFocusRect(QPainter *painter, const QStyleOptionFocusRect *option);
void paintPanelButtonTool(QPainter *painter, const QStyleOption *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintSizeGrip(QPainter *painter, const QStyleOptionSizeGrip *option);
void paintScrollAreaCorner(QPainter *painter, const QStyleOption *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void paintPanelItemViewItem(QPainter *painter, const QStyleOptionViewItemV4 *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
#endif

void paintMenuBarEmptyArea(QPainter *painter, const QStyleOption *option);
void paintPanelMenuBar(QPainter *painter, const QStyleOptionFrame *frame);
void paintMenuBarItem(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintFrameMenu(QPainter *painter, const QStyleOptionFrame *frame);
void paintMenuItem(QPainter *painter, const QStyleOptionMenuItem *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);

void paintTabBarTabShape(QPainter *painter, const QStyleOptionTab *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintTabBarTabLabel(QPainter *painter, const QStyleOptionTab *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintFrameTabBarBase(QPainter *painter, const QStyleOptionTabBarBase *option);
void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBoxV2 *option);
void paintHeaderEmptyArea(QPainter *painter, const QStyleOption *option);
void paintHeaderSection(QPainter *painter, const QStyleOptionHeader *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintHeaderLabel(QPainter *painter, const QStyleOptionHeader *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintIndicatorBranch(QPainter *painter, const QStyleOption *option);

void paintScrollBarSlider(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarAddLine(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarSubLine(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarPage(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole bgrole);
void paintProgressBarGroove(QPainter *painter, const QStyleOptionProgressBar *option);
void paintProgressBarContents(QPainter *painter, const QStyleOptionProgressBar *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintProgressBarLabel(QPainter *painter, const QStyleOptionProgressBarV2 *option);
void paintSplitter(QPainter *painter, const QStyleOption *option);
void paintDockWidgetTitle(QPainter *painter, const QStyleOptionDockWidget *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintRubberBand(QPainter *paint, const QStyleOptionRubberBand *option);

void paintSpinBox(QPainter *painter, const QStyleOptionSpinBox *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintComboBox(QPainter *painter, const QStyleOptionComboBox *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintDial(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintSlider(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintScrollBar(QPainter *painter, const QStyleOptionSlider *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
//void paintGroupBox(QPainter *painter, const QStyleOptionGroupBox *option);
void paintTitleBar(QPainter *painter, const QStyleOptionTitleBar *option, QPalette::ColorRole /*bgrole*/, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);
void paintToolButton(QPainter *painter, const QStyleOptionToolButton *option, QPalette::ColorRole bgrole, QPalette::ColorRole /*fgrole*/, void */*data*/, int /*id*/, const QWidget *widget, const QStyle *style);


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::readSettings(const QSettings &s)
{
	animateProgressBars = s.value(QString::fromAscii("ProgressBar/AnimateProgressBars"), true).toBool();
	allowScrollBarSliderToCoverArrows = s.value(QString::fromAscii("ScrollBar/AllowScrollBarSliderToCoverArrows"), true).toBool();
	hideShortcutUnderlines = s.value(QString::fromAscii("General/HideShortcutUnderlines"), true).toBool();
}


int SkulptureStyle::skulpturePrivateMethod(SkulptureStyle::SkulpturePrivateMethod id, void *data)
{
	switch (id) {
		case SPM_SupportedMethods:
			return SPM_SetSettingsFileName;
		case SPM_SetSettingsFileName: {
				SkMethodDataSetSettingsFileName *md = (SkMethodDataSetSettingsFileName *) data;
				if (md && md->version >= 1) {
					QSettings s(md->fileName, QSettings::IniFormat);
					if (s.status() == QSettings::NoError) {
						d->readSettings(s);
						return 1;
					}
				}
			}
			return 0;
		default:
			break;
	}
	return 0;
}


/*-----------------------------------------------------------------------*/

SkulptureStyle::Private::Private()
{
	init();
}


SkulptureStyle::Private::~Private()
{
	delete shortcut_handler;
	delete settings;
	QList<int> keys = draw_hash.keys();
	for (int i=0; i<keys.size(); i++)
		delete draw_hash.take(keys.value(i));
}


void SkulptureStyle::Private::register_draw(DrawElement type, int which, drawElementFunc *func, int option_type, void *data, int id)
{
	DrawElementEntry *e = new DrawElementEntry;

	if (e) {
		e->type = option_type;
		e->func = func;
		e->id = id;
		e->data = data;
		draw_hash.insert(type + which, e);
	}
}


void SkulptureStyle::Private::init()
{
	shortcut_handler = new ShortcutHandler(this);
	timer = 0;
	updatingShadows = false;
#if 0
	settings = new QSettings(QSettings::IniFormat,
		QSettings::UserScope,
		QString::fromUtf8("SkulptureStyle"),
		QString::fromUtf8(""));
#else
	settings = 0;
#endif

	QSettings s(QSettings::IniFormat, QSettings::UserScope, QString::fromUtf8("SkulptureStyle"), QString::fromUtf8(""));
	readSettings(s);

#define primitive(p, f, so) register_draw(DE_Primitive, QStyle::PE_ ## p, (drawElementFunc *) paint ## f, QStyleOption::SO_ ## so)

/* PRIMITIVE ELEMENT */
// Qt 3.x compatibility
//	primitive(Q3CheckListController, Default, Default);
//	primitive(Q3CheckListExclusiveIndicator, Default, Default);
//	primitive(Q3CheckListIndicator, Default, Default);
	primitive(Q3DockWindowSeparator, Nothing, DockWidget);
//	primitive(Q3Separator, Default, Default);
// Qt 4.0 Frames
	primitive(Frame, StyledFrame, Frame);
	primitive(FrameDefaultButton, Nothing, Button);
	primitive(FrameDockWidget, FrameDockWidget, Frame);
	primitive(FrameFocusRect, FrameFocusRect, FocusRect);
	primitive(FrameGroupBox, FrameGroupBox, Frame);
	primitive(FrameLineEdit, FrameLineEdit, Frame);
	/// Qt 4.3 calls FrameMenu with SO_ToolBar for a toolbar
	primitive(FrameMenu, FrameMenu, Default);
	primitive(FrameStatusBar, Nothing, Default);
	primitive(FrameTabWidget, TabWidgetFrame, TabWidgetFrame);
	primitive(FrameWindow, FrameWindow, Frame);
//	primitive(FrameButtonBevel, FrameButtonBevel, Frame);
//	primitive(FrameButtonTool, FrameButtonTool, Frame);
	primitive(FrameTabBarBase, FrameTabBarBase, TabBarBase);
// Qt 4.0 Panels
	register_draw(DE_Primitive, QStyle::PE_PanelButtonCommand, (drawElementFunc *) paintCommandButtonPanel, QStyleOption::SO_Button);
	register_draw(DE_Primitive, QStyle::PE_PanelButtonBevel, (drawElementFunc *) paintPanelButtonTool, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_PanelButtonTool, (drawElementFunc *) paintPanelButtonTool, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_PanelMenuBar, (drawElementFunc *) paintPanelMenuBar, QStyleOption::SO_Frame);
	register_draw(DE_Primitive, QStyle::PE_PanelToolBar, (drawElementFunc *) paintPanelToolBar, QStyleOption::SO_Frame);
	register_draw(DE_Primitive, QStyle::PE_PanelLineEdit, (drawElementFunc *) paintPanelLineEdit, QStyleOption::SO_Frame);
// Qt 4.0 Indicators
	register_draw(DE_Primitive, QStyle::PE_IndicatorArrowDown, (drawElementFunc *) paintIndicatorArrowDown, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorArrowLeft, (drawElementFunc *) paintIndicatorArrowLeft, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorArrowRight, (drawElementFunc *) paintIndicatorArrowRight, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorArrowUp, (drawElementFunc *) paintIndicatorArrowUp, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorBranch, (drawElementFunc *) paintIndicatorBranch, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorButtonDropDown, (drawElementFunc *) paintPanelButtonTool, QStyleOption::SO_Default);
//	register_draw(DE_Primitive, QStyle::PE_IndicatorViewItemCheck, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Primitive, QStyle::PE_IndicatorCheckBox, (drawElementFunc *) paintIndicatorCheckBox, QStyleOption::SO_Button);
	register_draw(DE_Primitive, QStyle::PE_IndicatorDockWidgetResizeHandle, (drawElementFunc *) paintSplitter, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorHeaderArrow, (drawElementFunc *) paintHeaderSortIndicator, QStyleOption::SO_Header);
	register_draw(DE_Primitive, QStyle::PE_IndicatorMenuCheckMark, (drawElementFunc *) paintIndicatorMenuCheckMark, QStyleOption::SO_MenuItem);
//	register_draw(DE_Primitive, QStyle::PE_IndicatorProgressChunk, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Primitive, QStyle::PE_IndicatorRadioButton, (drawElementFunc *) paintIndicatorRadioButton, QStyleOption::SO_Button);
	register_draw(DE_Primitive, QStyle::PE_IndicatorSpinDown, (drawElementFunc *) paintIndicatorSpinDown, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorSpinMinus, (drawElementFunc *) paintIndicatorSpinDown, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorSpinPlus, (drawElementFunc *) paintIndicatorSpinUp, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorSpinUp, (drawElementFunc *) paintIndicatorSpinUp, QStyleOption::SO_Default);
	register_draw(DE_Primitive, QStyle::PE_IndicatorToolBarHandle, (drawElementFunc *) paintToolBarHandle, QStyleOption::SO_ToolBar);
	register_draw(DE_Primitive, QStyle::PE_IndicatorToolBarSeparator, (drawElementFunc *) paintToolBarSeparator);
// ### which are Qt 4.1 / Qt 4.2 additions ?
//	register_draw(DE_Primitive, QStyle::PE_PanelTipLabel, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Primitive, QStyle::PE_IndicatorTabTear, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Primitive, QStyle::PE_PanelScrollAreaCorner, (drawElementFunc *) paintScrollAreaCorner);
//	register_draw(DE_Primitive, QStyle::PE_Widget, (drawElementFunc *) paint, QStyleOption::SO_);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
// Qt 4.3 additions
//	register_draw(DE_Primitive, QStyle::PE_IndicatorColumnViewArrow, (drawElementFunc *) paint, QStyleOption::SO_);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
// Qt 4.4 additions
	register_draw(DE_Primitive, QStyle::PE_PanelItemViewItem, (drawElementFunc *) paintPanelItemViewItem, QStyleOption::SO_ViewItem);
//	register_draw(DE_Primitive, QStyle::PE_PanelStatusBar, (drawElementFunc *) paint, QStyleOption::SO_);
#endif

/* CONTROL ELEMENT */
// Qt 4.0 Buttons
//	register_draw(DE_Element, QStyle::CE_PushButton, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Element, QStyle::CE_PushButtonBevel, (drawElementFunc *) paintPushButtonBevel, QStyleOption::SO_Button);
//	register_draw(DE_Element, QStyle::CE_PushButtonLabel, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_CheckBox, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_CheckBoxLabel, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_RadioButton, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_RadioButtonLabel, (drawElementFunc *) paint, QStyleOption::SO_);
// Qt 4.0 Controls
//	register_draw(DE_Element, QStyle::CE_TabBarTab, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Element, QStyle::CE_TabBarTabShape, (drawElementFunc *) paintTabBarTabShape, QStyleOption::SO_Tab);
	register_draw(DE_Element, QStyle::CE_TabBarTabLabel, (drawElementFunc *) paintTabBarTabLabel, QStyleOption::SO_Tab);
//	register_draw(DE_Element, QStyle::CE_ProgressBar, (drawElementFunc *) paint, QStyleOption::SO_ProgressBar);
	register_draw(DE_Element, QStyle::CE_ProgressBarGroove, (drawElementFunc *) paintProgressBarGroove, QStyleOption::SO_ProgressBar);
	register_draw(DE_Element, QStyle::CE_ProgressBarContents, (drawElementFunc *) paintProgressBarContents, QStyleOption::SO_ProgressBar);
	register_draw(DE_Element, QStyle::CE_ProgressBarLabel, (drawElementFunc *) paintProgressBarLabel, QStyleOption::SO_ProgressBar);
// Qt 4.0 Menus
/*/*/	register_draw(DE_Element, QStyle::CE_MenuItem, (drawElementFunc *) paintMenuItem, QStyleOption::SO_MenuItem);
//	register_draw(DE_Element, QStyle::CE_MenuScroller, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_MenuVMargin, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_MenuHMargin, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_MenuTearoff, (drawElementFunc *) paint, QStyleOption::SO_);
/*/*/	register_draw(DE_Element, QStyle::CE_MenuEmptyArea, (drawElementFunc *) paintNothing);
/*/*/	register_draw(DE_Element, QStyle::CE_MenuBarItem, (drawElementFunc *) paintMenuBarItem, QStyleOption::SO_MenuItem);
	register_draw(DE_Element, QStyle::CE_MenuBarEmptyArea, (drawElementFunc *) paintMenuBarEmptyArea);
// Qt 4.0 more Controls
//	register_draw(DE_Element, QStyle::CE_ToolButtonLabel, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_Header, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Element, QStyle::CE_HeaderSection, (drawElementFunc *) paintHeaderSection, QStyleOption::SO_Header);
	register_draw(DE_Element, QStyle::CE_HeaderLabel, (drawElementFunc *) paintHeaderLabel, QStyleOption::SO_Header);
//	register_draw(DE_Element, QStyle::CE_Q3DockWindowEmptyArea, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_ToolBoxTab, (drawElementFunc *) paint, QStyleOption::SO_ToolBox);
	register_draw(DE_Element, QStyle::CE_SizeGrip, (drawElementFunc *) paintSizeGrip, QStyleOption::SO_SizeGrip);
	register_draw(DE_Element, QStyle::CE_Splitter, (drawElementFunc *) paintSplitter);
	register_draw(DE_Element, QStyle::CE_RubberBand, (drawElementFunc *) paintRubberBand, QStyleOption::SO_RubberBand);
	register_draw(DE_Element, QStyle::CE_DockWidgetTitle, (drawElementFunc *) paintDockWidgetTitle, QStyleOption::SO_DockWidget);
// Qt 4.0 ScrollBar
	register_draw(DE_Element, QStyle::CE_ScrollBarAddLine, (drawElementFunc *) paintScrollBarAddLine, QStyleOption::SO_Slider);
	register_draw(DE_Element, QStyle::CE_ScrollBarSubLine, (drawElementFunc *) paintScrollBarSubLine, QStyleOption::SO_Slider);
	register_draw(DE_Element, QStyle::CE_ScrollBarAddPage, (drawElementFunc *) paintScrollBarPage, QStyleOption::SO_Slider);
	register_draw(DE_Element, QStyle::CE_ScrollBarSubPage, (drawElementFunc *) paintScrollBarPage, QStyleOption::SO_Slider);
	register_draw(DE_Element, QStyle::CE_ScrollBarSlider, (drawElementFunc *) paintScrollBarSlider, QStyleOption::SO_Slider);
//	register_draw(DE_Element, QStyle::CE_ScrollBarFirst, (drawElementFunc *) paint, QStyleOption::SO_);
//	register_draw(DE_Element, QStyle::CE_ScrollBarLast, (drawElementFunc *) paint, QStyleOption::SO_);
// Qt 4.0 even more Controls
//	register_draw(DE_Element, QStyle::CE_FocusFrame, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Element, QStyle::CE_ComboBoxLabel, (drawElementFunc *) paintComboBoxLabel, QStyleOption::SO_ComboBox);
	register_draw(DE_Element, QStyle::CE_ToolBar, (drawElementFunc *) paintPanelToolBar, QStyleOption::SO_ToolBar);
	register_draw(DE_Element, QStyle::CE_ToolBoxTabShape, (drawElementFunc *) paintToolBoxTabShape, QStyleOption::SO_ToolBox);
//	register_draw(DE_Element, QStyle::CE_ToolBoxTabLabel, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Element, QStyle::CE_HeaderEmptyArea, (drawElementFunc *) paintHeaderEmptyArea, QStyleOption::SO_Default);
// Qt 4.3 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
	register_draw(DE_Element, QStyle::CE_ColumnViewGrip, (drawElementFunc *) paintSplitter, QStyleOption::SO_Default);
#endif
// Qt 4.4 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//	register_draw(DE_Element, QStyle::CE_ItemViewItem, (drawElementFunc *) paint, QStyleOption::SO_);
#endif
//	register_draw(DE_Element, QStyle::CE_, (drawElementFunc *) paint, QStyleOption::SO_);

/* COMPLEX CONTROL */
	/**
	 * must override QCommonStyle, because of:
	 *
	 *	ComboBox: not handled
	 *	SpinBox: qDrawWinPanel
	 *	Dial: ugly rendering
	 *	Slider: only draws the tickmarks
	 *	Q3ListView: only draws the background
	 *
	 * must override QWindowsStyle, because of:
	 *
	 *	ComboBox: qDrawWinButton
	 *	Slider: fixed slider
	 *	SpinBox: qDrawWinPanel
	 *
	 **/
// Qt 4.0 Controls
	register_draw(DE_Complex, QStyle::CC_SpinBox, (drawElementFunc *) paintSpinBox, QStyleOption::SO_SpinBox);
	register_draw(DE_Complex, QStyle::CC_ComboBox, (drawElementFunc *) paintComboBox, QStyleOption::SO_ComboBox);
	register_draw(DE_Complex, QStyle::CC_ScrollBar, (drawElementFunc *) paintScrollBar, QStyleOption::SO_Slider);
	register_draw(DE_Complex, QStyle::CC_Slider, (drawElementFunc *) paintSlider, QStyleOption::SO_Slider);
	register_draw(DE_Complex, QStyle::CC_ToolButton, (drawElementFunc *) paintToolButton, QStyleOption::SO_ToolButton);
	register_draw(DE_Complex, QStyle::CC_TitleBar, (drawElementFunc *) paintTitleBar, QStyleOption::SO_TitleBar);
//	register_draw(DE_Complex, QStyle::CC_Q3ListView, (drawElementFunc *) paint, QStyleOption::SO_);
	register_draw(DE_Complex, QStyle::CC_Dial, (drawElementFunc *) paintDial, QStyleOption::SO_Slider);
//	register_draw(DE_Complex, QStyle::CC_GroupBox, (drawElementFunc *) paintGroupBox, QStyleOption::SO_GroupBox);
// Qt 4.3 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
//	register_draw(DE_Complex, QStyle::CC_MdiControls, (drawElementFunc *) paint, QStyleOption::SO_);
#endif
//	register_draw(DE_Complex, QStyle::CC_, (drawElementFunc *) paint, QStyleOption::SO_);
}


/*-----------------------------------------------------------------------*/

//#include "skulpture_p.moc"


