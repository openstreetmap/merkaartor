/*
 * Skulpture - Classical Three-Dimensional Artwork for Qt 4
 *
 * Copyright (c) 2007-2009 Christoph Feck <christoph@maxiom.de>
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#endif
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QWorkspace>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QTreeView>
#include <QtGui/QTableView>
#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QPainterPath>
#include <QtGui/QGroupBox>
#include <QtGui/QDockWidget>
#include <QtGui/QToolButton>
#include <QtGui/QTextEdit>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtGui/QPlainTextEdit>
#include <QtGui/QFormLayout>
#endif
#include <QtGui/QStackedLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QComboBox>
#include <QtGui/QDial>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
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
#include <QtCore/QTimer>
#include <cstdio>
#include <QtCore/QDebug>

/*-----------------------------------------------------------------------*/

#include <QtGui/QStylePlugin>

class SkulptureStylePlugin : public QStylePlugin
{
	public:
		QStringList keys() const {
			return QStringList(QLatin1String("Skulpture"));
		}

		QStyle *create(const QString &key) {
			if (key.toLower() == QLatin1String("skulpture")) {
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
    QString recursionCheck = QLatin1String("\n/* -skulpture-recursion-check- */\n");
    if (!d->styleSheetFileName.isEmpty()) {
        QString oldStyle = application->styleSheet();
        if (!oldStyle.contains(recursionCheck)) {
            QFile file(d->styleSheetFileName);
            if (file.open(QIODevice::ReadOnly)) {
                QTextStream stream(&file);
                QString newStyle = stream.readAll();
                application->setStyleSheet(newStyle + recursionCheck + oldStyle);
            }
        }
    }
#endif
	ParentStyle::polish(application);
	application->installEventFilter(d->shortcut_handler);
#if 0
	QPalette palette;
	polish(palette);
	application->setPalette(palette);
#endif
//	if (application->inherits("KApplication")) {
//		qDebug() << "KApplication is a" << application->metaObject()->className() << "(" << "object name:" << application->objectName() << ")";
//	}
//        QFontMetrics fm = QFontMetrics(QFont());
//        printf("h:%d, s:%d, xh:%d, xb:%d, Xb:%d, Xyb: %d\n", fm.height(), fm.lineSpacing(), fm.xHeight(), fm.boundingRect(QChar('x', 0)).height(), fm.boundingRect(QChar('X', 0)).height(), fm.boundingRect(QLatin1String("Xy")).height());
}


void SkulptureStyle::unpolish(QApplication *application)
{
	application->removeEventFilter(d->shortcut_handler);
	ParentStyle::unpolish(application);
}


/*-----------------------------------------------------------------------*/

enum SidebarViewMode {
    DefaultSidebar,
    TransparentSidebar
};


static void polishSidebarView(QAbstractItemView *view, SidebarViewMode viewMode)
{
    QWidget *viewport = view->viewport();
    QPalette palette = view->palette();

    if (viewMode == TransparentSidebar) {
        if (viewport->autoFillBackground()) {
            viewport->setAutoFillBackground(false);
            QPalette::ColorRole textRole = viewport->foregroundRole();
            if (textRole != QPalette::WindowText) {
                palette.setBrush(QPalette::Active, textRole, palette.brush(QPalette::Active, QPalette::WindowText));
                palette.setBrush(QPalette::Inactive, textRole, palette.brush(QPalette::Inactive, QPalette::WindowText));
                palette.setBrush(QPalette::Disabled, textRole, palette.brush(QPalette::Disabled, QPalette::WindowText));
                viewport->setPalette(palette);
            }
        }
        view->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    } else {
        if (viewport->autoFillBackground()) {
            palette.setBrush(QPalette::Active, QPalette::Window, palette.brush(QPalette::Active, QPalette::Base));
            palette.setBrush(QPalette::Inactive, QPalette::Window, palette.brush(QPalette::Inactive, QPalette::Base));
            palette.setBrush(QPalette::Disabled, QPalette::Window, palette.brush(QPalette::Disabled, QPalette::Base));
        } else {
            viewport->setAutoFillBackground(true);
        }
        view->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        viewport->setPalette(palette);
    }
}


/*-----------------------------------------------------------------------*/

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
static WidgetShadow *findShadow(QWidget *widget)
{
	QWidget *parent = widget->parentWidget();
	if (parent) {
		QList<WidgetShadow *> shadows = parent->findChildren<WidgetShadow *>();

		Q_FOREACH (WidgetShadow *shadow, shadows) {
			if (shadow->widget() == widget) {
				return shadow;
			}
		}
	}
	return 0;
}
#endif

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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
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
	//	lcd->setPalette(palette);
	//	lcd->installEventFilter(d);
	//	lcd->setContentsMargins(8, 8, 8, 8);
		lcd->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
		lcd->setSegmentStyle(QLCDNumber::Flat);
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
	if (widget->inherits("KTitleWidget")) {
//            widget->setMaximumHeight(0);
#if 0
		QPalette palette = widget->palette();
		palette.setColor(QPalette::Base, palette.color(QPalette::Window));
		palette.setColor(QPalette::Text, palette.color(QPalette::WindowText));
		widget->setPalette(palette);
#endif
        }
        if (qobject_cast<QScrollBar *>(widget)) {
                widget->installEventFilter(d);
        }
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
		if (frame->frameShadow() == QFrame::Plain && frame->backgroundRole() == QPalette::Base) {
			if (frame->parentWidget() && frame->parentWidget()->inherits("KTitleWidget")) {
				frame->setBackgroundRole(QPalette::Window);
			}
		}
#if 1
                if (!strcmp(widget->metaObject()->className(), "QListWidget")
                    && widget->parentWidget()
                    && !strcmp(widget->parentWidget()->metaObject()->className(), "Sidebar")) {
                    //(static_cast<QAbstractItemView *>(widget))->setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
                    (static_cast<QAbstractItemView *>(widget))->setFrameStyle(QFrame::NoFrame);
                }
                if (!strcmp(widget->metaObject()->className(), "Kontact::Navigator")) {
                    // (static_cast<QAbstractItemView *>(widget))->viewport()->setBackgroundRole(QPalette::Base);
                    // (static_cast<QAbstractItemView *>(widget))->viewport()->setAutoFillBackground(true);
                    (static_cast<QAbstractItemView *>(widget))->setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
                    // polishSidebarView(static_cast<QAbstractItemView *>(widget), d->transparentPlacesPanel ? TransparentSidebar : DefaultSidebar);
                }
		if (widget->inherits("SidebarTreeView")) {
                    polishSidebarView(static_cast<QAbstractItemView *>(widget), DefaultSidebar);
                }
		if (widget->inherits("KHTMLView")) {
		//	QPalette palette = widget->palette();
		//	palette.setColor(QPalette::Window, palette.color(QPalette::Base));
		//	((QAbstractScrollArea *) widget)->viewport()->setPalette(palette);
		//	printf("frame style is 0x%08x\n", ((QFrame *) widget)->frameStyle());
			((QFrame *) widget)->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		}
#endif
#if 1
		if (widget->inherits("KFilePlacesView")) {
                    polishSidebarView(static_cast<QAbstractItemView *>(widget), d->transparentPlacesPanel ? TransparentSidebar : DefaultSidebar);
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
                        edit->installEventFilter(d);
                        widget->setAttribute(Qt::WA_Hover, true);
		}
#endif
#if 1
		if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
			if (!qstrcmp(widget->metaObject()->className(), "SampleEdit")) {
				QWidget *bg = new QWidget(widget);
				bg->lower();
				bg->setObjectName(QLatin1String("sample_background"));
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
                        edit->installEventFilter(d);
                        widget->setAttribute(Qt::WA_Hover, true);
			edit->setTabChangesFocus(true);
#if 0
			if (QTextBrowser *browser = qobject_cast<QTextBrowser *>(widget)) {
				connect(browser, SIGNAL(sourceChanged()), &d->mapper, SLOT(map()));
			}
#endif
		}
#endif
	}
#if 0
	if (QComboBox *combo = qobject_cast<QComboBox *>(widget)) {
		if (!combo->isEditable()) {
			combo->setBackgroundRole(QPalette::Button);
			combo->setForegroundRole(QPalette::ButtonText);
		}
	}
	if (qobject_cast<QCheckBox *>(widget)
	 || qobject_cast<QRadioButton *>(widget)) {
		widget->setBackgroundRole(QPalette::Window);
		widget->setForegroundRole(QPalette::WindowText);
	}
#endif
	if (qobject_cast<QScrollBar *>(widget)
	 || qobject_cast<QSlider *>(widget)
	 || qobject_cast<QDial *>(widget)
	 || qobject_cast<QLineEdit *>(widget)
	 || qobject_cast<QAbstractSpinBox *>(widget)
	 || qobject_cast<QHeaderView*>(widget)
	 || qobject_cast<QTabBar *>(widget)
	 || qobject_cast<QSplitterHandle *>(widget)
	 || qobject_cast<QPushButton *>(widget)
	 || qobject_cast<QComboBox *>(widget)
	 || qobject_cast<QCheckBox *>(widget)
	 || qobject_cast<QRadioButton *>(widget)
	 || qobject_cast<QGroupBox *>(widget)
	 || qobject_cast<QToolButton *>(widget)) {
		widget->setAttribute(Qt::WA_Hover, true);
	}
#if 0
	if (d->allowScrollBarSliderToCoverArrows && qobject_cast<QScrollBar *>(widget)) {
		widget->installEventFilter(d);
	}
#endif
#if 0
        if (widget->inherits("Q3ProgressBar")) {
            widget->installEventFilter(d);
            if (widget->isVisible()) {
                d->setAnimated(widget, true);
            }
        }
#endif
	if (QProgressBar *pbar = qobject_cast<QProgressBar *>(widget)) {
		pbar->installEventFilter(d);
		if (pbar->isVisible() && !widget->inherits("StatusBarSpaceInfo")) {
			d->setAnimated(pbar, true);
		}
	}
#if 1
        if (qobject_cast<QMenu *>(widget)) {
            widget->installEventFilter(d);
        }
	if (QToolBar *toolbar = qobject_cast<QToolBar *>(widget)) {
		QFont font;
		font.setPointSizeF(font.pointSizeF() / (1.19));
		QList<QToolButton *> children = toolbar->findChildren<QToolButton *>();
		Q_FOREACH (QToolButton *child, children) {
			if (!child->icon().isNull()) {
				child->setFont(font);
			}
		}
		connect(toolbar, SIGNAL(orientationChanged(Qt::Orientation)), d, SLOT(updateToolBarOrientation(Qt::Orientation)));
                toolbar->setBackgroundRole(QPalette::Window);
        }
        if (widget->inherits("Q3ToolBar")) {
            widget->setBackgroundRole(QPalette::Window);
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
		Q_FOREACH (QAction *child, children) {
			child->setFont(oldfont);
		}*/
#else
		menu->setStyleSheet(QLatin1String("font-size: 6.5")/*.arg(menu->font().pointSizeF() / (1.19 * 1.19))*/);
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
		Q_FOREACH (QWidget *child, children) {
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
                        if (QTreeView *tree = qobject_cast<QTreeView *>(widget)) {
                            iv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                            if (tree->uniformRowHeights()) {
                                iv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
                            }
                        } else if (QListView *list = qobject_cast<QListView *>(widget)) {
                            if (list->uniformItemSizes()) {
                                iv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                                iv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
                            }
                        } else if (qobject_cast<QTableView *>(widget)) {
                            iv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
                            iv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
                        }
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
		if (area->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken)) {
			d->installFrameShadow(area);
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
#if 0
	if (widget->inherits("KTextEditor::View")) {
		QWidget *parent = widget->parentWidget();
		if (parent) {
			QFrame *frame = new QFrame(parent);
			if (frame) {
				frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
				widget->setParent(frame);
			}
		}
	}
#endif
#if 1
	if (widget->inherits("KCharSelectTable")) {
		QPalette palette;
		widget->setPalette(palette);
	}
#endif
#if 1
	if (widget->inherits("KFadeWidgetEffect")) {
		widget->installEventFilter(d);
	}
#endif
	if (widget->inherits("Q3ScrollView")) {
		QFrame *frame = qobject_cast<QFrame *>(widget);
		if (frame && frame->frameStyle() == (QFrame::StyledPanel | QFrame::Sunken)) {
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
        if (qobject_cast<QLineEdit *>(widget)) {
            widget->unsetCursor();
            widget->installEventFilter(d);
            widget->setMouseTracking(true);
        }
        if (QLayout *layout = widget->layout()) {
            // explicitely check public layout classes, QMainWindowLayout doesn't work here
            if (qobject_cast<QBoxLayout *>(layout)
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
             || qobject_cast<QFormLayout *>(layout)
#endif
             || qobject_cast<QGridLayout *>(layout)
             || qobject_cast<QStackedLayout *>(layout)) {
                d->polishLayout(layout);
             }
        }
        if (!qstrcmp(widget->metaObject()->className(), "InfoSidebarPage")) {
            widget->installEventFilter(d);
        }
#if 0//(QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
        if (widget->inherits("KTabWidget") && widget->property("closeButtonEnabled").toBool()) {
            widget->setProperty("tabsClosable", true);
            widget->setProperty("closeButtonEnabled", false);
            connect(widget, SIGNAL(tabCloseRequested(int)), widget, SIGNAL(closeRequest(int)));
        }
        if (widget->inherits("KTabBar")) {
            widget->setProperty("tabsClosable", true);
            connect(widget, SIGNAL(tabCloseRequested(int)), widget, SIGNAL(closeRequest(int)));
        }
#endif
        ParentStyle::polish(widget);
}


void SkulptureStyle::unpolish(QWidget *widget)
{
	ParentStyle::unpolish(widget);
//	return;
	if (qobject_cast<QScrollBar *>(widget)
	 || qobject_cast<QSlider *>(widget)
	 || qobject_cast<QDial *>(widget)
//	 || qobject_cast<QLineEdit *>(widget)
	 || qobject_cast<QAbstractSpinBox *>(widget)
	 || qobject_cast<QHeaderView*>(widget)
	 || qobject_cast<QTabBar *>(widget)
	 || qobject_cast<QSplitterHandle *>(widget)
	 || qobject_cast<QPushButton *>(widget)
	 || qobject_cast<QComboBox *>(widget)
	 || qobject_cast<QCheckBox *>(widget)
	 || qobject_cast<QRadioButton *>(widget)
	 || qobject_cast<QGroupBox *>(widget)
	 || qobject_cast<QToolButton *>(widget)) {
		widget->setAttribute(Qt::WA_Hover, false);
	}
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
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
#endif
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
#if 1
	if (widget->inherits("KFadeWidgetEffect")) {
		widget->removeEventFilter(d);
	}
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
	if (widget->inherits("QPlainTextEdit")) {
		QPlainTextEdit *edit = static_cast<QPlainTextEdit *>(widget);
		edit->viewport()->removeEventFilter(d);
                edit->removeEventFilter(d);
        }
#endif
        if (qobject_cast<QScrollBar *>(widget)) {
            widget->removeEventFilter(d);
        }
	if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
		if (!qstrcmp(widget->metaObject()->className(), "SampleEdit")) {
			QList<QObject *> children = widget->children();
			Q_FOREACH (QObject *child, children) {
				if (child->objectName() == QLatin1String("sample_background")) {
					child->setParent(0);
					child->deleteLater();
				}
			}
		} else {
			d->mapper.removeMappings(edit);
		}
		edit->viewport()->removeEventFilter(d);
                edit->removeEventFilter(d);
        }
	if (QToolBar *toolbar = qobject_cast<QToolBar *>(widget)) {
		QFont font;
	//	font.setPointSizeF(font.pointSizeF() / (1.19));
		QList<QToolButton *> children = toolbar->findChildren<QToolButton *>();
		Q_FOREACH (QToolButton *child, children) {
			if (!child->icon().isNull()) {
				child->setFont(font);
			}
		}
		disconnect(toolbar, SIGNAL(orientationChanged(Qt::Orientation)), d, SLOT(updateToolBarOrientation(Qt::Orientation)));
	}
	if (!qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
		widget->removeEventFilter(d);
	}
        if (qobject_cast<QLineEdit *>(widget)) {
            widget->setMouseTracking(false);
            widget->removeEventFilter(d);
            widget->setCursor(Qt::IBeamCursor);
        }
        if (!d->postEventWidgets.isEmpty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
            d->postEventWidgets.removeOne(widget);
#else
            d->postEventWidgets.removeAll(widget);
#endif
        }
        if ((QWidget *) d->oldEdit == widget) {
            d->oldEdit = 0;
        }
        if (!qstrcmp(widget->metaObject()->className(), "InfoSidebarPage")) {
            widget->removeEventFilter(d);
        }
        if (qobject_cast<QMenu *>(widget)) {
            widget->removeEventFilter(d);
        }
}


/*-----------------------------------------------------------------------*/

extern void lineEditMouseMoved(QLineEdit *lineEdit, QMouseEvent *event);

void SkulptureStyle::Private::processPostEventWidgets()
{
    QWidget *widget;

    while (!postEventWidgets.isEmpty() && (widget = postEventWidgets.takeFirst())) {
        if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget)) {
            handleCursor(edit);
        }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        else if (QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(widget)) {
            handleCursor(edit);
        }
#endif
    }
}


void SkulptureStyle::Private::addPostEventWidget(QWidget *widget)
{
    if (qobject_cast<QTextEdit *>(widget)
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        || qobject_cast<QPlainTextEdit *>(widget)
#endif
    ) {
        if (!postEventWidgets.contains(widget)) {
            bool signal = postEventWidgets.isEmpty();
            postEventWidgets.append(widget);
            if (signal) {
                QTimer::singleShot(0, this, SLOT(processPostEventWidgets()));
            }
        }
    }
}


bool SkulptureStyle::Private::eventFilter(QObject *watched, QEvent *event)
{
#if 0
	// can't happen, because widgets are the only ones to install it
	if (!watched->isWidgetType()) {
		return QObject::eventFilter(watched, event);
	}
#endif
	QWidget *widget = reinterpret_cast<QWidget *>(watched);
#if 0
        if (event->type() != QEvent::UpdateRequest && event->type() != QEvent::Paint) {
            qDebug() << "handling" << event->type() << "for object" << widget->objectName() << "which is a" << widget->metaObject()->className() << " which is a" << widget->metaObject()->superClass()->className();
        }
#endif
        if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
            return menuEventFilter(menu, event);
        }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
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
#endif
        if (event->type() == QEvent::Hide || event->type() == QEvent::Destroy) {
            if (!postEventWidgets.isEmpty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
                postEventWidgets.removeOne(widget);
#else
                postEventWidgets.removeAll(widget);
#endif
            }
            if ((QWidget *) oldEdit == widget) {
                oldEdit = 0;
            }
        } else if (event->type() != QEvent::Paint) {
            addPostEventWidget(widget);
            if (QWidget *parent = widget->parentWidget()) {
                addPostEventWidget(parent);
                if ((parent = parent->parentWidget())) {
                    addPostEventWidget(parent);
                }
            }
        }
        switch (event->type()) {
		case QEvent::Paint:
#if 1 // highlight current line in QTextEdit / QPlainTextEdit
			if (widget->objectName() == QLatin1String("qt_scrollarea_viewport")) {
				if (QTextEdit *edit = qobject_cast<QTextEdit *>(widget->parent())) {
					if (!qstrcmp(edit->metaObject()->className(), "SampleEdit")) {
						QList<QObject *> children = edit->children();
						Q_FOREACH (QObject *child, children) {
							if (child->objectName() == QLatin1String("sample_background")) {
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
					paintCursorLine(edit);
				}
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
				else if (widget->parent()->inherits("QPlainTextEdit")) {
					paintCursorLine(static_cast<QPlainTextEdit *>(widget->parent()));
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
                        if (!qstrcmp(widget->metaObject()->className(), "InfoSidebarPage")) {
                            QPainter painter(widget);
                            paintThinFrame(&painter, widget->rect().adjusted(0, 0, 0, 0), widget->palette(), 60, -20);
                            paintThinFrame(&painter, widget->rect().adjusted(1, 1, -1, -1), widget->palette(), -20, 60);
                        }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
			if (!qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
				QPainter painter(widget);
				QStyleOption option;
				option.initFrom(widget);
                                QIcon::Mode iconMode = QIcon::Normal;
				if (option.state & QStyle::State_Enabled && option.state & QStyle::State_MouseOver) {
                                    //iconMode = QIcon::Active;
				//	painter.fillRect(widget->rect(), Qt::red);
				} else {
                                    //iconMode = QIcon::Disabled;
					painter.setOpacity(0.2);
				}
				QRect r = QRect(widget->rect().center() - QPoint(6, 5), QSize(12, 12));
				painter.drawPixmap(r, q->standardIcon(QStyle::SP_TitleBarCloseButton, &option, widget).pixmap(12, 12, iconMode));
				event->accept();
				return true;
			}
			if (widget->inherits("KFadeWidgetEffect")) {
			//	widget->hide();
				event->accept();
			//	widget->removeEventFilter(this);
				return true;
			}
			break;
#endif
                case QEvent::MouseMove:
                    if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(watched)) {
                        lineEditMouseMoved(lineEdit, static_cast<QMouseEvent *>(event));
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
					Q_FOREACH (QObject *child, children) {
						if (child->objectName() == QLatin1String("sample_background")) {
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
			else if (qobject_cast<QMdiArea *>(widget)) {
				QList<WidgetShadow *> shadows = widget->findChildren<WidgetShadow *>();
				Q_FOREACH (WidgetShadow *shadow, shadows) {
					shadow->updateGeometry();
				}
			}
#endif
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

#define SkulptureDrawFunction(function, selectortype, optiontype, array) \
\
void SkulptureStyle::function(selectortype element, const optiontype *option, QPainter *painter, const QWidget *widget) const \
{ \
	if (uint(element) < array_elements(array)) { \
		const Private::DrawElementEntry *entry = &array[element]; \
		if (entry->func && option && (!entry->type || option->type == entry->type)) { \
			entry->func(painter, option, widget, this); \
			return; \
		} \
	} \
	ParentStyle::function(element, option, painter, widget); \
}

SkulptureDrawFunction(drawPrimitive, PrimitiveElement, QStyleOption, d->draw_primitive_entry)
SkulptureDrawFunction(drawControl, ControlElement, QStyleOption, d->draw_element_entry)
//SkulptureDrawFunction(drawComplexControl, ComplexControl, QStyleOptionComplex, d->draw_complex_entry)


/*-----------------------------------------------------------------------*/

//#include "skulpture.moc"


/*
 * skulpture_animations.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QProgressBar>
#include <QtCore/QTimerEvent>


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::setAnimated(QWidget *widget, bool animated)
{
	if (!widget) {
		return;
	}

	animations.removeAll(widget);
	if (animated && animateProgressBars) {
		animations.prepend(widget);
		if (!timer) {
			timer = startTimer(60);
		}
	} else {
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
		Q_FOREACH (QWidget *widget, animations) {
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
#include "sk_factory.h"
#include <QtGui/QPainter>


/*-----------------------------------------------------------------------*/

#define awf 0.8 /* width of inner arrow 0 ... 1 */
#define ahf 0.2 /* position of inner arrow, -1 ... 1 */
#define spf 0.2 /* position of spin plus/minus edges */
#define swf 0.8 /* width of spin plus/minus sign */
#define shf 1.0 /* height of spin plus/minus sign */

static const ShapeFactory::Code arrowShapeDescription[] = {
    Pmove(-1, 1), Pline(-awf, 1), Pline(0, ahf), Pline(awf, 1), Pline(1, 1), Pline(0, -1), Pend
};

static const ShapeFactory::Code spinPlusDescription[] = {
    Pmove(-swf, spf), Pline(-spf, spf), Pline(-spf, shf), Pline(spf, shf), Pline(spf, spf), Pline(swf, spf),
           Pline(swf, -spf), Pline(spf, -spf), Pline(spf, -shf), Pline(-spf, -shf), Pline(-spf, -spf), Pline(-swf, -spf), Pend
};

static const ShapeFactory::Code spinMinusDescription[] = {
    Pmove(-swf, spf), Pline(swf, spf), Pline(swf, -spf), Pline(-swf, -spf), Pend
};

static const ShapeFactory::Code sortIndicatorShapeDescription[] = {
    Pmove(-1, 1), Pline(1, 1), Pline(0, -1), Pend
};


/*-----------------------------------------------------------------------*/

static inline QPainterPath arrowPath(const QStyleOption *option, Qt::ArrowType arrow, bool spin)
{
	qreal var[ShapeFactory::MaxVar + 1];
	var[1] = 0.01 * arrow;
	var[2] = spin ? 1.0 : 0.0;
	var[3] = option->fontMetrics.height();
	var[4] = 0.0;
	QPainterPath shape = ShapeFactory::createShape(
         spin && arrow == Qt::LeftArrow ? spinMinusDescription :
         spin && arrow == Qt::RightArrow ? spinPlusDescription :
        arrowShapeDescription, var);
	if (var[4] != 0.0) {
		shape.setFillRule(Qt::WindingFill);
	}

	qreal h = 2.0 + var[3] * (spin ? 2.0 : 3.0) / 9.0;
	qreal w = 2.0 + var[3] / 3.0;
	h /= 2; w /= 2;
	if (arrow == Qt::DownArrow || arrow == Qt::RightArrow) {
		h = -h;
	}
	bool horiz = !spin && (arrow == Qt::LeftArrow || arrow == Qt::RightArrow);
	QMatrix arrowMatrix(horiz ? 0 : w, horiz ? w : 0, horiz ? h : 0 , horiz ? 0 : h, 0, 0);
	return arrowMatrix.map(shape);
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
#if 1
	switch (arrow) {
		case Qt::UpArrow:
			painter->translate(0, -0.5);
			break;
		case Qt::DownArrow:
			painter->translate(0, 0.5);
			break;
		case Qt::LeftArrow:
                    if (!spin) {
                        painter->translate(-0.5, 0);
                    }
			break;
		case Qt::RightArrow:
                    if (!spin) {
                        painter->translate(0.5, 0);
                    }
			break;
		case Qt::NoArrow:
			break;
	}
#endif
	painter->setPen(Qt::NoPen);
        QColor color = option->palette.color(spin ? (option->state & QStyle::State_Enabled ? QPalette::WindowText : QPalette::Text) : QPalette::ButtonText);
	if ((option->state & QStyle::State_MouseOver) && option->state & QStyle::State_Enabled /* && !(option->state & QStyle::State_Sunken)*/) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
		color = option->palette.color(QPalette::Highlight).darker(200);
#else
                color = option->palette.color(QPalette::Highlight).dark(200);
#endif
	//	painter->setPen(QPen(Qt::white, 1.0));
	} else {
	//	painter->setPen(QPen(Qt::white, 0.5));
	}
	color.setAlpha((179 * color.alpha()) >> 8);
	painter->setBrush(color);
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


/*
 * skulpture_buttons.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>


static QPainterPath button_path(const QRectF &rect, qreal k)
{
	k *= 0.1;
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


static QBrush button_gradient(const QRectF &rect, const QColor &color, const QStyleOptionButton *option)
{
    qreal ch = color.hueF();
    qreal cs = color.saturationF() * 1.0;
    qreal cv = color.valueF() * 1.0;
    int ca = color.alpha();
    QColor col;

    if (rect.height() > 64) {
        return QColor(color);
    }
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    col.setHsvF(ch, cs, qMax(0.0, cv - 0.02));
    col.setAlpha(ca);
    gradient.setColorAt(0.0, col);
    col.setHsvF(ch, cs, qMin(1.0, cv + 0.03));
    col.setAlpha(ca);
    gradient.setColorAt(1.0, col);
    return gradient;
}


void paintButtonPanel(QPainter *painter, const QStyleOptionButton *option, QPalette::ColorRole bgrole)
{
	const QRectF &c_rect = option->rect;
	const qreal t = 1.0;
	QRectF rect = c_rect;
	bool frame = true;
	if (option->features & QStyleOptionButton::Flat && !(option->state & QStyle::State_Sunken)) {
		frame = false;
	}
	painter->setPen(Qt::NoPen);
	if ((option->features & QStyleOptionButton::DefaultButton) && (option->state & QStyle::State_Enabled)) {
		painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.3), blend_color(QColor(0, 0, 0, 10), option->palette.color(QPalette::Highlight).lighter(110), 0.2), blend_color(QColor(0, 0, 0, 15), option->palette.color(QPalette::Highlight).lighter(110), 0.2)));
	} else {
		painter->setBrush(path_edge_gradient(rect, option, button_path(rect, 1.3), shaded_color(option->palette.color(QPalette::Window), -10), shaded_color(option->palette.color(QPalette::Window), -15)));
	}
	painter->drawPath(button_path(rect, 1.5));
	rect.adjust(t, t, -t, -t);
	QBrush bgbrush = option->palette.brush(option->state & QStyle::State_Enabled ? (bgrole == QPalette::NoRole ? QPalette::Button : bgrole) : QPalette::Button);
	if (bgbrush.style() == Qt::SolidPattern && bgbrush.color().alpha() == 0) {
		QColor color = option->palette.color(QPalette::Window);
		color.setAlpha(0);
		bgbrush = color;
	}
	if (frame) {
		if (option->state & QStyle::State_Enabled) {
                        if (option->state & QStyle::State_Sunken || option->state & QStyle::State_On) {
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
					bgcolor = bgcolor.lighter(102);
				} else if (option->state & QStyle::State_MouseOver) {
					bgcolor = bgcolor.lighter(104);
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
			bgcolor = bgcolor.lighter(104);
		}
		if (option->state & QStyle::State_On) {
			bgcolor = blend_color(bgcolor, option->palette.color(QPalette::Highlight), 0.2);
		}
		painter->setBrush(bgcolor);
	}
	rect.adjust(t, t, -t, -t);
	painter->save();
	// make transparent buttons appear transparent
	painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter->setBrush(Qt::black);
	painter->drawPath(button_path(rect, 0.9));
	painter->restore();
	painter->drawPath(button_path(rect, 0.9));
}


void paintPushButtonBevel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget, const QStyle *style)
{
	QStyleOptionButton opt = *option;

	opt.features &= ~(QStyleOptionButton::HasMenu);
	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_PushButtonBevel, &opt, painter, widget);
	if (option->features & QStyleOptionButton::Flat) {
		if (!(option->state & (QStyle::State_Sunken | QStyle::State_On))) {
			if (option->state & QStyle::State_MouseOver) {
				painter->fillRect(option->rect.adjusted(2, 2, -2, -2), QColor(255, 255, 255, 60));
			}
		}
	}
	if (option->features & QStyleOptionButton::HasMenu) {
		int size = style->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, widget);
		opt.palette.setColor(QPalette::WindowText, opt.palette.color(widget ? widget->foregroundRole() : QPalette::ButtonText));
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
#include <QtGui/QGradient>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include "sk_factory.h"
#include <cmath>

// FIXME
#if (QT_VERSION < QT_VERSION_CHECK(4, 3, 0))
#define cacheKey serialNumber
#endif


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

void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget)
{
	Q_UNUSED(widget);
	QPalette::ColorRole bgrole = /*widget ? widget->backgroundRole() : */QPalette::Button;

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

void paintPanelButtonTool(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);
	QStyleOptionButton button;

	if (widget && !qstrcmp(widget->metaObject()->className(), "QDockWidgetTitleButton")) {
		if (!(option->state & QStyle::State_MouseOver) && !(option->state & QStyle::State_On)) return;
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
	// FIXME bgrole?
	paintCommandButtonPanel(painter, &button, 0);
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


static void paintIndicatorShape(QPainter *painter, const QStyleOption *option, qreal scale, const QPainterPath &shapePath)
{
    // configuration
    const QPalette::ColorRole indicatorRole = QPalette::Text;

    if (option->state & QStyle::State_Sunken || option->state & QStyle::State_On || option->state & QStyle::State_MouseOver) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->translate(QRectF(option->rect).center());
        painter->setRenderHint(QPainter::Antialiasing, true);
        QColor color;
        if ((option->state & QStyle::State_MouseOver || option->state & QStyle::State_Sunken) && option->state & QStyle::State_Enabled) {
            color = option->palette.color(QPalette::Highlight);
            if (!(option->state & QStyle::State_Sunken) && !(option->state & QStyle::State_On)) {
                color.setAlpha(80);
            }
        } else if (!(option->state & QStyle::State_Sunken) && option->state & QStyle::State_On) {
            color = option->palette.color(indicatorRole);
            color.setAlpha(80);
        }
        if (color.isValid()) {
            painter->setBrush(color);
            QMatrix matrix(scale, 0, 0, scale, 0, 0);
            painter->drawPath(matrix.map(shapePath));
        }
        if (!(option->state & QStyle::State_Sunken) && option->state & QStyle::State_On) {
            painter->setBrush(option->palette.brush(indicatorRole));
            QMatrix matrix(scale - 1, 0, 0, scale - 1, 0, 0);
            painter->drawPath(matrix.map(shapePath));
        }
        painter->restore();
    }
}


/*-----------------------------------------------------------------------*/

#define csx 0.35
#define csc 0.2

// cross
static const ShapeFactory::Code checkShapeDescription1[] = {
    Pmove(-1 + csc, -1), Pline(0, -csx), Pline(1 - csc, -1), Pline(1, -1 + csc),
    Pline(csx, 0), Pline(1, 1 - csc), Pline(1 - csc, 1), Pline(0, csx),
    Pline(-1 + csc, 1), Pline(-1, 1 - csc), Pline(-csx, 0), Pline(-1, -1 + csc), Pend
};

// checkmark
static const ShapeFactory::Code checkShapeDescription2[] = {
    Pmove(1 - csc, -1), Pline(1, -1 + csc), Pline(csx, 1), Pline(-csx, 1), Pline(-1, csc),
    Pline(-1 + csc, 0), Pline(0, 1 - 2 * csx), Pend
};


static void paintCheckBox(QPainter *painter, const QStyleOption *option)
{
    if (option->state & QStyle::State_NoChange) {
        paintThinFrame(painter, option->rect, option->palette, 30, -10);
        paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -50, -60);
        paintThinFrame(painter, option->rect.adjusted(2, 2, -2, -2), option->palette, 0, 60);
        QColor color = option->palette.color(QPalette::Window);
        if (option->state & QStyle::State_Enabled) {
            if (option->state & QStyle::State_Sunken) {
                color = color.darker(110);
            } else if (option->state & QStyle::State_MouseOver) {
                color = color.lighter(106);
            }
        } else {
            color = color.darker(106);
        }
        painter->fillRect(option->rect.adjusted(3, 3, -3, -3), color);
    } else {
        QColor color = option->palette.color(QPalette::Base);
        if (!(option->state & QStyle::State_On) && !(option->state & QStyle::State_Enabled)) {
            color = option->palette.color(QPalette::Window);
        } else if (option->state & QStyle::State_MouseOver) {
            color = color.lighter(105);
        }
        painter->fillRect(option->rect.adjusted(2, 2, -2, -2), color);
        paintRecessedFrame(painter, option->rect, option->palette, RF_Small);
        if (!(option->state & QStyle::State_Sunken)) {
            if (option->state & QStyle::State_Enabled) {
                paintThinFrame(painter, option->rect.adjusted(2, 2, -2, -2), option->palette, 140, 200);
            } else {
                paintThinFrame(painter, option->rect.adjusted(2, 2, -2, -2), option->palette, 180, 180);
            }
        }
        const ShapeFactory::Description description = checkShapeDescription1;
        const qreal scale = (option->rect.width() - 4) * 0.35;
        paintIndicatorShape(painter, option, scale, ShapeFactory::createShape(description));
    }
}


void paintIndicatorCheckBox(QPainter *painter, const QStyleOptionButton *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ option->rect.width() * option->rect.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_NoChange | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-icb-%x-%x-%llx-%x-%x", state, option->direction, option->palette.cacheKey(), option->rect.width(), option->rect.height());
	}
	paintIndicatorCached(painter, option, paintCheckBox, useCache, pixmapName);
}


/*-----------------------------------------------------------------------*/

static void paintThinBevel(QPainter *painter, const QPainterPath &path, const QColor &dark, const QColor &light, qreal lightAngle = M_PI / 4)
{
    QMatrix scaleUp;
    scaleUp.scale(10, 10);
    QList<QPolygonF> bevel = path.toSubpathPolygons(scaleUp);
    Q_FOREACH (QPolygonF polygon, bevel) {
        for (int i = 0; i < polygon.size() - 1; ++i) {
            QLineF line(polygon.at(i) / 10, polygon.at(i + 1) / 10);
            line.setLength(line.length() + 0.20);
            painter->setPen(QPen(blend_color(light, dark, sin(atan2(polygon.at(i + 1).y() - polygon.at(i).y(), polygon.at(i + 1).x() - polygon.at(i).x()) - lightAngle) / 2 + 0.5), 1.0, Qt::SolidLine, Qt::FlatCap));
            painter->drawLine(line);
        }
    }
}


static void paintThinBevel(QPainter *painter, const QPainterPath &path, const QPalette &palette, int dark, int light, qreal lightAngle = M_PI / 4)
{
    paintThinBevel(painter, path, shaded_color(palette.color(QPalette::Window), dark), shaded_color(palette.color(QPalette::Window), light), lightAngle);
}


static inline QPainterPath radioShape(const QRectF rect)
{
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}


static void paintRadioButton(QPainter *painter, const QStyleOption *option)
{
    const qreal lightAngle = option->direction == Qt::LeftToRight ? M_PI / 4 : 3 * M_PI / 4;
    QColor color = option->palette.color(QPalette::Base);
    if (!(option->state & QStyle::State_On) && !(option->state & QStyle::State_Enabled)) {
        color = option->palette.color(QPalette::Window);
    } else if (option->state & QStyle::State_MouseOver) {
        color = color.lighter(105);
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPath(radioShape(QRectF(option->rect).adjusted(2, 2, -2, -2)));
    paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5)), option->palette, 39, -26, lightAngle);
    paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(1.5, 1.5, -1.5, -1.5)), option->palette, -26, -91, lightAngle);
    paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(2.5, 2.5, -2.5, -2.5)), QColor(0, 0, 0, 15), QColor(0, 0, 0, 30), lightAngle);
    paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(3.5, 3.5, -3.5, -3.5)), QColor(0, 0, 0, 8), QColor(0, 0, 0, 15), lightAngle);
    paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(4.5, 4.5, -4.5, -4.5)), QColor(0, 0, 0, 4), QColor(0, 0, 0, 8), lightAngle);
    if (!(option->state & QStyle::State_Sunken)) {
        if (option->state & QStyle::State_Enabled) {
            paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(2.5, 2.5, -2.5, -2.5)), option->palette, 140, 300, lightAngle);
        } else {
            paintThinBevel(painter, radioShape(QRectF(option->rect).adjusted(2.5, 2.5, -2.5, -2.5)), option->palette, 180, 180, lightAngle);
        }
    }
    painter->restore();
    const qreal scale = (option->rect.width() - 4) * 0.35;
    QPainterPath circlePath;
    const qreal radius = 0.7;
    circlePath.addEllipse(QRectF(-radius, -radius, 2 * radius, 2 * radius));
    paintIndicatorShape(painter, option, scale, circlePath);
}


void paintIndicatorRadioButton(QPainter *painter, const QStyleOptionButton *option)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ option->rect.width() * option->rect.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
		pixmapName.sprintf("scp-irb-%x-%x-%llx-%x-%x", state, option->direction, option->palette.cacheKey(), option->rect.width(), option->rect.height());
	}
	paintIndicatorCached(painter, option, paintRadioButton, useCache, pixmapName);
}


/*-----------------------------------------------------------------------*/

void paintIndicatorMenuCheckMark(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style)
{
	QStyleOptionButton buttonOption;

	buttonOption.QStyleOption::operator=(*option);
//	buttonOption.rect.adjust(-2, -2, 2, 2);
//	qDebug("here!");
//	printf("state 0x%08x\n", uint(buttonOption.state));
        if (option->state & QStyle::State_Enabled) {
		if (buttonOption.state & QStyle::State_On) {
			buttonOption.state |= QStyle::State_Sunken;
		}
	} else {
            buttonOption.state &= ~QStyle::State_Sunken;
        }
        if (option->state & QStyle::State_Selected) {
            buttonOption.state |= QStyle::State_MouseOver;
        } else {
            buttonOption.state &= ~QStyle::State_MouseOver;
        }
        if (option->checked) {
            buttonOption.state |= QStyle::State_On;
        } else {
            buttonOption.state &= ~QStyle::State_On;
        }
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
                buttonOption.rect = QRect(option->rect.x() + ((option->rect.width() - size.width()) >> 1), option->rect.y() + ((option->rect.height() - size.height()) >> 1), size.width(), size.height());
		paintIndicatorRadioButton(painter, &buttonOption);
	} else {
		QSize size(style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_IndicatorHeight, option, widget));
                buttonOption.rect = QRect(option->rect.x() + ((option->rect.width() - size.width()) >> 1), option->rect.y() + ((option->rect.height() - size.height()) >> 1), size.width(), size.height());
                paintIndicatorCheckBox(painter, &buttonOption);
	}
}


void paintQ3CheckListIndicator(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget, const QStyle *style)
{
	if (!option->items.isEmpty()) {
		QStyleOptionButton buttonOption;

		buttonOption.QStyleOption::operator=(*option);
		QSize size(style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_IndicatorHeight, option, widget));
		buttonOption.rect = QRect(option->rect.center() - QPoint(size.width() / 2, size.height() / 2), size);
//		buttonOption.rect.adjust(0, -1, 0, -1);
		paintIndicatorCheckBox(painter, &buttonOption);
	}
}


void paintQ3CheckListExclusiveIndicator(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget, const QStyle *style)
{
	if (!option->items.isEmpty()) {
		QStyleOptionButton buttonOption;

		buttonOption.QStyleOption::operator=(*option);
		QSize size(style->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight, option, widget));
		buttonOption.rect = QRect(option->rect.center() - QPoint(size.width() / 2, size.height() / 2), size);
//		buttonOption.rect.adjust(0, -1, 0, -1);
		paintIndicatorRadioButton(painter, &buttonOption);
	}
}


void paintIndicatorItemViewItemCheck(QPainter *painter, const QStyleOption *option)
{
	QStyleOptionButton buttonOption;

	buttonOption.QStyleOption::operator=(*option);
	buttonOption.state &= ~QStyle::State_MouseOver;
	paintIndicatorCheckBox(painter, &buttonOption);
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
			color = option->palette.color(QPalette::Highlight).darker(110);
		} else {
			color = option->palette.color(QPalette::Button);
		}
	} else {
		color = option->palette.color(QPalette::Button);
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
        // ### merge opacity into color
        painter->setOpacity(opacity);
#endif
	painter->drawEllipse(rect);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
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
#endif
	painter->restore();
}


void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole /*bgrole*/)
{
	bool useCache = UsePixmapCache;
	QString pixmapName;

	if (/* option->state & (QStyle::State_HasFocus | QStyle::State_MouseOver) ||*/ option->rect.width() * option->rect.height() > 4096) {
		useCache = false;
	}
	if (useCache) {
		uint state = uint(option->state) & (QStyle::State_Enabled | QStyle::State_On | QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_HasFocus);
		if (!(state & QStyle::State_Enabled)) {
			state &= ~(QStyle::State_MouseOver | QStyle::State_HasFocus);
		}
		state &= ~(QStyle::State_HasFocus);
                QByteArray colorName = option->palette.color(QPalette::Button).name().toAscii();
                pixmapName.sprintf("scp-isg-%x-%x-%s-%x-%x", state, option->direction, colorName.constData(), option->rect.width(), option->rect.height());
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
		painter->setBrush(option->palette.color(QPalette::Highlight).darker(180));
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
              QLinearGradient dial_gradient(option->direction == Qt::LeftToRight ? r.topLeft() : r.topRight(), option->direction == Qt::LeftToRight ? r.bottomRight() : r.bottomLeft());
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
        const qreal r = qMin(option->rect.width() * 0.5, option->fontMetrics.height() * 0.15);
        const QPointF center = QRectF(option->rect).center();
	painter->drawEllipse(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r));
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
#include <QtGui/QApplication>


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

enum ColorScheme {
    NormalColorScheme, // allow 3D effects
    DarkColorScheme, // too dark, no 3D effects
    BrightColorScheme // too bright, no 3D effects
};


ColorScheme guessColorScheme(const QPalette &palette, QPalette::ColorGroup colorGroup = QPalette::Active, QPalette::ColorRole colorRole = QPalette::Window)
{
    const QColor windowColor = palette.color(colorGroup, colorRole);
    int r, g, b;
    windowColor.getRgb(&r, &g, &b);
    int brightness = qGray(r, g, b);

    if (brightness > 230) {
        return BrightColorScheme;
    } else if (brightness < 40) {
        return DarkColorScheme;
    }
    return NormalColorScheme;
}


static void computeAlternateBase(QPalette &palette, QPalette::ColorGroup colorGroup)
{
    switch (guessColorScheme(palette, colorGroup, QPalette::Base)) {
        case DarkColorScheme:
            palette.setColor(colorGroup, QPalette::AlternateBase, palette.color(colorGroup, QPalette::Base).lighter(103));
            break;
        case BrightColorScheme:
        case NormalColorScheme:
            palette.setColor(colorGroup, QPalette::AlternateBase, palette.color(colorGroup, QPalette::Base).darker(103));
            break;
    }
}


static void copyColorGroup(QPalette &palette, QPalette::ColorGroup fromColorGroup, QPalette::ColorGroup toColorGroup)
{
    for (int role = int(QPalette::WindowText); role <= int(QPalette::LinkVisited); ++role) {
        QPalette::ColorRole colorRole = QPalette::ColorRole(role);
        palette.setColor(toColorGroup, colorRole, palette.color(fromColorGroup, colorRole));
    }
}


static void computeColorGroups(QPalette &palette, bool kdeMode = false, bool makeDisabledWidgetsTransparent = true)
{
    // ### Is this used by pre-Qt 4.5 for HLine / VLine ?
    palette.setColor(QPalette::Disabled, QPalette::Dark, shaded_color(palette.color(QPalette::Active, QPalette::Window), -20));
    palette.setColor(QPalette::Disabled, QPalette::Light, shaded_color(palette.color(QPalette::Active, QPalette::Window), 60));

    if (!kdeMode) {
        // compute remaining colors in Active group
        computeAlternateBase(palette, QPalette::Active);

        // copy Active group to Inactive group
        copyColorGroup(palette, QPalette::Active, QPalette::Inactive);

        // compute remaining colors in Inactive group
        computeAlternateBase(palette, QPalette::Inactive);
    }

    if (!kdeMode || makeDisabledWidgetsTransparent) {
        // create Disabled group
        QColor disabledBackgroundColor = palette.color(QPalette::Active, QPalette::Window);
        QColor disabledForegroundColor;
        switch (guessColorScheme(palette, QPalette::Active, QPalette::Window)) {
            case DarkColorScheme:
                disabledForegroundColor = palette.color(QPalette::Active, QPalette::Window).lighter(125);
                break;
            case BrightColorScheme:
            case NormalColorScheme:
                disabledForegroundColor = palette.color(QPalette::Active, QPalette::Window).darker(125);
                break;
        }

        palette.setColor(QPalette::Disabled, QPalette::Window, disabledBackgroundColor);
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::Base, disabledBackgroundColor);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::Link, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::LinkVisited, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::Button, disabledBackgroundColor);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::Highlight, disabledForegroundColor);
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledBackgroundColor);

        computeAlternateBase(palette, QPalette::Disabled);
    }
}


QPalette SkulptureStyle::standardPalette() const
{
    QPalette palette(QColor(205, 205, 205));

    palette.setColor(QPalette::Active, QPalette::Base, QColor(229, 229, 229));
    palette.setColor(QPalette::Active, QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Active, QPalette::Link, QColor(80, 40, 120));
    palette.setColor(QPalette::Active, QPalette::LinkVisited, QColor(80, 50, 80));
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor(114, 174, 211));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(0, 0, 0));
    palette.setColor(QPalette::Active, QPalette::Window, QColor(200, 200, 200));
    palette.setColor(QPalette::Active, QPalette::WindowText, QColor(0, 0, 0));
//    palette.setColor(QPalette::Active, QPalette::Button, QColor(205, 205, 205));
    palette.setColor(QPalette::Active, QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::Active, QPalette::Shadow, QColor(0, 0, 0));
    palette.setColor(QPalette::Active, QPalette::BrightText, QColor(240, 240, 240));
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, QColor(240, 230, 190));
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, QColor(0, 0, 0));
#endif

    computeColorGroups(palette);
    return palette;
}


void SkulptureStyle::polish(QPalette &palette)
{
    ParentStyle::polish(palette);
    computeColorGroups(palette, qApp->inherits("KApplication"), d->makeDisabledWidgetsTransparent);
}


/*
 * skulpture_combobox.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QApplication>


/*-----------------------------------------------------------------------*/

extern void paintComplexControlArea(QPainter *painter, const QStyleOption *option);

void paintComboBox(QPainter *painter, const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionComboBox opt = *option;
    const bool buttonMode = false; //!option->editable;
    QRect rect = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxArrow, widget);
    if (option->subControls & (QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxEditField)) {
        if (buttonMode) {
            QStyleOptionButton buttonOption;
            buttonOption.QStyleOption::operator=(opt);
            if (buttonOption.state & QStyle::State_On) {
                buttonOption.state |= QStyle::State_Sunken;
            } else {
                buttonOption.state &= ~QStyle::State_Sunken;
            }
            buttonOption.state &= ~QStyle::State_On;

            // separator position
            opt.rect = rect;
            if (option->direction == Qt::LeftToRight) {
                opt.rect.setWidth(1);
            } else {
                opt.rect.setLeft(rect.left() + rect.width() - 1);
            }
            if (option->frame) {
                style->drawPrimitive(QStyle::PE_PanelButtonCommand, &buttonOption, painter, widget);
                QColor color = option->palette.color(QPalette::Button);
                if (!(opt.state & QStyle::State_On)) {
                    opt.rect.translate(option->direction == Qt::LeftToRight ? -1 : 1, 0);
                }
                painter->fillRect(opt.rect, shaded_color(color, option->state & QStyle::State_Enabled ? -30 : -15));
                if (option->state & QStyle::State_Enabled) {
                    opt.rect.translate(option->direction == Qt::LeftToRight ? 1 : -1, 0);
                    painter->fillRect(opt.rect, shaded_color(color, 80));
                }
            } else {
                QColor bg = option->palette.color(QPalette::Button);
                painter->fillRect(option->rect, bg);
                painter->fillRect(opt.rect, shaded_color(bg, -15));
            }
        } else {
            int fw = option->frame ? style->pixelMetric(QStyle::PM_ComboBoxFrameWidth, option, widget) : 0;
            QColor color = option->palette.color(QPalette::Base);
            if (option->state & QStyle::State_Enabled) {
                if (option->state & QStyle::State_HasFocus && option->editable) {
                    color = blend_color(color, option->palette.color(QPalette::Highlight), 0.15);
                } else if (option->state & QStyle::State_MouseOver /*&& !option->editable*/) {
                    color = color.lighter(103);
                }
            }
            QRect edit = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxFrame, widget).adjusted(fw, fw, -fw, -fw);
            painter->fillRect(edit, color);
            if (false && option->state & QStyle::State_Enabled && option->rect.height() <= 64) {
                QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
                if (color.valueF() > 0.9) {
                    panelGradient.setColorAt(0.0, shaded_color(color, -20));
                }
                panelGradient.setColorAt(0.6, shaded_color(color, 0));
                panelGradient.setColorAt(1.0, shaded_color(color, 10));
                painter->fillRect(edit, panelGradient);
            }
            opt.rect = rect;
            if (!(option->activeSubControls & QStyle::SC_ComboBoxArrow)) {
                opt.state &= ~QStyle::State_MouseOver;
            }
            paintComplexControlArea(painter, &opt);
            if (option->subControls & QStyle::SC_ComboBoxFrame && option->frame) {
                QStyleOptionFrame frameOpt;
                frameOpt.QStyleOption::operator=(*option);
                frameOpt.rect = style->subControlRect(QStyle::CC_ComboBox, option, QStyle::SC_ComboBoxFrame, widget);
                frameOpt.state |= QStyle::State_Sunken;
                frameOpt.lineWidth = fw;
                frameOpt.midLineWidth = 0;
                style->drawPrimitive(QStyle::PE_FrameLineEdit, &frameOpt, painter, widget);
            }
        }
    }

    // arrow
    if (option->subControls & (QStyle::SC_ComboBoxArrow)) {
        opt.rect = rect;
        opt.state &= /*QStyle::State_MouseOver |*/ QStyle::State_Enabled;
        if (buttonMode) {
            opt.state &= ~QStyle::State_MouseOver;
            if (option->state & QStyle::State_On) {
                int sx = style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, option, widget);
                int sy = style->pixelMetric(QStyle::PM_ButtonShiftVertical, option, widget);
                opt.rect.adjust(sx, sy, sx, sy);
            }
        } else {
            opt.palette.setColor(QPalette::ButtonText, opt.palette.color(option->state & QStyle::State_Enabled ? QPalette::WindowText : QPalette::Text));
        }
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, widget);
    }

    // focus frame
    if ((option->state & QStyle::State_HasFocus) && !option->editable) {
        QStyleOptionFocusRect focus;
        focus.QStyleOption::operator=(*option);
        focus.rect = style->subElementRect(QStyle::SE_ComboBoxFocusRect, option, widget);
        focus.state |= QStyle::State_FocusAtBorder;
        focus.backgroundColor = option->palette.color(buttonMode ? QPalette::Button : QPalette::Base);
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, painter, widget);
    }
}


void paintComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionComboBox opt = *option;
    const bool buttonMode = false; //!option->editable;
    if (!buttonMode) {
        opt.palette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    } else {
        painter->save();
        painter->setPen(opt.palette.color(QPalette::ButtonText));
        if (opt.state & QStyle::State_On) {
            int sx = style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, option, widget);
            int sy = style->pixelMetric(QStyle::PM_ButtonShiftVertical, option, widget);
            opt.rect.adjust(sx, sy, sx, sy);
        }
    }
    ((const QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_ComboBoxLabel, &opt, painter, widget);
    if (buttonMode) {
        painter->restore();
    }
}


/*-----------------------------------------------------------------------*/

QRect subControlRectComboBox(const QStyleOptionComboBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
{
    int fw = option->frame ? style->pixelMetric(QStyle::PM_ComboBoxFrameWidth, option, widget) : 0;
    int bw = style->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget);
    if (option->editable) bw = qMax(bw, qApp->globalStrut().width());
    QRect rect;

    switch (subControl) {
        case QStyle::SC_ComboBoxArrow:
            rect = QRect(option->rect.right() - bw - fw + 1, option->rect.top() + fw, bw, option->rect.height() - 2 * fw);
            break;
        case QStyle::SC_ComboBoxEditField: {
            if (option->editable) {
                rect = option->rect.adjusted(fw, fw, -fw - bw, -fw);
            } else {
                rect = option->rect.adjusted(fw + 4, fw, -fw - bw - 4, -fw);
            }
            break;
        }
        case QStyle::SC_ComboBoxFrame:
        default: // avoid warning
            rect = option->rect;
            break;
    }
    return style->visualRect(option->direction, option->rect, rect);
}


QRect subElementRectComboBoxFocusRect(const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style)
{
    int fw = option->frame ? (option->editable ? style->pixelMetric(QStyle::PM_ComboBoxFrameWidth, option, widget) : 4) : 2;
    int bw = true || option->editable ? qMax(style->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget), qApp->globalStrut().width()) : 0;

    return style->visualRect(option->direction, option->rect, option->rect.adjusted(fw, fw, -fw - bw, -fw));
}


/*
 * skulpture_complex.cpp
 *
 */

#include "skulpture_p.h"


/*-----------------------------------------------------------------------*/

extern QRect subControlRectSpinBox(const QStyleOptionSpinBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);
extern QRect subControlRectScrollBar(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode);
extern QRect subControlRectSlider(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);
extern QRect subControlRectToolButton(const QStyleOptionToolButton *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);
extern QRect subControlRectComboBox(const QStyleOptionComboBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);
extern QRect subControlRectGroupBox(const QStyleOptionGroupBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);
extern QRect subControlRectTitleBar(const QStyleOptionTitleBar *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style);


/*-----------------------------------------------------------------------*/

#define SC_CASE(cc, so) \
    case CC_## cc: \
        if (option->type == QStyleOption::SO_## so) { \
            return subControlRect ## cc((const QStyleOption ## so *) option, subControl, widget, this); \
        } \
        break


QRect SkulptureStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    switch (control) {
        SC_CASE(SpinBox, SpinBox);
        SC_CASE(ComboBox, ComboBox);
        case CC_ScrollBar:
            if (option && option && option->type == QStyleOption::SO_Slider) {
                return subControlRectScrollBar((const QStyleOptionSlider *) option, subControl, widget, this, d->horizontalArrowMode, d->verticalArrowMode);
            }
            break;
        SC_CASE(Slider, Slider);
        SC_CASE(TitleBar, TitleBar);
        case CC_Q3ListView: break;
        SC_CASE(ToolButton, ToolButton);
        case CC_Dial: break;
//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
        SC_CASE(GroupBox, GroupBox);
//#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case CC_MdiControls:
            break;
#endif
        case CC_CustomBase: // avoid warning
            break;
    }
    return ParentStyle::subControlRect(control, option, subControl, widget);
}


/*-----------------------------------------------------------------------*/

extern QStyle::SubControl hitTestComplexControlScrollBar(const QStyleOptionSlider *option, const QPoint &position, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode);


/*-----------------------------------------------------------------------*/

#define HIT_CASE(cc, so) \
    case CC_## cc: \
        if (option->type == QStyleOption::SO_## so) { \
            return hitTestComplexControl ## cc((const QStyleOption ## so *) option, position, widget, this); \
        } \
        break


QStyle::SubControl SkulptureStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const
{
    switch (control) {
        case CC_ScrollBar:
            if (option->type == QStyleOption::SO_Slider) {
                return hitTestComplexControlScrollBar((const QStyleOptionSlider *) option, position, widget, this, d->horizontalArrowMode, d->verticalArrowMode);
            }
            break;
        default:
            break;
    }
    return ParentStyle::hitTestComplexControl(control, option, position, widget);
}


/*-----------------------------------------------------------------------*/

extern void paintSpinBox(QPainter *painter, const QStyleOptionSpinBox *option, const QWidget *widget, const QStyle *style);
extern void paintComboBox(QPainter *painter, const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style);
extern void paintScrollBar(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode);
extern void paintSlider(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style);
extern void paintToolButton(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style);
extern void paintTitleBar(QPainter *painter, const QStyleOptionTitleBar *option, const QWidget *widget, const QStyle *style);
extern void paintQ3ListView(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget, const QStyle *style);
extern void paintDial(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style);
//extern void paintGroupBox(QPainter *painter, const QStyleOptionGroupBox *option, const QWidget *widget, const QStyle *style);
//extern void paintMdiControls(QPainter *painter, const QStyleOptionComplex *option, const QWidget *widget, const QStyle *style);


/*-----------------------------------------------------------------------*/

#define CC_CASE(cc, so) \
    case CC_## cc: \
        if (option->type == QStyleOption::SO_## so) { \
            paint ## cc(painter, (const QStyleOption ## so *) option, widget, this); \
            return; \
        } \
        break


void SkulptureStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                        QPainter *painter, const QWidget *widget) const
{
    switch (control) {
        CC_CASE(SpinBox, SpinBox);
        CC_CASE(ComboBox, ComboBox);
        case CC_ScrollBar:
            if (option->type == QStyleOption::SO_Slider) {
                paintScrollBar(painter, (const QStyleOptionSlider *) option, widget, this, d->horizontalArrowMode, d->verticalArrowMode);
                return;
            }
            break;
        CC_CASE(Slider, Slider);
        CC_CASE(ToolButton, ToolButton);
        CC_CASE(TitleBar, TitleBar);
        CC_CASE(Q3ListView, Q3ListView);
        CC_CASE(Dial, Slider);
//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
        case CC_GroupBox:
            break;
//#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case CC_MdiControls:
            break;
#endif
        case CC_CustomBase: // avoid warning
            break;
    }
    ParentStyle::drawComplexControl(control, option, painter, widget);
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

void paintDial(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style)
{
	int d = qMin(option->rect.width() & ~1, option->rect.height() & ~1);
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
        opt.rect.setWidth(opt.rect.width() & ~1);
        opt.rect.setHeight(opt.rect.height() & ~1);
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


void paintDockWidgetTitle(QPainter *painter, const QStyleOptionDockWidget *option, const QWidget *widget, const QStyle *style)
{
	const QDockWidget *dock = qobject_cast<const QDockWidget *>(widget);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
    const bool vertical = dock && (dock->features() & QDockWidget::DockWidgetVerticalTitleBar);
#else
    const bool vertical = false;
#endif
    const bool floating = dock && dock->isFloating();
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
	r = style->subElementRect(QStyle::SE_DockWidgetTitleBarText, option, widget);
#else
        // FIXME
        r = option->rect;
#endif
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
	style->drawItemText(painter, r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextHideMnemonic, option->palette, true, option->title, QPalette::WindowText);
	painter->restore();
}

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
QRect subElementRectDockWidget(QStyle::SubElement element, const QStyleOptionDockWidget *option, const QWidget *widget, const QStyle *style)
{
    switch (element) {
        case QStyle::SE_DockWidgetCloseButton:
        case QStyle::SE_DockWidgetFloatButton: {
            const QDockWidget *dock = qobject_cast<const QDockWidget *>(widget);
            bool floating = option->floatable && dock && dock->isFloating();
            bool vertical = dock && (dock->features() & QDockWidget::DockWidgetVerticalTitleBar);
            QRect r = ((QCommonStyle *) style)->QCommonStyle::subElementRect(element, option, widget);
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
    case QStyle::SE_DockWidgetTitleBarText:
        return ((QCommonStyle *) style)->QCommonStyle::subElementRect(element, option, widget).adjusted(4, -3, -4, 5);
    case QStyle::SE_DockWidgetIcon:
        return ((QCommonStyle *) style)->QCommonStyle::subElementRect(element, option, widget).adjusted(4, -3, 4, 5);
    default: // avoid warning
        return option->rect;
    }
}
#endif

/*
 * skulpture_effects.cpp
 *
 */

#include "sk_effects.h"


/*-----------------------------------------------------------------------*/
/**
 * Image blurring and sharpening functions
 *
 * Currently, these are only used to blur disabled text, but may be
 * used for other things in the future.
 *
 */


/*-----------------------------------------------------------------------*/

#define V_SHIFT 10
#define V_PREC 8

#define V_MIN	0
#define V_MAX ((1 << V_PREC) - 1)
#define V_ROUND (1 << (V_SHIFT - V_PREC - 1))


/*-----------------------------------------------------------------------*/

static inline int clampValue(int v)
{
	v = (v + V_ROUND) >> V_SHIFT;
	return (v < V_MIN ? V_MIN : (v > V_MAX ? V_MAX : v));
}


/*-----------------------------------------------------------------------*/

static void sharpenRgbSpan(int count, QRgb *rgb, int offset, int f)
{
	unsigned char *ptr = (unsigned char *) rgb;
	int v0, v1, v2, v3;

	v0 = ptr[0] << V_SHIFT;
	v1 = ptr[1] << V_SHIFT;
	v2 = ptr[2] << V_SHIFT;
	v3 = ptr[3] << V_SHIFT;
	do {
		ptr += offset;
		v0 += (((ptr[0] << V_SHIFT) - v0) * f) >> F_SHIFT; ptr[0] = clampValue(v0);
		v1 += (((ptr[1] << V_SHIFT) - v1) * f) >> F_SHIFT; ptr[1] = clampValue(v1);
		v2 += (((ptr[2] << V_SHIFT) - v2) * f) >> F_SHIFT; ptr[2] = clampValue(v2);
		v3 += (((ptr[3] << V_SHIFT) - v3) * f) >> F_SHIFT; ptr[3] = clampValue(v3);
	} while (--count >= 0);
}


/*-----------------------------------------------------------------------*/

static void blurRgbSpan(int count, QRgb *rgb, int offset, int f)
{
	unsigned char *ptr = (unsigned char *) rgb;
	int v0, v1, v2, v3;

	v0 = ptr[0] << V_SHIFT;
	v1 = ptr[1] << V_SHIFT;
	v2 = ptr[2] << V_SHIFT;
	v3 = ptr[3] << V_SHIFT;
	do {
		ptr += offset;
		v0 += (((ptr[0] << V_SHIFT) - v0) * f) >> F_SHIFT; ptr[0] = (v0 + V_ROUND) >> V_SHIFT;
		v1 += (((ptr[1] << V_SHIFT) - v1) * f) >> F_SHIFT; ptr[1] = (v1 + V_ROUND) >> V_SHIFT;
		v2 += (((ptr[2] << V_SHIFT) - v2) * f) >> F_SHIFT; ptr[2] = (v2 + V_ROUND) >> V_SHIFT;
		v3 += (((ptr[3] << V_SHIFT) - v3) * f) >> F_SHIFT; ptr[3] = (v3 + V_ROUND) >> V_SHIFT;
	} while (--count >= 0);
}


/*-----------------------------------------------------------------------*/

void filterRgbPixels(QRgb *rgb, int w, int h, int stride, int f) {
	if (f < (1 << F_SHIFT)) {
		if (w >= 2) {
			int i = h; while (--i >= 0) {
				blurRgbSpan(w - 2, rgb + i * stride, 4, f);
				blurRgbSpan(w - 2, rgb + i * stride + w - 1, -4, f);
			}
		}
		if (h >= 2) {
			int i = w; while (--i >= 0) {
				blurRgbSpan(h - 2, rgb + i, 4 * stride, f);
				blurRgbSpan(h - 2, rgb + i + (h - 1) * w, -4 * stride, f);
			}
		}
	} else if (f > (1 << F_SHIFT)) {
		if (w >= 2) {
			int i = h; while (--i >= 0) {
				sharpenRgbSpan(w - 2, rgb + i * stride, 4, f);
				sharpenRgbSpan(w - 2, rgb + i * stride + w - 1, -4, f);
			}
		}
		if (h >= 2) {
			int i = w; while (--i >= 0) {
				sharpenRgbSpan(h - 2, rgb + i, 4 * stride, f);
				sharpenRgbSpan(h - 2, rgb + i + (h - 1) * w, -4 * stride, f);
			}
		}
	}
}


/*
 * skulpture_factory.cpp
 *
 */

#include "skulpture_p.h"
#include "sk_factory.h"


/*-----------------------------------------------------------------------*/

bool AbstractFactory::evalCondition()
{
	Code code = *p++;
	if (code < Or) {
		qreal v1 = evalValue();
		qreal v2 = evalValue();
		switch (code) {
			case EQ: return qAbs(v1 - v2) < 1.0e-9;
			case NE: return qAbs(v1 - v2) >= 1.0e-9;
			case LT: return v1 < v2;
			case GE: return v1 >= v2;
			case GT: return v1 > v2;
			case LE: return v1 <= v2;
			default:
				break;
		}
	} else {
		switch (code) {
			case OptionState:
				return opt && (opt->state & (1 << *p++));
			case OptionRTL:
				return opt && (opt->direction != Qt::LeftToRight);
			case OptionVersion:
				return opt && (opt->version >= *p++);
			case OptionType:
				return opt && (!*p || opt->type == *p++);
			case OptionComplex:
				return opt && ((!*p && opt->type >= QStyleOption::SO_Complex) || (opt->type == QStyleOption::SO_Complex + *p++));
			case FactoryVersion:
				return version() >= *p++;
			case Not:
				return !evalCondition();
			case Or:
				if (evalCondition()) {
					skipCondition();
					return true;
				} else {
					return evalCondition();
				}
			case And:
				if (!evalCondition()) {
					skipCondition();
					return false;
				} else {
					return evalCondition();
				}
			default:
				break;
		}
	}
	return false;
}


void AbstractFactory::skipCondition()
{
	Code code = *p++;
	if (code < Or) {
		skipValue();
		skipValue();
	} else {
		skipCondition();
		skipCondition();
	}
}


/*-----------------------------------------------------------------------*/

qreal AbstractFactory::evalValue()
{
	Code code = *p++;
	if (code >= MinVal && code <= MaxVal) {
		return code * 0.01;
	} else if (code >= GetVar + MinVar && code <= GetVar + MaxVar) {
		return var[code - GetVar];
	} else if (code >= Add && code <= Max) {
		qreal v1 = evalValue();
		qreal v2 = evalValue();
		switch (code) {
			case Add: return v1 + v2;
			case Sub: return v1 - v2;
			case Mul: return v1 * v2;
			case Div: return v2 != 0 ? v1 / v2 : 0;
			case Min: return qMin(v1, v2);
			case Max: return qMax(v1, v2);
		}
	} else if (code == Mix) {
		qreal v = evalValue();
		return v * evalValue() + (1 - v) * evalValue();
	} else if (code == Cond) {
		if (evalCondition()) {
			qreal v = evalValue();
			skipValue();
			return v;
		} else {
			skipValue();
			return evalValue();
		}
	}
	return 0;
}


void AbstractFactory::skipValue()
{
	Code code = *p++;
	if (code >= MinVal && code <= MaxVal) {
		return;
	} else if (code >= GetVar + MinVar && code <= GetVar + MaxVar) {
		return;
	} else if (code >= Add && code <= Max) {
		skipValue();
		skipValue();
		return;
	} else if (code == Mix) {
		skipValue();
		skipValue();
		skipValue();
		return;
	} else if (code == Cond) {
		skipCondition();
		skipValue();
		skipValue();
		return;
	}
}


/*-----------------------------------------------------------------------*/

QColor AbstractFactory::evalColor()
{
	Code code = *p++;
	switch (code) {
		case RGB: {
			const quint8 *c = (const quint8 *) p;
			p += 3;
			return QColor(c[0], c[1], c[2]);
		}
		case RGBA: {
			const quint8 *c = (const quint8 *) p;
			p += 4;
			return QColor(c[0], c[1], c[2], c[3]);
		}
		case RGBAf: {
			qreal v[4];
			for (int n = 0; n < 4; ++n) {
				v[n] = qMin(qMax(qreal(0), evalValue()), qreal(1));
			}
			return QColor::fromRgbF(v[0], v[1], v[2], v[3]);
		}
		case Blend: {
			QColor color0 = evalColor();
			QColor color1 = evalColor();
			return blend_color(color0, color1, evalValue());
		}
		case Palette: {
			if (opt) {
				return opt->palette.color(QPalette::ColorRole(*p++));
			}
			break;
		}
		case Shade: {
			QColor color = evalColor();
			return shaded_color(color, int(evalValue() * 200));
		}
		case Darker: {
			QColor color = evalColor();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                        return color.darker(*p++);
#else
                        return color.dark(*p++);
#endif
                }
		case Lighter: {
			QColor color = evalColor();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                        return color.lighter(*p++);
#else
                        return color.light(*p++);
#endif
                }
		default:
			break;
	}
	return QColor();
}


void AbstractFactory::skipColor()
{
	Code code = *p++;
	switch (code) {
		case RGB: {
			p += 3;
			return;
		}
		case RGBA: {
			p += 4;
			return;
		}
		case RGBAf: {
			for (int n = 0; n < 4; ++n) {
				skipValue();
			}
			return;
		}
		case Blend: {
			skipColor();
			skipColor();
			skipValue();
			return;
		}
		case Palette: {
			++p;
			return;
		}
		case Shade: {
			skipColor();
			skipValue();
			return;
		}
		case Darker:
		case Lighter: {
			skipColor();
			p++;
			return;
		}
		default:
			break;
	}
}


/*-----------------------------------------------------------------------*/

void AbstractFactory::executeCode(Code code)
{
	if (code >= SetVar + MinVar && code <= SetVar + MaxVar) {
		var[code - SetVar] = evalValue();
	} else switch (code) {
		case Begin: {
			while (*p != End) {
				Code code = *p++;
				executeCode(code);
			}
			++p;
			return;
		}
		case While: {
			const Code *loop_p = p;
			int counter = 100; // prevent infinite loop
			while (evalCondition() && --counter >= 0) {
				Code code = *p++;
				executeCode(code);
				p = loop_p;
			}
			Code code = *p++;
			skipCode(code);
			return;
		}
		case If: {
			if (evalCondition()) {
				Code code = *p++;
				executeCode(code);
				if (*p == Else) {
					++p;
					Code code = *p++;
					skipCode(code);
				}
			} else {
				Code code = *p++;
				skipCode(code);
				if (*p == Else) {
					++p;
					Code code = *p++;
					executeCode(code);
				}
			}
			return;
		}
	}
}


void AbstractFactory::skipCode(Code code)
{
	if (code >= SetVar + MinVar && code <= SetVar + MaxVar) {
		skipValue();
	} else switch (code) {
		case Begin: {
			while (*p != End) {
				Code code = *p++;
				skipCode(code);
			}
			++p;
			return;
		}
		case While: {
			skipCondition();
			Code code = *p++;
			skipCode(code);
			return;
		}
		case If: {
			skipCondition();
			Code code = *p++;
			skipCode(code);
			if (*p == Else) {
				++p;
				Code code = *p++;
				skipCode(code);
			}
			return;
		}
	}
}


/*-----------------------------------------------------------------------*/

void AbstractFactory::create()
{
	if (p != 0) {
		while (*p != End) {
			Code code = *p++;
			executeCode(code);
		}
	}
}


/*
 * skulpture_frames.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QAbstractSpinBox>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QAbstractItemView>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QPainter>
#include <QtGui/QLCDNumber>
#include <QtGui/QLineEdit>
#include <cmath>


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


static inline bool is_popup_menu(const QWidget *widget)
{
	if (widget) {
		Qt::WindowFlags flags = widget->windowFlags();
		Qt::WindowType type = Qt::WindowType(int(flags & Qt::WindowType_Mask));

		if ((type & Qt::Window) && (flags & Qt::FramelessWindowHint || type == Qt::Popup)) {
			return true;
		}
	}
	return false;
}


void paintStyledFrame(QPainter *painter, const QStyleOptionFrame *option, const QWidget *widget, const QStyle */*style*/)
{
	QPalette::ColorRole bgrole = widget ? widget->backgroundRole() : QPalette::Window;

	if (option->state & QStyle::State_Sunken) {
		if (qobject_cast<const QFrame *>(widget) && widget->parentWidget() && widget->parentWidget()->inherits("KFontRequester")) {
			paintThinFrame(painter, option->rect, option->palette, 60, -20);
			paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -20, 60);
			QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
			panelGradient.setColorAt(0.6, QColor(0, 0, 0, 0));
			panelGradient.setColorAt(1.0, shaded_color(option->palette.color(QPalette::Window), 70));
			painter->fillRect(option->rect.adjusted(2, 2, -2, -2), panelGradient);
		} else {
		/*	if (option->palette.color(QPalette::Base) == QColor(220, 230, 210)) {
				painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.color(QPalette::Base));
				paintRecessedFrame(painter, option->rect, option->palette, RF_Small);
			} else*/ {
				RecessedFrame rf = RF_Large;
				if (!(option->state & QStyle::State_Enabled)
				 || (widget && (!widget->isEnabled() || qobject_cast<const QLCDNumber *>(widget)))) {
					rf = RF_Small;
				}
				if (qobject_cast<const QAbstractItemView *>(widget) || (widget && widget->inherits("Q3ScrollView"))) {
					const QList<QObject *> children = widget->children();
                    Q_FOREACH (QObject *child, children) {
						if (qobject_cast<FrameShadow *>(child)) {
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
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -40, 80, bgrole);
		//	painter->fillRect(option->rect, Qt::red);
		}
	} else {
		// Plain
		if (qobject_cast<const QFrame *>(widget) && widget->parentWidget() && widget->parentWidget()->inherits("KTitleWidget")) {
			QRect r = option->rect;
			bgrole = QPalette::Window;
//			bgrole = QPalette::Base;
#if 1
			QColor bgcolor = option->palette.color(bgrole);
#else
			QColor bgcolor = QColor(230, 230, 230);
#endif
		//	painter->fillRect(r.adjusted(1, 1, -1, -1), bgcolor);
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
		} else if (is_popup_menu(widget)) {
			QRect r = option->rect;
			paintThinFrame(painter, r, option->palette, -60, 160);
			paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 60, bgrole);
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


class LineEditHack : public QLineEdit
{
    public:
        QRect cursorRectHack() const {
#if QT_VERSION >= QT_VERSION_CHECK(4, 4, 0)
            return cursorRect();
#else
            return inputMethodQuery(Qt::ImMicroFocus).toRect();
#endif
        }
};


static QRect getCursorRect(const QWidget *widget)
{
    if (const QLineEdit *lineEdit = qobject_cast<const QLineEdit *>(widget)) {
        const LineEditHack *lineEditHack = reinterpret_cast<const LineEditHack *>(lineEdit);
        /*if (lineEdit->cursorPosition() != lineEdit->text().length())*/ {
            return lineEditHack->cursorRectHack();
        }
    }
    return QRect();
}


void lineEditMouseMoved(QLineEdit *lineEdit, QMouseEvent *event)
{
    if (!lineEdit->hasFocus()) {
        QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(lineEdit->parentWidget());
        int oldCursorPosition = lineEdit->cursorPosition();
        int newCursorPosition = lineEdit->cursorPositionAt(event->pos());

        if (spinBox && lineEdit->text() == spinBox->specialValueText()) {
            // newCursorPosition = lineEdit->text().length();
        } else {
            if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(lineEdit->parentWidget())) {
                newCursorPosition = qBound(spinBox->prefix().length(), newCursorPosition, lineEdit->text().length() - spinBox->suffix().length());
            } else if (QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(lineEdit->parentWidget())) {
                newCursorPosition = qBound(spinBox->prefix().length(), newCursorPosition, lineEdit->text().length() - spinBox->suffix().length());
            }
        }

        if (oldCursorPosition != newCursorPosition) {
            lineEdit->update(getCursorRect(lineEdit).adjusted(-4, -16, 4, 16));
            lineEdit->setCursorPosition(newCursorPosition);
            lineEdit->update(getCursorRect(lineEdit).adjusted(-4, -16, 4, 16));
        }
    }
}


void paintPanelLineEdit(QPainter *painter, const QStyleOptionFrame *option, const QWidget *widget, const QStyle *style)
{
	QPalette::ColorRole bgrole = widget ? widget->backgroundRole() : QPalette::Window;

	bool focus = (option->state & QStyle::State_HasFocus) && !(option->state & QStyle::State_ReadOnly);
        int fw = option->lineWidth;
        if (option->palette.brush(bgrole).style() == Qt::SolidPattern) {
            QRect cursorRect;
                QColor color = option->palette.color(bgrole);
//		printf("style=%d, bgrole=%d, panel color: r=%d, g=%d, b=%d, a=%d\n", option->palette.brush(bgrole).style(), bgrole, color.red(), color.green(), color.blue(), color.alpha());
		if (focus && color.alpha() > 0) {
			color = blend_color(color, option->palette.color(QPalette::Highlight), 0.15);
		} else {
                    focus = false;
#if 1
                    if (option->state & QStyle::State_MouseOver && option->state & QStyle::State_Enabled && !(option->state & QStyle::State_ReadOnly)) {
                        color = color.lighter(103);
                        cursorRect = getCursorRect(widget);
                    } else if (widget) {
                        QWidget *box = widget->parentWidget();
                        if (qobject_cast<QComboBox *>(box) || qobject_cast<QAbstractSpinBox *>(box)) {
                            if (box->underMouse() && option->state & QStyle::State_Enabled) {
                                QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(box);
                                if (!spinBox || !spinBox->isReadOnly()) {
                                    color = color.lighter(103);
                                }
                            }
                        }
                    }
#endif
                }
                painter->fillRect(option->rect.adjusted(fw, fw, -fw, -fw), color);
		if (false && option->state & QStyle::State_Enabled && option->rect.height() <= 64) {
			QLinearGradient panelGradient(option->rect.topLeft(), option->rect.bottomLeft());
			if (color.valueF() > 0.9) {
				panelGradient.setColorAt(0.0, shaded_color(color, -20));
			}
			panelGradient.setColorAt(0.6, shaded_color(color, 0));
			panelGradient.setColorAt(1.0, shaded_color(color, 10));
			painter->fillRect(option->rect.adjusted(fw, fw, -fw, -fw), panelGradient);
		}
                if (!cursorRect.isEmpty()) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
                    QRect cursor = style->subElementRect(QStyle::SE_LineEditContents, option, widget).adjusted(0, 2, 0, -3);
#else
                    QRect cursor = option->rect.adjusted(0, fw + 2, 0, -fw - 3);
#endif
                    if (cursor.height() != option->fontMetrics.height() - 1) {
                        cursor.adjust(0, 1, 0, 0);
                    }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
                    int cursorWidth = style->pixelMetric(QStyle::PM_TextCursorWidth, option, widget);
#else
                    int cursorWidth = style->pixelMetric((QStyle::PixelMetric)((int) QStyle::PM_CustomBase + 1), option, widget);
#endif
                    cursor.setLeft(cursorRect.center().x() + 1 - (cursorWidth >> 1));
                    cursor.setWidth(cursorWidth);
                    cursor.setTop(cursor.top() + ((cursor.height() - option->fontMetrics.height() + 2) >> 1));
                    cursor.setHeight(cursorRect.height() - 2);
                    QColor color = option->palette.color(QPalette::Text);
                    color.setAlpha(60);
                    painter->fillRect(cursor, color);
                    painter->fillRect(QRect(cursor.left() - cursorWidth, cursor.top() - 1, cursorWidth, 1), color);
                    painter->fillRect(QRect(cursor.left() + cursorWidth, cursor.top() - 1, cursorWidth, 1), color);
                    painter->fillRect(QRect(cursor.left() - cursorWidth, cursor.bottom() + 1, cursorWidth, 1), color);
                    painter->fillRect(QRect(cursor.left() + cursorWidth, cursor.bottom() + 1, cursorWidth, 1), color);
                }
        }
	if (focus && option->state & QStyle::State_KeyboardFocusChange) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
            QColor color = option->palette.color(QPalette::Highlight).darker(120);
#else
            QColor color = option->palette.color(QPalette::Highlight).dark(120);
#endif
		color.setAlpha(120);
                QRect r = option->rect.adjusted(fw + 2, fw + 2, -fw - 2, -fw - 2);
                r.setTop(r.top() + r.height() - 1);
		painter->fillRect(r, color);
	}
	if (fw) {
		if (option->state & QStyle::State_ReadOnly && !(option->state & QStyle::State_Enabled)) {
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
                // we need to adjust shadows to the real frame, not that of the line edit
                int left = widget->geometry().left(), right = widget->geometry().right();
		QComboBox *combo = qobject_cast<QComboBox *>(widget->parent());
                int parentFrameWidth;
                if (combo) {
                    if (!combo->hasFrame()) {
                        return;
                    }
                    parentFrameWidth = style->pixelMetric(QStyle::PM_ComboBoxFrameWidth, option, widget);
		} else {
                    QAbstractSpinBox *spin = qobject_cast<QAbstractSpinBox *>(widget->parent());
                    if (spin && !spin->hasFrame()) {
                        return;
                    }
                    parentFrameWidth = style->pixelMetric(QStyle::PM_SpinBoxFrameWidth, option, widget);
                }
                // FIXME use correct right adjustment
                paintRecessedFrameShadow(painter, option->rect.adjusted(parentFrameWidth - left, 0, 100, 0), option->rect.height() <= 64 ? RF_Small : RF_Small);
        }
}


void paintFrameFocusRect(QPainter *painter, const QStyleOptionFocusRect *option, const QWidget *widget)
{
	if (!(option->state & QStyle::State_KeyboardFocusChange)) {
		return;
	}
        if (option->state & QStyle::State_Item) {
            if (widget && widget->window() && !widget->window()->testAttribute(Qt::WA_KeyboardFocusChange)) {
                return;
            }
        }
	QColor color = option->palette.color(QPalette::Highlight);
	color.setAlpha(20);
	painter->fillRect(option->rect, color);
//	painter->fillRect(option->rect.adjusted(1, 1, -1, -1), color);
	painter->fillRect(option->rect.adjusted(2, 2, -2, -2), color);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        color = color.darker(120);
#else
        color = color.dark(120);
#endif
	color.setAlpha(230);
	painter->fillRect(option->rect.adjusted(0, option->rect.height() - 1, 0, 0), color);
}

//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
QRect subControlRectGroupBox(const QStyleOptionGroupBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
{
    switch (subControl) {
        case QStyle::SC_GroupBoxContents:
            return option->rect.adjusted(0, option->fontMetrics.height(), 0, 0);
        case QStyle::SC_GroupBoxCheckBox:
        case QStyle::SC_GroupBoxLabel: {
            if (!(option->features & QStyleOptionFrameV2::Flat)) {
                int x = option->direction == Qt::LeftToRight ? -8 : 8;
                int y = (subControl == QStyle::SC_GroupBoxCheckBox) ? 0 : 1;
                return ((const QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_GroupBox, option, subControl, widget).adjusted(x, y, x, y);
            }
            break;
        }
        default:
            break;
    }
    return ((const QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_GroupBox, option, subControl, widget);
}
//#endif

/*-----------------------------------------------------------------------*/

QGradient path_edge_gradient(const QRectF &rect, const QStyleOption *option, const QPainterPath &path, const QColor &color2, const QColor &color1)
{
	QPointF c = rect.center();
	QColor color[8];
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
	QConicalGradient gradient(c, 0);
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
	return gradient;
}


/*
 * skulpture_gradients.cpp
 *
 */

#include "skulpture_p.h"
#include "sk_factory.h"


/*-----------------------------------------------------------------------*/
/**
 * GradientFactory - create a QGradient from a description
 *
 * The gradient description is a bytecode stream that allows simple arithmetic,
 * conditionals, and looping.
 *
 */


/*-----------------------------------------------------------------------*/

void GradientFactory::executeCode(Code code)
{
	qreal v;

	switch (code) {
		case ColorAt:
			v = evalValue();
			gradient.setColorAt(v, evalColor());
			break;
		default:
			AbstractFactory::executeCode(code);
			break;
	}
}


void GradientFactory::skipCode(Code code)
{
	switch (code) {
		case ColorAt:
			skipValue();
			skipColor();
			break;
		default:
			AbstractFactory::skipCode(code);
			break;
	}
}


/*-----------------------------------------------------------------------*/

QGradient GradientFactory::createGradient(GradientFactory::Description description, qreal var[])
{
	GradientFactory factory;

	factory.setDescription(description);
	for (int n = MinVar; n <= MaxVar; ++n) {
		factory.setVar(n, var[n]);
	}
	factory.create();
	for (int n = MinVar; n <= MaxVar; ++n) {
		var[n] = factory.getVar(n);
	}
	return factory.getGradient();
}


QGradient GradientFactory::createGradient(GradientFactory::Description description)
{
	GradientFactory factory;

	factory.setDescription(description);
	factory.create();
	return factory.getGradient();
}


/*
 * skulpture_header.cpp
 *
 */

#include "skulpture_p.h"
#include "sk_factory.h"
#include <QtGui/QPainter>
#include <QtGui/QHeaderView>


/*-----------------------------------------------------------------------*/

void paintHeaderEmptyArea(QPainter *painter, const QStyleOption *option)
{
	if (option->state & QStyle::State_Enabled) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).lighter(107));
	} else {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).darker(104));
	}
	if (option->state & QStyle::State_Horizontal) {
		paintThinFrame(painter, option->rect.adjusted(0, -2, 32000, -1), option->palette, -20, 60);
//		painter->fillRect(option->rect.adjusted(0, option->rect.height() - 1, 0, 0), QColor(255, 255, 255, 160));
	} else {
		paintThinFrame(painter, option->rect.adjusted(-2, 0, -1, 32000), option->palette, -20, 60);
//		painter->fillRect(option->rect.adjusted(option->rect.width() - 1, 0, 0, 0), QColor(255, 255, 255, 160));
	}
}


static bool isHeaderEnabled(const QStyleOptionHeader *option, const QWidget *widget)
{
    bool enabled = true;
    if (!(option->state & QStyle::State_Enabled)) {
        enabled = false;
        if (widget && widget->inherits("Q3Header")) {
            enabled = widget->isEnabled();
        }
    }
    return enabled;
}


void paintHeaderSection(QPainter *painter, const QStyleOptionHeader *option, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);

	if (!(option->state & (QStyle::State_Raised | QStyle::State_Sunken))) {
		painter->fillRect(option->rect, option->palette.color(QPalette::Window).darker(104));
		paintRecessedFrame(painter, option->rect.adjusted(-9, -9, 3, 3), option->palette, RF_Small);
		painter->fillRect(QRect(option->rect.right(), option->rect.bottom(), 1, 1), option->palette.color(QPalette::Window));
	} else {
                if (isHeaderEnabled(option, widget)) {
			bool hover = false;
			const QHeaderView *view = qobject_cast<const QHeaderView *>(widget);
			if (view && (view->isClickable() || view->isMovable())) {
				hover = option->state & QStyle::State_MouseOver;
			}
			painter->fillRect(option->rect, option->palette.color(QPalette::Base).darker(hover ? 104 : (option->state & QStyle::State_On ? 120 : 106)));
		} else {
			painter->fillRect(option->rect, option->palette.color(QPalette::Window).darker(104));
		}
		if (true || !(option->state & QStyle::State_On)) {
			if (option->orientation == Qt::Horizontal) {
				const QHeaderView *view = qobject_cast<const QHeaderView *>(widget);
				if (view && view->rect().right() == option->rect.right()) {
					paintThinFrame(painter, option->rect.adjusted(0, -2, 1, -1), option->palette, -20, 60);
				} else {
					paintThinFrame(painter, option->rect.adjusted(0, -2, 0, -1), option->palette, -20, 60);
				}
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


void paintHeaderLabel(QPainter *painter, const QStyleOptionHeader *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionHeader opt = *option;
    if (isHeaderEnabled(option, widget)) {
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::Text));
    } else {
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
    }
	painter->save();
	if (widget) {
		painter->setFont(widget->font());
	}
	((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_HeaderLabel, &opt, painter, widget);
	painter->restore();
}


static const ShapeFactory::Code headerSortIndicatorShapeDescription[] = {
	Pmove(-1, 1), Pline(1, 1), Pline(0, -1), Pend
};


void paintHeaderSortIndicator(QPainter *painter, const QStyleOptionHeader *option)
{
	int h = option->fontMetrics.height() / 2 + 2;
	int w = option->fontMetrics.height() / 4 + 2;
	QPainterPath path;

	h /= 2; w /= 2;
	if (option->sortIndicator == QStyleOptionHeader::SortDown) {
		h = -h;
	}
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->translate(option->rect.center());
	painter->translate(0.5, 1.5);
	painter->setPen(Qt::NoPen);
	QColor color = option->palette.color(option->state & QStyle::State_Enabled ? QPalette::Text : QPalette::WindowText);
	color.setAlphaF(0.6 * color.alphaF());
	painter->setBrush(color);
	QMatrix matrix(w, 0, 0, h, 0, 0);
	painter->drawPath(matrix.map(ShapeFactory::createShape(headerSortIndicatorShapeDescription)));
	painter->restore();
}


/*
 * skulpture_icons.cpp
 *
 */

#include "skulpture_p.h"
#include "sk_factory.h"
#include <QtCore/QSettings>
#include <QtGui/QStyleOption>
#include <QtGui/QDockWidget>
#include <QtGui/QFrame>
#include <QtGui/QPainter>
#include "sk_effects.h"
#include <cstdlib>


/*-----------------------------------------------------------------------*/

QPixmap SkulptureStyle::standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const
{
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

static const ShapeFactory::Code titleBarMenuButtonDescription[] = {
    Pmove(0, 0.6), Pline(0.6, 0), Pline(0, -0.6), Pline(-0.6, 0), Pend
};

static const ShapeFactory::Code titleBarCloseButtonDescription[] = {
	#define kx3 0.3
	Pmove(-1, -1), Pline(0, -kx3), Pline(1, -1), Pline(kx3, 0), Pline(1, 1), Pline(0, kx3), Pline(-1, 1), Pline(-kx3, 0), Pend
};

static const ShapeFactory::Code titleBarShadeButtonDescription[] = {
	Pmove(-1, -0.4), Pline(0, -0.6), Pline(1, -0.4), Pline(0, -1), Pend
};

static const ShapeFactory::Code titleBarUnshadeButtonDescription[] = {
	Pmove(-1, -1), Pline(0, -0.8), Pline(1, -1), Pline(0, -0.4), Pend
};

static const ShapeFactory::Code titleBarMinButtonDescription[] = {
	Pmove(-1, 0.4), Pline(0, 0.6), Pline(1, 0.4), Pline(0, 1), Pend
};

static const ShapeFactory::Code titleBarMaxButtonDescription[] = {
	#define kx1 0.8
	#define kx2 0.55
	Pmove(0, -1), Pline(1, 0), Pline(0, 1), Pline(-1, 0), Pclose,
	Pmove(0, -kx2), Pline(-kx1, 0), Pline(0, kx2), Pline(kx1, 0), Pend,
};

static const ShapeFactory::Code titleBarNormalButtonDescription[] = {
    Pmove(0, -1), Pline(1, 0), Pline(0, 1), Pline(-1, 0), Pclose,
    Pmove(0, -kx1), Pline(-kx2, 0), Pline(0, kx1), Pline(kx2, 0), Pend,
};

static const ShapeFactory::Code titleBarHelpButtonDescription[] = {
	Pmove(0.0305, 0.513), Pline(-0.0539, 0.513), Pline(0.0117, 0.227), Pline(0.22, -0.0859), Pline(0.38, -0.323),
	Pline(0.417, -0.491), Pline(0.279, -0.767), Pline(-0.0609, -0.87), Pline(-0.342, -0.814), Pline(-0.445, -0.692),
	Pline(-0.383, -0.568), Pline(-0.321, -0.456), Pline(-0.368, -0.373), Pline(-0.483, -0.339), Pline(-0.64, -0.396),
	Pline(-0.71, -0.555), Pline(-0.512, -0.827), Pline(0.0281, -0.947), Pline(0.649, -0.783), Pline(0.797, -0.516),
	Pline(0.73, -0.31), Pline(0.476, -0.0625), Pline(0.111, 0.255), Pclose,
	Pmove(0.00234, 0.681), Pline(0.165, 0.726), Pline(0.232, 0.834), Pline(0.164, 0.943),
	Pline(0.00234, 0.988), Pline(-0.158, 0.943), Pline(-0.225, 0.834), Pline(-0.158, 0.726), Pend
};

static const ShapeFactory::Code titleBarStickyButtonDescription[] = {
	Pmove(0, -1), Pline(0.2, -0.2), Pline(1, 0), Pline(0.2, 0.2), Pline(0, 1),
	Pline(-0.2, 0.2), Pline(-1, 0), Pline(-0.2, -0.2), Pend
};

static const ShapeFactory::Code titleBarUnstickyButtonDescription[] = {
	Pmove(0, -0.2), Pline(1, 0), Pline(0, 0.2), Pline(-1, 0), Pend
};

static const ShapeFactory::Code titleBarAboveButtonDescription[] = {
	Pmove(0, -0.2), Pline(1, 0), Pline(0, 0.2), Pline(-1, 0), Pclose,
	Pmove(-1, -0.4), Pline(0, -0.6), Pline(1, -0.4), Pline(0, -1), Pend
};

static const ShapeFactory::Code titleBarBelowButtonDescription[] = {
	Pmove(0, -0.2), Pline(1, 0), Pline(0, 0.2), Pline(-1, 0), Pclose,
	Pmove(-1, 0.4), Pline(0, 0.6), Pline(1, 0.4), Pline(0, 1), Pend
};

static const ShapeFactory::Code titleBarUnaboveButtonDescription[] = {
	Pmove(0, -0.2), Pline(1, 0), Pline(0, 0.2), Pline(-1, 0), Pclose,
	Pmove(-1, -1), Pline(0, -0.8), Pline(1, -1), Pline(0, -0.4), Pend
};

static const ShapeFactory::Code titleBarUnbelowButtonDescription[] = {
	Pmove(0, -0.2), Pline(1, 0), Pline(0, 0.2), Pline(-1, 0), Pclose,
	Pmove(-1, 1), Pline(0, 0.8), Pline(1, 1), Pline(0, 0.4), Pend
};

static const ShapeFactory::Code toolBarHorizontalExtensionButtonDescription[] = {
	Pmove(-1, -1), Pline(0, 0), Pline(-1, 1), Pline(-0.5, 0), Pclose,
	Pmove(0, -1), Pline(1, 0), Pline(0, 1), Pline(0.5, 0), Pend
};

static const ShapeFactory::Code toolBarVerticalExtensionButtonDescription[] = {
	Pmove(-1, -1), Pline(0, -0.5), Pline(1, -1), Pline(0, 0), Pclose,
	Pmove(-1, 0), Pline(0, 0.5), Pline(1, 0), Pline(0, 1), Pend
};

static const ShapeFactory::Code * const titleBarButtonDescriptions[] = {
	titleBarMenuButtonDescription,
	titleBarMinButtonDescription,
	titleBarMaxButtonDescription,
	titleBarCloseButtonDescription,
	titleBarNormalButtonDescription,
	titleBarShadeButtonDescription,
	titleBarUnshadeButtonDescription,
	titleBarHelpButtonDescription
};

static const ShapeFactory::Code * const titleBarCustomDescriptions[] = {
	0, // CustomBase intentionally left blank
	titleBarStickyButtonDescription,
	titleBarUnstickyButtonDescription,
	0, // this used to be titleBarIconDescription
	titleBarAboveButtonDescription,
	titleBarBelowButtonDescription,
	titleBarUnaboveButtonDescription,
	titleBarUnbelowButtonDescription
};


static QPainterPath growPath(const QPainterPath &path, double offset, Qt::PenJoinStyle style = Qt::MiterJoin)
{
    QPainterPathStroker stroker;
    QPainterPath path2;
    QPainterPath res;
    bool add;

    if (offset > 0) {
        stroker.setWidth(2.0 * offset);
        add = false;
    } else if (offset < 0) {
        stroker.setWidth(-2.0 * offset);
        add = true;
    } else {
        return path;
    }
    stroker.setCapStyle(Qt::FlatCap);
    stroker.setJoinStyle(style);
    stroker.setDashPattern(Qt::SolidLine);
    stroker.setCurveThreshold(0.1);
    path2 = stroker.createStroke(path);

    for (int i = 0; i < path2.elementCount(); ++i) {
        const QPainterPath::Element &element = path2.elementAt(i);
        switch (element.type)
        {
            case QPainterPath::MoveToElement:
                // copy every other subpath, extracting either inner or outer winding.
                add = !add;
                if (add) {
                    res.moveTo(element.x, element.y);
                }
                break;
            case QPainterPath::LineToElement:
                if (add) {
                    res.lineTo(element.x, element.y);
                }
                break;
            case QPainterPath::CurveToElement: {
                const QPainterPath::Element &element1 = path2.elementAt(++i);
                const QPainterPath::Element &element2 = path2.elementAt(++i);
                if (add) {
                    res.cubicTo(element.x, element.y, element1.x, element1.y, element2.x, element2.y);
                }
                break;
            }
            case QPainterPath::CurveToDataElement:
                break;
        }
    }
    res.setFillRule(Qt::WindingFill);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    res = res.simplified();
#endif
    if (offset < 0) {
        return res.toReversed();
    }
    return res;
}


QIcon SkulptureStyle::standardIconImplementation(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
	const ShapeFactory::Code *code = 0;
        int numStates = 1;
	int size = 10;

	if (standardIcon > QStyle::SP_CustomBase) {
		if (standardIcon - QStyle::SP_CustomBase < sizeof(titleBarCustomDescriptions) / sizeof(titleBarCustomDescriptions[0])) {
			code = titleBarCustomDescriptions[standardIcon - QStyle::SP_CustomBase];
                        numStates = 3;
		}
	} else if (/*standardIcon >= QStyle::SP_TitleBarMenuButton && */standardIcon <= QStyle::SP_TitleBarContextHelpButton) {
		code = titleBarButtonDescriptions[standardIcon - QStyle::SP_TitleBarMenuButton];
                numStates = 3;
        } else {
		switch (standardIcon) {
			case QStyle::SP_ToolBarHorizontalExtensionButton:
				code = toolBarHorizontalExtensionButtonDescription;
                                numStates = 2;
                                size = 8;
				break;
			case QStyle::SP_ToolBarVerticalExtensionButton:
				code = toolBarVerticalExtensionButtonDescription;
                                numStates = 2;
                                size = 8;
				break;
			default:
				break;
		}
	}
	if (code) {
                QIcon icon;
		bool dock = qobject_cast<const QDockWidget *>(widget) != 0;
		if (dock) {
			size = 14;
                        numStates = 2;
                }
		qreal s = size / 2.0;
                if (numStates == 3) {
                    if (widget && !qstrcmp(widget->metaObject()->className(), "KLineEditButton")) {
                        s = qMin(22, widget->fontMetrics().height()) * 0.25;
                        size += 2;
                    } else if (widget && !qstrcmp(widget->metaObject()->className(), "CloseButton")) {
                        s = qMin(20, widget->fontMetrics().height()) * 0.25;
                    } else {
                        size += 4;
                        if (option) {
                            s = qMin(22, option->fontMetrics.height()) * 0.3;
                        }
                    }
                }
                for (int state = 0; state < numStates; ++state) {
                    QImage image(size, size, QImage::Format_ARGB32);
                    image.fill(0);
                    QPainter painter(&image);
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.translate(size / 2.0, size / 2.0);
                    if (dock) {
                            painter.scale(s - 2, s - 2);
                    } else {
                            painter.scale(s, s);
                    }
                    painter.setPen(Qt::NoPen);
                    QPalette palette;
                    if (option) {
                        palette = option->palette;
                    }
                    QPalette::ColorRole role = QPalette::Text;
                    QColor shapeColor;
                    if (numStates == 2) {
                        if (state == 0) {
                            role = QPalette::WindowText;
                        } else {
                            role = QPalette::ButtonText;
                        }
                        shapeColor = palette.color(role);
                    } else if (numStates == 3) {
                        if (state == 1) {
                            QColor glowColor = palette.color(role);
                            if (standardIcon == QStyle::SP_TitleBarCloseButton) {
                                glowColor = QColor(255, 0, 0, 100);
                            } else {
                                glowColor.setAlpha(50);
                            }
#if 0
                            painter.setBrush(glowColor);
                            QPainterPath path = ShapeFactory::createShape(code);
                            path = growPath(path, 0.15);
                            painter.drawPath(path);
                            filterImage(image, 0.9);
                            shapeColor = palette.color(role);
#else
                            painter.fillRect(QRectF(-1.5, -1.5, 3, 3), glowColor);
                            QPainterPath path = ShapeFactory::createShape(code);
                            shapeColor = palette.color(role);
//                            painter.setPen(QPen(shapeColor, 0.3));
                            painter.drawRect(QRectF(-1.5, -1.5, 3, 3));
                            painter.setPen(Qt::NoPen);
                            painter.setBrush(shapeColor);
                            painter.drawPath(path);
#endif
                        } else {
                            shapeColor = palette.color(role);
                        }
                    }
                    QColor shadowColor = option ? option->palette.color(QPalette::Shadow) : Qt::black;
                    shadowColor.setAlpha(25);
                    painter.translate(1 / s, 1 / s);
                    painter.setBrush(shadowColor);
                    painter.drawPath(ShapeFactory::createShape(code));
                    painter.translate(-1 / s, -1 / s);
                    painter.setBrush(shapeColor);
                    painter.drawPath(ShapeFactory::createShape(code));
                    painter.end();
                    QIcon::Mode iconMode;
                    switch (state) {
                        case 1:
                            iconMode = QIcon::Active;
                            break;
                        case 2:
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
                            iconMode = QIcon::Selected;
#else
                            iconMode = QIcon::Normal;
#endif
                            break;
                        default:
                            iconMode = QIcon::Normal;
                            break;
                    }
                    icon.addPixmap(QPixmap::fromImage(image), iconMode);
                }
                return icon;
	}
	return ParentStyle::standardIconImplementation(standardIcon, option, widget);
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
 * skulpture_kde4.cpp
 *
 */

#include "skulpture_p.h"
//#include "config.h"

#if KDE4_FOUND
/*-----------------------------------------------------------------------*/

QVariant readKdeSetting(const QString &entry)
{
    return QVariant();
}

#else
/*-----------------------------------------------------------------------*/

QVariant readKdeSetting(const QString &entry)
{
    return QVariant();
}

#endif

/*
 * skulpture_layout.cpp
 *
 */

#include "skulpture_p.h"
#include <QtCore/QSettings>
#include <QtGui/QFrame>
#include <QtGui/QApplication>
#include <QtGui/QShortcut>
#include <QtGui/QLayout>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#endif


/*-----------------------------------------------------------------------*/

int SkulptureStyle::Private::verticalTextShift(const QFontMetrics &fontMetrics)
{
    if (fontMetrics == qApp->fontMetrics()) {
        return textShift;
    }
    QFont boldFont;
    boldFont.setBold(true);
    if (fontMetrics == QFontMetrics(boldFont)) {
        return textShift;
    }
    return 0;
}


static QFontMetrics styledFontMetrics(const QStyleOption *option, const QWidget *widget)
{
    if (option) {
        return option->fontMetrics;
    } else if (widget) {
        return widget->fontMetrics();
    }
    return qApp->fontMetrics();
}


int SkulptureStyle::Private::textLineHeight(const QStyleOption *option, const QWidget *widget)
{
    QFontMetrics fm = styledFontMetrics(option, widget);
    return fm.height() + (verticalTextShift(fm) & 1);
}


static int fontHeight(const QStyleOption *option, const QWidget *widget)
{
    return styledFontMetrics(option, widget).height();
}


/*-----------------------------------------------------------------------*/

int SkulptureStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
        /* entries are stricly sorted in Qt order for future lookup table */
        case PM_ButtonMargin: return d->pushButtonSize;
        case PM_ButtonDefaultIndicator: return 0;
        case PM_MenuButtonIndicator: return fontHeight(option, widget);
        case PM_ButtonShiftHorizontal: {
            Qt::LayoutDirection direction;
            if (option) {
                direction = option->direction;
            } else if (widget) {
                direction = widget->layoutDirection();
            } else {
                direction = QApplication::layoutDirection();
            }
            return direction == Qt::LeftToRight ? 1 : -1;
        }
        case PM_ButtonShiftVertical: return 1;

        case PM_DefaultFrameWidth: return 2; // 3 for command buttons
        case PM_SpinBoxFrameWidth: return 2;
        case PM_ComboBoxFrameWidth: return 2; // 3 for non-editable combobox (in button mode)

        case PM_MaximumDragDistance: return -1;

        case PM_ScrollBarExtent: {
            if (d->scrollBarSize > 0) {
                return d->scrollBarSize;
            }
             // do not depend on widget font size
            int extent = ((fontHeight(0, 0) >> 1) & ~1) + 9;
#if 0
            if (option && (option->state & QStyle::State_Horizontal)) {
                return (qMax(extent, qApp->globalStrut().height()) & ~1) + 1;
            } else {
                return (qMax(extent, qApp->globalStrut().width()) & ~1) + 1;
            }
#else
            return extent;
#endif
        }
        case PM_ScrollBarSliderMin: {
            if (d->scrollBarLength > 0) {
                return d->scrollBarLength;
            }
            return fontHeight(0, 0) + 1;
        }

        case PM_SliderThickness:
        case PM_SliderControlThickness: {
            if (d->sliderSize > 0) {
                return d->sliderSize + 4;
            }
            int thickness = (fontHeight(option, widget) & ~1) + 5;
            if (option && !(option->state & QStyle::State_Horizontal)) {
                return (qMax(thickness, qApp->globalStrut().width()) & ~1) + 1;
            } else {
                return (qMax(thickness, qApp->globalStrut().height()) & ~1) + 1;
            }
        }

        case PM_SliderLength: {
            if (d->sliderLength > 0) {
                return d->sliderLength;
            }
            int length = (fontHeight(option, widget)) + 6;
            if (option && !(option->state & QStyle::State_Horizontal)) {
                return qMax(length, qApp->globalStrut().height());
            } else {
                return qMax(length, qApp->globalStrut().width());
            }
        }
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
        case PM_SliderSpaceAvailable: {
            return QCommonStyle::pixelMetric(metric, option, widget);
        }

        case PM_DockWidgetSeparatorExtent: return ((qMax(fontHeight(option, widget), 16) >> 1) & ~1) - 1;
        case PM_DockWidgetHandleExtent: return 8;
        case PM_DockWidgetFrameWidth: return 2;

        case PM_TabBarTabOverlap: return 2;
        case PM_TabBarTabHSpace: return 0;
        case PM_TabBarTabVSpace: return 0;
        case PM_TabBarBaseHeight: return 2;
        case PM_TabBarBaseOverlap: return 2;

        case PM_ProgressBarChunkWidth: return fontHeight(option, widget) >> 1;

        case PM_SplitterWidth: return ((qMax(fontHeight(option, widget), 16) >> 1) & ~1) - 1;
        case PM_TitleBarHeight: return d->textLineHeight(option, widget) + 4;

        case PM_MenuScrollerHeight: return (fontHeight(option, widget) >> 1) + 2;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        case PM_MenuHMargin: return 1;
        case PM_MenuVMargin: return 1;
        case PM_MenuPanelWidth: return 1;
#else
        case PM_MenuHMargin: return 0; // ### anything other than 0 messes Qt's menu positioning code ...
        case PM_MenuVMargin: return 0;
        case PM_MenuPanelWidth: return 2;
#endif
        case PM_MenuTearoffHeight: return (fontHeight(option, widget) >> 1) + 2;
        case PM_MenuDesktopFrameWidth: return 0;

        case PM_MenuBarPanelWidth: return 0;
        case PM_MenuBarItemSpacing: return 0;
        case PM_MenuBarVMargin: return 1;
        case PM_MenuBarHMargin: return 1;

        case PM_IndicatorWidth:
        case PM_IndicatorHeight:
        case PM_ExclusiveIndicatorWidth:
        case PM_ExclusiveIndicatorHeight:
        case PM_CheckListButtonSize:
        case PM_CheckListControllerSize: {
            // do not use strut width, because label is usually wide enough
            return qMax(d->textLineHeight(option, widget), QApplication::globalStrut().height());
        }

        case PM_DialogButtonsSeparator: return 6;
        case PM_DialogButtonsButtonWidth: return 64;
        case PM_DialogButtonsButtonHeight: return 16;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case PM_MdiSubWindowFrameWidth: return 3;
        case PM_MdiSubWindowMinimizedWidth: return fontHeight(option, widget) * 12;
#else
        case PM_MDIFrameWidth: return 3;
        case PM_MDIMinimizedWidth: return fontHeight(option, widget) * 12;
#endif

        case PM_HeaderMargin: return 3;
        case PM_HeaderMarkSize: return 5;
        case PM_HeaderGripMargin: return 4;
        case PM_TabBarTabShiftHorizontal: return 0;
        case PM_TabBarTabShiftVertical: return 0;
        case PM_TabBarScrollButtonWidth: return (fontHeight(option, widget) & ~1) + 1;

        case PM_ToolBarFrameWidth: return 1;
        case PM_ToolBarHandleExtent: return 9;
        case PM_ToolBarItemSpacing: return 2;
        case PM_ToolBarItemMargin: return 0;
        case PM_ToolBarSeparatorExtent: return 4;
        case PM_ToolBarExtensionExtent: return 12;

        case PM_SpinBoxSliderHeight: return 2;

        case PM_DefaultTopLevelMargin: {
            if (d->dialogMargins >= 0) {
                return d->dialogMargins;
            }
            return (fontHeight(option, widget) >> 1);
        }
        case PM_DefaultChildMargin: {
            if (d->widgetMargins >= 0) {
                return d->widgetMargins;
            }
            return (fontHeight(option, widget) >> 1);
        }
        case PM_DefaultLayoutSpacing: {
            if (d->verticalSpacing >= 0) {
                return d->verticalSpacing;
            }
            return ((fontHeight(option, widget) * 3) >> 3);
        }

        case PM_ToolBarIconSize: return pixelMetric(PM_SmallIconSize, option, widget);
        case PM_ListViewIconSize: return pixelMetric(PM_SmallIconSize, option, widget);
        case PM_IconViewIconSize: return pixelMetric(PM_LargeIconSize, option, widget);
        case PM_SmallIconSize: {
            int iconSize = d->textLineHeight(option, widget);
            return iconSize;
        }
        case PM_LargeIconSize: {
            const bool hasSvgIcons = false;
            int iconSize = 2 * fontHeight(option, widget);
            if (!hasSvgIcons) {
                if (iconSize < 28) {
                    return 22;
                } else if (iconSize < 40) {
                    return 32;
                } else if (iconSize < 56) {
                    return 48;
                } else if (iconSize < 96) {
                    return 64;
                }
            }
            return iconSize;
        }
        case PM_FocusFrameVMargin: return 2;
        case PM_FocusFrameHMargin: return 2;

        case PM_ToolTipLabelFrameWidth: return 1;
        case PM_CheckBoxLabelSpacing: {
            if (d->labelSpacing >= 0) {
                return d->labelSpacing;
            }
            return (fontHeight(option, widget) >> 1) - 2;
        }
        case PM_TabBarIconSize: return pixelMetric(PM_SmallIconSize, option, widget);
        case PM_SizeGripSize: return 13; // ### make this variable
        case PM_DockWidgetTitleMargin: return 2;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
        case PM_MessageBoxIconSize: return pixelMetric(PM_LargeIconSize, option, widget);
        case PM_ButtonIconSize: return pixelMetric(PM_SmallIconSize, option, widget);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case PM_DockWidgetTitleBarButtonMargin: return 0;
        case PM_RadioButtonLabelSpacing:  {
            if (d->labelSpacing >= 0) {
                return d->labelSpacing;
            }
            return (fontHeight(option, widget) >> 1) - 2;
        }
        case PM_LayoutLeftMargin:
        case PM_LayoutTopMargin:
        case PM_LayoutRightMargin:
        case PM_LayoutBottomMargin: {
            PixelMetric metric = QStyle::PM_DefaultChildMargin;
            if ((option && (option->state & QStyle::State_Window)) || (widget && widget->isWindow())) {
                metric = QStyle::PM_DefaultTopLevelMargin;
            }
            return pixelMetric(metric, option, widget);
        }

        case PM_LayoutHorizontalSpacing:
        case PM_LayoutVerticalSpacing: {
            return -1;
        }
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        case PM_TabBar_ScrollButtonOverlap: return 0;
        case PM_TextCursorWidth: {
            if (d->textCursorWidth > 0) {
                return qMax(1, int(d->textCursorWidth + 0.5));
            }
            return qMax(1, (fontHeight(option, widget) + 8) / 12);
        }
#else
        // used for TextCursorWidth in Qt < 4.4
        case PM_CustomBase + 1: {
            if (d->textCursorWidth > 0) {
                return qMax(1, int(d->textCursorWidth + 0.5));
            }
            return qMax(1, (fontHeight(option, widget) + 8) / 12);
        }
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
        case PM_TabCloseIndicatorWidth:
        case PM_TabCloseIndicatorHeight: {
            return (fontHeight(option, widget) & ~1);
        }
        case PM_ScrollView_ScrollBarSpacing: return 0;
        case PM_SubMenuOverlap: return -2;
#endif

        case PM_CustomBase: // avoid warning
            break;
    }
    return ParentStyle::pixelMetric(metric, option, widget);
}


/*-----------------------------------------------------------------------*/
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
int SkulptureStyle::layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const
{
    Q_UNUSED(control2);

    if (orientation == Qt::Horizontal) {
        if (control1 == QSizePolicy::Label) {
            if (d->labelSpacing >= 0) {
                return d->labelSpacing + 2;
            }
            return fontHeight(option, widget) >> 1;
        }
        if (d->horizontalSpacing >= 0) {
            return d->horizontalSpacing;
        }
        return fontHeight(option, widget) >> 1;
    }
    if (control1 & (QSizePolicy::CheckBox | QSizePolicy::RadioButton)
     && control2 & (QSizePolicy::CheckBox | QSizePolicy::RadioButton)) {
        if (d->verticalSpacing >= 0) {
            return qMax(0, d->verticalSpacing - 2);
        }
        return pixelMetric(PM_DefaultLayoutSpacing, option, widget) - 2;
    }
    if (d->verticalSpacing >= 0) {
        return d->verticalSpacing;
    }
    return pixelMetric(PM_DefaultLayoutSpacing, option, widget);
}
#endif

/*-----------------------------------------------------------------------*/

extern QSize sizeFromContentsToolButton(const QStyleOptionToolButton *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int toolButtonSize);
extern QSize sizeFromContentsMenuItem(const QStyleOptionMenuItem *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int menuItemSize, int textLineHeight);


/*-----------------------------------------------------------------------*/

static inline QSize sizeFromContentsPushButton(const QStyleOptionButton *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int pushButtonSize, int textLineHeight)
{
    Q_UNUSED(style);

    // width
    int w = contentsSize.width() + (fontHeight(option, widget) & ~1);
    if (!option->text.isEmpty()) {
        w += 6 + 2 * pushButtonSize;
        const int sizeBase = qMin(64, 4 * fontHeight(option, widget));
        const int sizeIncrement = qMin(32, qMin(sizeBase, qMax(1, 2 * pushButtonSize)));
        if (w < sizeBase) {
            w = sizeBase;
        } else {
            w = ((w - sizeBase + sizeIncrement - 1) / sizeIncrement) * sizeIncrement + sizeBase;
        }
    }

    // height
    int h = qMax(contentsSize.height(), textLineHeight);
    h += 2 * pushButtonSize + 4;

    return QSize(w, h);
}


static inline QSize sizeFromContentsComboBox(const QStyleOptionComboBox *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int widgetSize, int textLineHeight)
{
//    if (contentsSize.height() > 16) {
//        fh = contentsSize.height() - 2;
//    }
    return ((const QCommonStyle *) style)->QCommonStyle::sizeFromContents(QStyle::CT_ComboBox, option, QSize(contentsSize.width(), textLineHeight + 2 * widgetSize), widget);
}


static inline QSize sizeFromContentsLineEdit(const QStyleOptionFrame *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int widgetSize, int textLineHeight)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    int fw = option->lineWidth;
    int fh = textLineHeight;
//    if (contentsSize.height() > 14) {
//        fh = contentsSize.height() - 4;
//    }
    return QSize(contentsSize.width() + 6 + 2 * fw, fh + 2 * (widgetSize + fw));
}


static inline QSize sizeFromContentsSpinBox(const QStyleOptionSpinBox *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style)
{
    Q_UNUSED(option); Q_UNUSED(widget); Q_UNUSED(style);

    return contentsSize + QSize(4, 0);
}


static inline QSize sizeFromContentsGroupBox(const QStyleOptionGroupBox *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    if (!(option->features & QStyleOptionFrameV2::Flat)) {
        return contentsSize + QSize(fontHeight(option, widget) & ~1, 0);
    }
    return contentsSize;
}


static inline QSize sizeFromContentsTabBarTab(const QStyleOptionTab *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int tabBarSize, int textShift)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    if (int(option->shape) & 2) {
        return (contentsSize + QSize(8, 24)).expandedTo(QApplication::globalStrut());
    }
    if (!option->icon.isNull()) {
        textShift = 0;
    }
    return (contentsSize + QSize(2 * tabBarSize + (fontHeight(option, widget) & ~1), 2 + 2 * tabBarSize + (textShift & 1))).expandedTo(QApplication::globalStrut());
}


static inline QSize sizeFromContentsProgressBar(const QStyleOptionProgressBar *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int widgetSize, int textShift)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    if (option->version >= 2 && ((const QStyleOptionProgressBarV2 *) option)->orientation == Qt::Vertical) {
        return contentsSize + QSize(2 * widgetSize, 6);
    }
    return contentsSize + QSize(6, (textShift & 1) + 2 * widgetSize - 6);
}


/*-----------------------------------------------------------------------*/

#define CT_CASE(ct, so) \
    case CT_## ct: \
        if (option && option->type == QStyleOption::SO_## so) { \
            return sizeFromContents ## ct((const QStyleOption ## so *) option, contentsSize, widget, this); \
        } \
        break


QSize SkulptureStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    switch (type) {
        case CT_PushButton:
            if (option && option->type == QStyleOption::SO_Button) {
                return sizeFromContentsPushButton((const QStyleOptionButton *) option, contentsSize, widget, this, d->pushButtonSize, d->textLineHeight(option, widget));
            }
            break;
        case CT_CheckBox:
        case CT_RadioButton:
            return ParentStyle::sizeFromContents(type, option, contentsSize, widget) + QSize(0, 2 * d->widgetSize - 4);
            break;

        case CT_ToolButton:
            if (option && option->type == QStyleOption::SO_ToolButton) {
                return sizeFromContentsToolButton((const QStyleOptionToolButton *) option, contentsSize, widget, this, d->toolButtonSize);
            }
            break;
        case CT_ComboBox:
            if (option && option->type == QStyleOption::SO_ComboBox) {
                return sizeFromContentsComboBox((const QStyleOptionComboBox *) option, contentsSize, widget, this, d->widgetSize, d->textLineHeight(option, widget));
            }
            break;

        case CT_Splitter:
        case CT_Q3DockWindow:
            return contentsSize;
        case CT_ProgressBar:
            if (option && option->type == QStyleOption::SO_ProgressBar) {
                return sizeFromContentsProgressBar((const QStyleOptionProgressBar *) option, contentsSize, widget, this, d->widgetSize, d->verticalTextShift(styledFontMetrics(option, widget)));
            }
            break;

        case CT_MenuItem:
            if (option && option->type == QStyleOption::SO_MenuItem) {
                return sizeFromContentsMenuItem((const QStyleOptionMenuItem *) option, contentsSize, widget, this, 2 * d->menuItemSize, d->textLineHeight(option, widget));
            }
            break;
        case CT_MenuBarItem: {
            int h = 2 * (d->menuBarSize >= 0 ? d->menuBarSize : 2) + d->textLineHeight(option, widget);
            return QSize(contentsSize.width() + (((fontHeight(option, widget) * 7) >> 3) & ~1), h).expandedTo(qApp->globalStrut());
        }
        case CT_MenuBar: return contentsSize;
        case CT_Menu: return contentsSize;

        case CT_TabBarTab:
            if (option && option->type == QStyleOption::SO_Tab) {
                return sizeFromContentsTabBarTab((const QStyleOptionTab *) option, contentsSize, widget, this, d->tabBarSize, d->verticalTextShift(styledFontMetrics(option, widget)));
            }
            break;

        case CT_Slider:
        case CT_ScrollBar:
        case CT_Q3Header:
            return contentsSize;

        case CT_LineEdit:
            if (option && option->type == QStyleOption::SO_Frame) {
                return sizeFromContentsLineEdit((const QStyleOptionFrame *) option, contentsSize, widget, this, d->widgetSize, d->textLineHeight(option, widget));
            }
            break;
        CT_CASE(SpinBox, SpinBox);

        case CT_SizeGrip: return contentsSize;
        case CT_TabWidget: return contentsSize + QSize(4, 4);
        case CT_DialogButtons: return contentsSize;
        case CT_HeaderSection: break;

//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
        CT_CASE(GroupBox, GroupBox);
//#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case CT_MdiControls: break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        case CT_ItemViewItem: break;
#endif

        case CT_CustomBase: // avoid warning
            break;
    }
    return ParentStyle::sizeFromContents(type, option, contentsSize, widget);
}


/*-----------------------------------------------------------------------*/

extern QRect subElementRectDockWidget(QStyle::SubElement element, const QStyleOptionDockWidget *option, const QWidget *widget, const QStyle *style);
extern QRect subElementRectComboBoxFocusRect(const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style);


/*-----------------------------------------------------------------------*/
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
static inline QRect subElementRectFrameContents(const QStyleOption *option, const QWidget *widget, const QStyle *style)
{
    Q_UNUSED(style);

    if (widget && widget->inherits("KHTMLView")) {
        QWidget *window;
        // case 1: the parent widget is a QFrame, and already has a sunken frame
        //  do not show any frame (KMail mail view, Kopete chat view)
        if (QFrame *frame = qobject_cast<QFrame *>(widget->parentWidget())) {
            if (frame->frameShape() == QFrame::StyledPanel) {
                // ### fix Kopete frame
                // frame->setFrameShadow(QFrame::Sunken);
                return option->rect;
            }
        } else if ((window = widget->window()) && window->inherits("KonqMainWindow")) {
            // case 2: the html view covers the full width of window:
            //  do not show side frames (Konqueror)
            return option->rect.adjusted(0, 2, 0, -2);
        } else {
            // case 3: detect KMail 4.2
            while ((widget = widget->parentWidget())) {
                if (widget->inherits("KMReaderWin")) {
                    return option->rect;
                }
            }
        }
    }
    return option->rect.adjusted(2, 2, -2, -2);
}


static inline QRect subElementRectLineEditContents(const QStyleOptionFrame *option, const QWidget *widget, const QStyle *style, int textShift)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    int fw = option->lineWidth;
    if (textShift & 1 && !(option->rect.height() & 1)) {
        textShift += 1;
    }
    return option->rect.adjusted(fw + 2, fw + ((-textShift) >> 1), -fw - 2, -fw + ((-textShift) >> 1));
}
#endif

/*-----------------------------------------------------------------------*/

QRect SkulptureStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
        case SE_PushButtonContents:
        case SE_PushButtonFocusRect:

        case SE_CheckBoxIndicator:
        case SE_CheckBoxContents:
        case SE_CheckBoxFocusRect:
        case SE_CheckBoxClickRect:

        case SE_RadioButtonIndicator:
        case SE_RadioButtonContents:
        case SE_RadioButtonFocusRect:
        case SE_RadioButtonClickRect:
            break;

        case SE_ComboBoxFocusRect:
            if (option->type == QStyleOption::SO_ComboBox) {
                return subElementRectComboBoxFocusRect((const QStyleOptionComboBox *) option, widget, this);
            }
            break;

        case SE_SliderFocusRect:

        case SE_Q3DockWindowHandleRect:
            break;

        case SE_ProgressBarGroove:
        case SE_ProgressBarContents:
        case SE_ProgressBarLabel:
            return option->rect;

        case SE_DialogButtonAccept:
        case SE_DialogButtonReject:
        case SE_DialogButtonApply:
        case SE_DialogButtonHelp:
        case SE_DialogButtonAll:
        case SE_DialogButtonAbort:
        case SE_DialogButtonIgnore:
        case SE_DialogButtonRetry:
        case SE_DialogButtonCustom:
            break;

        case SE_ToolBoxTabContents:
            return option->rect.adjusted(11, 0, -6, 0);

        case SE_HeaderLabel:
        case SE_HeaderArrow:
            break;

        case SE_TabWidgetTabBar:
        case SE_TabWidgetTabPane:
        case SE_TabWidgetTabContents:
            break;
        case SE_TabWidgetLeftCorner:
        case SE_TabWidgetRightCorner:
            return QCommonStyle::subElementRect(element, option, widget).adjusted(1, 1, -1, 1);

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        case SE_ItemViewItemCheckIndicator:
#else
        case SE_ViewItemCheckIndicator:
#endif
            break;

        case SE_TabBarTearIndicator:

        case SE_TreeViewDisclosureItem:
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
        case SE_LineEditContents:
            if (option->type == QStyleOption::SO_Frame) {
                return subElementRectLineEditContents((const QStyleOptionFrame *) option, widget, this, d->textShift);
            }
            break;
        case SE_FrameContents:
            return subElementRectFrameContents(option, widget, this);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        case SE_DockWidgetCloseButton:
        case SE_DockWidgetFloatButton:
        case SE_DockWidgetTitleBarText:
        case SE_DockWidgetIcon:
            if (option->type == QStyleOption::SO_DockWidget) {
                return subElementRectDockWidget(element, (const QStyleOptionDockWidget *) option, widget, this);
            }
            break;

        case SE_CheckBoxLayoutItem:
        case SE_ComboBoxLayoutItem:
        case SE_DateTimeEditLayoutItem:
            break;
        case SE_DialogButtonBoxLayoutItem: break;
        case SE_LabelLayoutItem:
        case SE_ProgressBarLayoutItem:
        case SE_PushButtonLayoutItem:
        case SE_RadioButtonLayoutItem:
        case SE_SliderLayoutItem:
        case SE_SpinBoxLayoutItem:
        case SE_ToolButtonLayoutItem:
            break;

        case SE_FrameLayoutItem:
        case SE_GroupBoxLayoutItem:
        case SE_TabWidgetLayoutItem:
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        case SE_ItemViewItemDecoration:
        case SE_ItemViewItemText:
        case SE_ItemViewItemFocusRect:
            break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
        case SE_TabBarTabLeftButton:
        case SE_TabBarTabRightButton:
        case SE_TabBarTabText:
            break;
        case SE_ShapedFrameContents:
            break;
#endif
        case SE_CustomBase: // avoid warning
            break;
    }
    return ParentStyle::subElementRect(element, option, widget);
}


/*-----------------------------------------------------------------------*/
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void SkulptureStyle::Private::polishFormLayout(QFormLayout *layout)
{
    if (layout->labelAlignment() & Qt::AlignVCenter) {
        return;
    }
    int addedHeight = -1;
    for (int row = 0; row < layout->rowCount(); ++row) {
        QLayoutItem *labelItem = layout->itemAt(row, QFormLayout::LabelRole);
        if (!labelItem) {
            continue;
        }
        QLayoutItem *fieldItem = layout->itemAt(row, QFormLayout::FieldRole);
        if (!fieldItem) {
            continue;
        }
        QWidget *label = labelItem->widget();
        if (!label) {
            continue;
        }
        int labelHeight;
        if (addedHeight < 0) {
#if 0
            // fixed value in Qt
            static const int verticalMargin = 1;
            QStyleOptionFrame option;
            option.initFrom(label);
            option.lineWidth = label->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, label);
            option.midLineWidth = 0;
            option.rect = QRect(0, 0, 10, fontHeight(option, label) + 2 * verticalMargin);
            // label should be aligned centered to LineEdit, so use its size
            addedHeight = label->style()->sizeFromContents(QStyle::CT_LineEdit, &option, option.rect.size(), label).height() - fontHeight(option, height);
#else
            addedHeight = 4 + 2 * widgetSize;
#endif
        }
        if (qobject_cast<QLabel *>(label)) {
            labelHeight = label->sizeHint().height() + addedHeight;
        } else if (qobject_cast<QCheckBox *>(label)) {
            labelHeight = label->sizeHint().height();
        } else {
            continue;
        }
        int fieldHeight = fieldItem->sizeHint().height();
        // work around KIntNumInput::sizeHint() bug
        if (fieldItem->widget() && fieldItem->widget()->inherits("KIntNumInput")) {
            fieldHeight -= 2;
            fieldItem->widget()->setMaximumHeight(fieldHeight);
        }
        /* for large fields, we don't center */
        if (fieldHeight <= 2 * fontHeight(0, label) + addedHeight) {
            if (fieldHeight > labelHeight) {
                labelHeight = fieldHeight;
            }
        } else {
            if (verticalTextShift(label->fontMetrics()) & 1) {
                labelHeight += 1;
            }
        }
        if (qobject_cast<QCheckBox *>(label)) {
            label->setMinimumHeight(labelHeight);
        } else {
            // QFormLayout determines label size as height * 5 / 4, so revert that
            label->setMinimumHeight((labelHeight * 4 + 4) / 5);
        }
    }
}
#endif

void SkulptureStyle::Private::polishLayout(QLayout *layout)
{
    if (forceSpacingAndMargins) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        if (QFormLayout *formLayout = qobject_cast<QFormLayout *>(layout)) {
            if (formLayout->spacing() >= 2) {
                formLayout->setSpacing(-1);
            }
        } else
#endif
        if (QGridLayout *gridLayout = qobject_cast<QGridLayout *>(layout)) {
            if (gridLayout->spacing() >= 2) {
                gridLayout->setSpacing(-1);
            }
        } else if (QBoxLayout *boxLayout = qobject_cast<QBoxLayout *>(layout)) {
            if (boxLayout->spacing() >= 2) {
                boxLayout->setSpacing(-1);
            }
        } else {
            if (layout->spacing() >= 2) {
                layout->setSpacing(-1);
            }
        }
        if (layout->margin() >= 4) {
            layout->setMargin(-1);
        }
    }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
    if (QFormLayout *formLayout = qobject_cast<QFormLayout *>(layout)) {
        polishFormLayout(formLayout);
    }
#endif
    // recurse into layouts
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);
        if (QLayout *layout = item->layout()) {
            polishLayout(layout);
        }
    }
}


/*
 * skulpture_mdi.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
#include <QtGui/QMdiSubWindow>
#endif
#include <QtCore/QSettings>
#include <cmath>


/*-----------------------------------------------------------------------*/

QRect subControlRectTitleBar(const QStyleOptionTitleBar *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
{
    QRect r = ((const QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_TitleBar, option, subControl, widget);
    if (subControl != QStyle::SC_TitleBarSysMenu) {
        return r.adjusted(option->direction == Qt::LeftToRight ? -2 : 2, -2, option->direction == Qt::LeftToRight ? -3 : 3, -3);
    } else {
        return r.adjusted(0, -1, 0, -1);
    }
}


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


void paintTitleBar(QPainter *painter, const QStyleOptionTitleBar *option, const QWidget *widget, const QStyle *style)
{
	QColor barColor;
	QColor textColor;

	painter->save();
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
        qreal opacity = painter->opacity();
#endif

	QPalette palette = option->palette;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        if (qobject_cast<const QMdiSubWindow *>(widget)) {
		if (widget->objectName() != QLatin1String("SkulpturePreviewWindow")) {
			getTitleBarPalette(palette);
		}
	}
#endif
	if (option->state & QStyle::State_Active) {
		barColor = palette.color(QPalette::Highlight);
		textColor = palette.color(QPalette::HighlightedText);
	} else {
		barColor = palette.color(QPalette::Window);
		textColor = palette.color(QPalette::WindowText);
	}

	QLinearGradient barGradient(option->rect.topLeft() + QPoint(-1, -1), option->rect.bottomLeft() + QPoint(-1, -2));
//	barGradient.setColorAt(0.0, option->palette.color(QPalette::Window));
//	barGradient.setColorAt(0.3, barColor);
//	barGradient.setColorAt(0.7, barColor);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        barGradient.setColorAt(0.0, barColor.darker(105));
        barGradient.setColorAt(1.0, barColor.lighter(120));
#else
        barGradient.setColorAt(0.0, barColor.dark(105));
        barGradient.setColorAt(1.0, barColor.light(120));
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
                painter->setOpacity(option->state & QStyle::State_Active ? opacity : 0.7 * opacity);
#endif
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
                                QIcon::Mode iconMode = QIcon::Normal;
                                if (option->activeSubControls & sc) {
                                    iconMode = QIcon::Active;
                                }
                                opt.palette.setColor(QPalette::Text, textColor);
                                icon = style->standardIcon((QStyle::StandardPixmap)(QStyle::SP_TitleBarMenuButton + i), &opt, widget);
                                icon.paint(painter, rect, Qt::AlignCenter, iconMode);
			}
		}
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
                painter->setOpacity(opacity);
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
		if (qobject_cast<const QMdiSubWindow *>(widget)) {
			QFont font = painter->font();
			font.setBold(true);
			labelRect = option->rect.adjusted(option->fontMetrics.height() + 10, -1, -2, -3);
		//	font.setPointSizeF(10);
			painter->setFont(font);
		} else
#endif
                {
			labelRect = style->subControlRect(QStyle::CC_TitleBar, option, QStyle::SC_TitleBarLabel, widget);
		}
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
                painter->setOpacity(opacity * 0.1);
		painter->setPen(Qt::black);
                style->drawItemText(painter, labelRect.adjusted(1, 1, 1, 1), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->palette, true, option->text, QPalette::NoRole);
		//painter->drawText(labelRect.adjusted(1, 1, 1, 1), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->text);
		painter->setOpacity(option->state & QStyle::State_Active ? opacity : 0.7 * opacity);
#endif
		painter->setPen(textColor);
                style->drawItemText(painter, labelRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->palette, true, option->text, QPalette::NoRole);
                //painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, option->text);
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
#include <QtGui/QApplication>
#include <QtGui/QTabletEvent>
#include <QtGui/QMouseEvent>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
#include <QtGui/QWidgetAction>
#endif


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

extern void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget);

void paintMenuBarItem(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionMenuItem opt = *option;
    if (option->state & QStyle::State_Selected || option->state & QStyle::State_MouseOver) {
        QStyleOptionButton button;

        button.QStyleOption::operator=(*option);
        button.features = QStyleOptionButton::None;
        button.rect.adjust(-1, -1, 1, 1);
        button.state |= QStyle::State_MouseOver;
        // call without widget to get QPalette::Button background
        paintCommandButtonPanel(painter, &button, 0);
    } else {
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
    }
    opt.state &= ~QStyle::State_Sunken;
    ((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_MenuBarItem, &opt, painter, widget);
}


/*-----------------------------------------------------------------------*/

enum MenuMode {
    ButtonMenu,
    ItemViewMenu,
};

enum CheckColumnMode {
    UseIconColumn,
    ShowCheckColumn,
    NoCheckColumn
};

enum IconColumnMode {
    ForceIconColumn,
    ShowIconColumn,
    HideIconColumn
};

enum SeparatorMode {
    TextSeparator,
    IconSeparator,
    ItemSeparator
};

enum SelectionMode {
    TextSelection,
    IconSelection,
    ItemSelection
};


/*-----------------------------------------------------------------------*/

QSize sizeFromContentsMenuItem(const QStyleOptionMenuItem *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int menuItemSize, int textLineHeight)
{
    if (option->menuItemType == QStyleOptionMenuItem::Separator) {
        if (option->text.isEmpty()) {
            return QSize(4, 4);
        }
        // ### width?
        return QSize(4, QFontMetrics(option->font).height() + 8);
    }

    int w = contentsSize.width(), h = contentsSize.height();

    // always make room for icon column
    const int iconMargin = 4;
    int iconWidth;
    if (option->maxIconWidth) {
        iconWidth = option->maxIconWidth - 4 + 2 * iconMargin;
    } else {
        iconWidth = style->pixelMetric(QStyle::PM_SmallIconSize, option, widget) + 2 * iconMargin;
    }
    w += iconWidth;

    // add size for check column
    const int checkMargin = 4;
    if (option->checkType != QStyleOptionMenuItem::NotCheckable) {
        w += style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget) + checkMargin;
    }

    // submenu arrow and spacing
    w += option->fontMetrics.height() + 4;
    if (option->menuItemType == QStyleOptionMenuItem::SubMenu || option->text.indexOf(QChar('\t', 0)) >= 0) {
        w += option->fontMetrics.height();
    }

    if (h < textLineHeight) {
        h = textLineHeight;
    }
    h += menuItemSize;

    return QSize(w, qMax(h, qApp->globalStrut().height()));
}


/*-----------------------------------------------------------------------*/

inline QRect remainingHorizontalVisualRect(const QRect &rect, const QStyleOption *option, int width)
{
    return rect.adjusted(option->direction == Qt::LeftToRight ? width : 0, 0, option->direction != Qt::LeftToRight ? -width : 0, 0);
}


inline QRect horizontalVisualRect(const QRect &rect, const QStyleOption *option, int width)
{
    QRect res = rect;

    if (option->direction != Qt::LeftToRight) {
        res.setLeft(rect.left() + rect.width() - width);
    } else {
        res.setWidth(width);
    }
    return res;
}


inline QRect rightHorizontalVisualRect(const QRect &rect, const QStyleOption *option, int width)
{
    QRect res = rect;

    if (option->direction != Qt::LeftToRight) {
        res.setWidth(width);
    } else {
        res.setLeft(rect.left() + rect.width() - width);
    }
    return res;
}


/*-----------------------------------------------------------------------*/

void paintMenuItem(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style)
{
    // configuration
    const MenuMode menuMode = ButtonMenu;
    const CheckColumnMode checkColumnMode = option->maxIconWidth ? (/*option->menuHasCheckableItems ? UseIconColumn :*/ NoCheckColumn) : NoCheckColumn;
    const bool showUncheckedIndicator = true;
    const bool showShortcut = true;
    const IconColumnMode iconColumnMode = checkColumnMode == UseIconColumn && option->menuHasCheckableItems ? ForceIconColumn : option->maxIconWidth ? ShowIconColumn : ShowIconColumn;
    const SeparatorMode separatorMode = TextSeparator;
    const SelectionMode selectionMode = ItemSelection;

    // layout
    // ### make margins configurable
    const int checkMargin = 4;
    const int iconMargin = 4;
    const int checkSize = option->menuHasCheckableItems ? qMax(style->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, option, widget), style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget)) : 0;
    const int iconSize = style->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
    const int checkColumnWidth = checkSize + 2 * checkMargin;
    // FIXME qMax(checkSize, iconSize) for useIconColum
    const int iconColumnWidth = iconColumnMode == HideIconColumn ? 0 : option->maxIconWidth ? option->maxIconWidth - 4 + 2 * iconMargin : iconSize + 2 * iconMargin;
    // Qt 4.x has a bug where the option->rect is one pixel too wide
    const QRect itemRect = option->rect.adjusted(0, 0, -1, 0);
    QRect iconRect = horizontalVisualRect(itemRect, option, iconColumnWidth);
    QRect textRect = remainingHorizontalVisualRect(itemRect, option, iconColumnWidth);

    // background color
    QPalette::ColorRole menuBackgroundRole;
    QPalette::ColorRole menuForegroundRole;
    QBrush menuBackground;
    QBrush iconBackground;
    switch (menuMode) {
        case ButtonMenu:
            menuBackgroundRole = QPalette::Window;
            menuForegroundRole = QPalette::WindowText;
            iconBackground = QColor(0, 0, 0, 10);
            break;
        case ItemViewMenu:
            menuBackgroundRole = QPalette::Base;
            menuForegroundRole = QPalette::Text;
            iconBackground = option->palette.brush(QPalette::Active, QPalette::Window);
            break;
    }

    // background
    menuBackground = option->palette.brush(QPalette::Active, menuBackgroundRole);
    painter->fillRect(textRect, menuBackground);
    if (!iconRect.isEmpty()) {
        if (!iconBackground.isOpaque()) {
            painter->fillRect(iconRect, menuBackground);
        }
    }
    painter->fillRect(iconRect, iconBackground);
//    painter->fillRect(iconRect.adjusted(2, 2, -2, -2), QColor(0, 255, 0, 120));

    // separator
    if (option->menuItemType == QStyleOptionMenuItem::Separator) {
        QRect separatorRect;
        switch (separatorMode) {
            case IconSeparator:
                separatorRect = iconRect;
                break;
            case TextSeparator:
                separatorRect = textRect;
                break;
            case ItemSeparator:
                separatorRect = itemRect;
                break;
        }
        // ### make separator margins configurable
        separatorRect.adjust(0, 1, 0, -1);
        if (option->text.isEmpty()) {
            paintThinFrame(painter, separatorRect, option->palette, 60, -20);
        } else {
            const int textFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine;
            QColor bgcolor = menuBackground.color();
            paintThinFrame(painter, separatorRect, option->palette, -10, -20);
            paintThinFrame(painter, separatorRect.adjusted(1, 1, -1, -1), option->palette, -30, 80, menuBackgroundRole);
            QLinearGradient gradient(separatorRect.topLeft(), separatorRect.bottomLeft());
            gradient.setColorAt(0.0, shaded_color(bgcolor, 90));
            gradient.setColorAt(0.2, shaded_color(bgcolor, 60));
            gradient.setColorAt(0.5, shaded_color(bgcolor, 0));
            gradient.setColorAt(0.51, shaded_color(bgcolor, -10));
            gradient.setColorAt(1.0, shaded_color(bgcolor, -20));
            painter->fillRect(separatorRect.adjusted(1, 1, -1, -1), gradient);
            // ### margins
            separatorRect = remainingHorizontalVisualRect(separatorRect, option, 8);
            style->drawItemText(painter, separatorRect, textFlags, option->palette, true, option->text, menuForegroundRole);
        }
        return;
    }

    // selection background
    painter->save();
    if (option->state & QStyle::State_Selected) {
        QRect selectionRect;
        switch (selectionMode) {
            case IconSelection:
                selectionRect = iconRect;
                break;
            case TextSelection:
                selectionRect = textRect;
                break;
            case ItemSelection:
                selectionRect = itemRect;
                break;
        }
        switch (menuMode) {
            case ButtonMenu: {
                QStyleOptionButton button;
                button.QStyleOption::operator=(*option);
                button.features = QStyleOptionButton::None;
                button.state |= QStyle::State_MouseOver;
                button.rect = selectionRect.adjusted(-1, -1, 1, 1);
                paintCommandButtonPanel(painter, &button, 0);
                menuForegroundRole = QPalette::ButtonText;
                break;
            }
            case ItemViewMenu: {
                QColor color = option->palette.color(QPalette::Active, QPalette::Highlight);
                color.setAlpha(option->state & QStyle::State_Enabled ? 180 : 40);
                painter->fillRect(selectionRect, color);
                if (option->state & QStyle::State_Enabled) {
                    paintThinFrame(painter, selectionRect, option->palette, -20, -20);
                }
                menuForegroundRole = QPalette::HighlightedText;
                break;
            }
        }
        painter->setPen(option->palette.color(QPalette::Active, menuForegroundRole));
    }

    // arrow
    if (option->menuItemType == QStyleOptionMenuItem::SubMenu) {
        const int arrowWidth = option->fontMetrics.height();
        const QRect arrowRect = rightHorizontalVisualRect(textRect, option, arrowWidth);
        QStyleOptionMenuItem opt = *option;
        opt.rect = arrowRect;
        QFont font = painter->font();
        font.setPointSizeF(font.pointSizeF() / 1.19);
        opt.fontMetrics = QFontMetrics(font);
        opt.state &= QStyle::State_Enabled;
        opt.palette.setColor(QPalette::ButtonText, option->palette.color(option->state & QStyle::State_Enabled ? QPalette::Active : QPalette::Disabled, menuForegroundRole));
        style->drawPrimitive((option->direction == Qt::RightToLeft) ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowRight, &opt, painter, widget);
    }

    // check
    if (option->checkType != QStyleOptionMenuItem::NotCheckable) {
        QRect checkRect;
        switch (checkColumnMode)
        {
            case UseIconColumn:
                checkRect = iconRect;
                if (option->checked) {
                    // when using the icon colum, we do not show an icon for checked items
                    iconRect = QRect();
                }
                break;
            case NoCheckColumn:
            case ShowCheckColumn:
                checkRect = horizontalVisualRect(textRect, option, checkColumnWidth);
                textRect = remainingHorizontalVisualRect(textRect, option, checkColumnWidth - checkMargin);
                break;
        }
//        painter->fillRect(checkRect.adjusted(2, 2, -2, -2), QColor(0, 0, 255, 120));
        if (option->checked || option->state & QStyle::State_Selected || (showUncheckedIndicator && option->checkType == QStyleOptionMenuItem::NonExclusive)) {
            QStyleOptionMenuItem opt = *option;
            opt.rect = checkRect;
            style->drawPrimitive(QStyle::PE_IndicatorMenuCheckMark, &opt, painter, widget);
        }
    } else if (checkColumnMode == ShowCheckColumn) {
        textRect = remainingHorizontalVisualRect(textRect, option, checkColumnWidth - checkMargin);
    }
//    painter->fillRect(textRect.adjusted(2, 2, -2, -2), QColor(255, 0, 0, 120));

    // text
    if (!option->text.isEmpty()) {
        const int shortcutPos = option->text.indexOf(QChar('\t', 0));
        int textFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
        if (!style->styleHint(QStyle::SH_UnderlineShortcut, option, widget)) {
            textFlags |= Qt::TextHideMnemonic;
        }
        if (showShortcut && shortcutPos >= 0) {
            const int shortcutWidth = option->tabWidth + option->fontMetrics.height() - 2;
            const QRect shortcutRect = rightHorizontalVisualRect(textRect, option, shortcutWidth);
//            painter->fillRect(shortcutRect, QColor(255, 220, 0, 120));
            style->drawItemText(painter, shortcutRect, textFlags, option->palette, option->state & QStyle::State_Enabled, option->text.mid(shortcutPos + 1), menuForegroundRole);
        }
        QFont font = option->font;
        if (option->menuItemType == QStyleOptionMenuItem::DefaultItem) {
            font.setBold(true);
        }
        painter->setFont(font);
        // ### textMargin
        textRect = remainingHorizontalVisualRect(textRect, option, 4);
        style->drawItemText(painter, textRect, textFlags, option->palette, option->state & QStyle::State_Enabled, option->text.left(shortcutPos), menuForegroundRole);
    }

    // icon
    if (iconColumnMode != HideIconColumn && !option->icon.isNull() && !iconRect.isEmpty()) {
        QIcon::Mode mode;
        if (option->state & QStyle::State_Enabled) {
            if (option->state & QStyle::State_Selected) {
                mode = QIcon::Active;
            } else {
                mode = QIcon::Normal;
            }
        } else {
            mode = QIcon::Disabled;
        }
        iconRect = QRect((iconRect.left() + iconRect.right() + 2 - iconSize) >> 1, (iconRect.top() + iconRect.bottom() + 2 - iconSize) >> 1, iconSize, iconSize);
        option->icon.paint(painter, iconRect, Qt::AlignCenter, mode, QIcon::Off);
    }
    painter->restore();
}


/*-----------------------------------------------------------------------*/

void paintMenuTitle(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style)
{
    const QPalette::ColorRole bgrole = QPalette::Window;
    QColor bgcolor = option->palette.color(bgrole);
    QStyleOptionToolButton opt = *option;
    opt.state &= ~(QStyle::State_Sunken | QStyle::State_On | QStyle::State_Selected);
    // Qt 4.x has a bug where the option->rect is one pixel too wide
    opt.rect.adjust(0, 0, -1, 0);
    opt.palette.setColor(QPalette::ButtonText, option->palette.color(QPalette::WindowText));
    paintThinFrame(painter, opt.rect, option->palette, -10, -20);
    paintThinFrame(painter, opt.rect.adjusted(1, 1, -1, -1), opt.palette, -30, 80, bgrole);
    QLinearGradient gradient(opt.rect.topLeft(), opt.rect.bottomLeft());
    gradient.setColorAt(0.0, shaded_color(bgcolor, 90));
    gradient.setColorAt(0.2, shaded_color(bgcolor, 60));
    gradient.setColorAt(0.5, shaded_color(bgcolor, 0));
    gradient.setColorAt(0.51, shaded_color(bgcolor, -10));
    gradient.setColorAt(1.0, shaded_color(bgcolor, -20));
    painter->fillRect(opt.rect.adjusted(1, 1, -1, -1), gradient);
    ((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_ToolButton, &opt, painter, widget);
}


/*-----------------------------------------------------------------------*/

bool SkulptureStyle::Private::menuEventFilter(QMenu *menu, QEvent *event)
{
    QHash<QMenu *, MenuInfo>::iterator i = menuHash.begin();
    while (i != menuHash.end()) {
        if (!i->menu) {
            i = menuHash.erase(i);
        } else {
            ++i;
        }
    }
    i = menuHash.find(menu);
    MenuInfo *menuInfo = i != menuHash.end() ? &(*i) : 0;

    if (event->type() == QEvent::Hide) {
        if (menuInfo) {
            menuHash.erase(i);
        }
        menuInfo = 0;
    } else {
        QPoint eventPos;
        bool moveEvent = false;
        if (event->type() == QEvent::TabletMove) {
            QTabletEvent *tabletEvent = (QTabletEvent *) event;
            eventPos = tabletEvent->pos();
            moveEvent = true;
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = (QMouseEvent *) event;
            eventPos = mouseEvent->pos();
            moveEvent = true;
        }

        QAction *menuAction = 0;
        QAction *action = 0;
        if (moveEvent && menu->rect().contains(eventPos)) {
            action = menu->actionAt(eventPos);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
            if (QWidgetAction * widgetAction = qobject_cast<QWidgetAction *>(action)) {
                if (widgetAction->defaultWidget()) {
                    action = 0;
                }
            }
#endif
            if (action && action->menu()) {
                menuAction = action;
            }
        }

        if (menuAction && !menuInfo) {
            MenuInfo info;
            info.menu = menu;
            info.delayTimer = 0;
            info.lastPos = eventPos;
            info.eventCount = 0;
            i = menuHash.insert(menu, info);
            menuInfo = &(*i);
        }

        if (menuInfo) {
            if (event->type() == QEvent::Enter) {
                menuInfo->lastSubMenuAction = 0;
                menuInfo->eventCount = 0;
            } else if (event->type() == QEvent::Leave) {
                menuInfo->lastSubMenuAction = 0;
                menuInfo->eventCount = 0;
            } else if (moveEvent) {
                if (action != menuAction) {
                    menuInfo->lastSubMenuAction = 0;
                    menuInfo->eventCount = 0;
                }
                if (menu->rect().contains(eventPos)) {
                    if (menuAction) {
                        QAction *last = menuInfo->lastSubMenuAction;
                        menuInfo->lastSubMenuAction = menuAction;
                        if (last && last == menuAction) {
                            if (event->type() == QEvent::MouseMove) {
                                QMouseEvent *mouseEvent = (QMouseEvent *) event;
                                ++menuInfo->eventCount;
                                if (menuInfo->eventCount > 2 && mouseEvent->buttons() == Qt::NoButton) {
                                    event->accept();
                                    return true;
                                }
                            }
                        } else {
                            menuInfo->eventCount = 0;
                        }
                    }
                } else {
                    menuInfo->lastSubMenuAction = 0;
                    menuInfo->eventCount = 0;
                }
            }
        }
    }
    return false;
}


/*
 * skulpture_misc.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QPainter>
#include <QtGui/QTableView>


/*-----------------------------------------------------------------------*/

extern void paintPanelButtonTool(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style);

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#if 0
void paintPanelPlacesViewItem(QPainter *painter, const QStyleOptionViewItemV4 *option, const QWidget *widget, const QStyle *style)
{
    QStyleOption opt = *option;
    if (opt.state & QStyle::State_Selected) {
        opt.state |= QStyle::State_On;
    } else if (!(opt.state & QStyle::State_MouseOver)) {
        // draw nothing
        return;
    }
    paintPanelButtonTool(painter, &opt, 0, style);
}
#endif

void paintPanelItemViewItem(QPainter *painter, const QStyleOptionViewItemV4 *option, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);
#if 0
        if (widget && widget->inherits("KFilePlacesView")) {
            paintPanelPlacesViewItem(painter, option, widget, style);
            return;
        }
#endif
	QColor color = option->palette.color(QPalette::Highlight);
	const bool mouse = option->state & QStyle::State_MouseOver && option->state & QStyle::State_Enabled;
        const QTableView *table = qobject_cast<const QTableView *>(widget);
        const bool largePanel = option->rect.height() > 7 + option->fontMetrics.height() && !table;
        QRect panelRect;

        if (largePanel) {
            if (option->version >= 4) {
                switch (option->viewItemPosition) {
                    case QStyleOptionViewItemV4::Beginning:
                        panelRect = option->rect.adjusted(1, 1, 0, 0);
                        break;
                    case QStyleOptionViewItemV4::End:
                        panelRect = option->rect.adjusted(0, 1, -1, 0);
                        break;
                    case QStyleOptionViewItemV4::Middle:
                        panelRect = option->rect.adjusted(0, 1, 0, 0);
                        break;
                    case QStyleOptionViewItemV4::Invalid:
                    case QStyleOptionViewItemV4::OnlyOne:
                        panelRect = option->rect.adjusted(1, 1, -1, 0);
                        break;
                }
            } else {
                panelRect = option->rect.adjusted(1, 1, -1, 0);
            }
        } else {
            panelRect = option->rect;
        }

        if (option->version >= 2 && option->features & QStyleOptionViewItemV2::Alternate) {
		painter->fillRect(panelRect, option->palette.color(QPalette::AlternateBase));
	} else {
		painter->fillRect(panelRect, option->backgroundBrush);
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
//	QColor shine(255, 255, 255, panelRect.height() > 20 ? 25 : 10);
	QColor shadow(0, 0, 0, largePanel ? 50 : 20);
	painter->setPen(shadow);
//	painter->setPen(QPen(color.darker(panelRect.height() > 20 ? 150 : 120), 1));
	painter->fillRect(panelRect, color);
	if (table && table->showGrid()) {
		painter->restore();
		return;
	}
	if (option->version >= 4) {
		switch (option->viewItemPosition) {
			case QStyleOptionViewItemV4::Beginning:
				painter->drawLine(panelRect.topLeft() + QPoint(0, 1), panelRect.bottomLeft() - QPoint(0, 1));
				painter->drawLine(panelRect.topLeft(), panelRect.topRight());
				painter->drawLine(panelRect.bottomLeft(), panelRect.bottomRight());
				break;
			case QStyleOptionViewItemV4::End:
				painter->drawLine(panelRect.topRight() + QPoint(0, 1), panelRect.bottomRight() - QPoint(0, 1));
				painter->drawLine(panelRect.topLeft(), panelRect.topRight());
				painter->drawLine(panelRect.bottomLeft(), panelRect.bottomRight());
				break;
			case QStyleOptionViewItemV4::Middle:
				painter->drawLine(panelRect.topLeft(), panelRect.topRight());
				painter->drawLine(panelRect.bottomLeft(), panelRect.bottomRight());
				break;
			case QStyleOptionViewItemV4::Invalid:
			case QStyleOptionViewItemV4::OnlyOne:
				painter->drawRect(panelRect.adjusted(0, 0, -1, -1));
				break;
		}
	} else {
		painter->drawRect(panelRect.adjusted(0, 0, -1, -1));
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
            if (option->direction == Qt::LeftToRight) {
                painter->fillRect(QRect(center.x() + 1, center.y(), option->rect.right() - center.x(), 1), lineColor);
            } else {
                painter->fillRect(QRect(option->rect.left(), center.y(), center.x() - option->rect.left(), 1), lineColor);
            }
            if (!(option->state & QStyle::State_Sibling)) {
                lineColor.setAlpha(25);
                painter->fillRect(QRect(center.x(), center.y(), 1, 1), lineColor);
            }
        }
    }
    if (option->state & QStyle::State_Children && !(option->state & QStyle::State_Open)) {
        QStyleOption opt = *option;
        static const int d = 9;
        opt.rect = QRect(center.x() - d / 2, center.y() - d / 2, d, d);
        paintCachedIndicatorBranchChildren(painter, &opt);
    }
}


/*-----------------------------------------------------------------------*/

void paintQ3ListView(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget,
                     const QStyle *style)
{
    Q_UNUSED(widget); Q_UNUSED(style);

    if (option->subControls & QStyle::SC_Q3ListView) {
        painter->fillRect(option->rect, option->viewportPalette.brush(option->viewportBGRole));
    }
    if (option->subControls & QStyle::SC_Q3ListViewBranch) {
        QStyleOption opt = *((QStyleOption *) option);
        int y = option->rect.y();
        for (int i = 1; i < option->items.size(); ++i) {
            QStyleOptionQ3ListViewItem item = option->items.at(i);
            if (y + item.totalHeight > 0 && y < option->rect.height()) {
                opt.state = QStyle::State_Item;
                if (i + 1 < option->items.size()) {
                    opt.state |= QStyle::State_Sibling;
                }
                if (item.features & QStyleOptionQ3ListViewItem::Expandable
                    || (item.childCount > 0 && item.height > 0)) {
                    opt.state |= QStyle::State_Children | (item.state & QStyle::State_Open);
                }
                opt.rect = QRect(option->rect.left(), y, option->rect.width(), item.height);
                paintIndicatorBranch(painter, &opt);

                if (opt.state & QStyle::State_Sibling && item.height < item.totalHeight) {
                    opt.state = QStyle::State_Sibling;
                    opt.rect = QRect(option->rect.left(), y + item.height,
                                     option->rect.width(), item.totalHeight - item.height);
                    paintIndicatorBranch(painter, &opt);
                }
            }
            y += item.totalHeight;
        }
    }
}


/*-----------------------------------------------------------------------*/

void paintSizeGrip(QPainter *painter, const QStyleOption *option)
{
    Qt::Corner corner = Qt::BottomRightCorner;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
    if (option->type == QStyleOption::SO_SizeGrip) {
        const QStyleOptionSizeGrip *sizegrip = static_cast<const QStyleOptionSizeGrip *>(option);
        corner = sizegrip->corner;
    }
#endif
	QRect r;

	switch (corner) {
		case Qt::TopLeftCorner:		r = option->rect.adjusted(0, 0, 2, 2);	break;
		case Qt::TopRightCorner:		r = option->rect.adjusted(-2, 0, 0, 2);	break;
		case Qt::BottomLeftCorner:	r = option->rect.adjusted(0, -2, 2, 0);	break;
		case Qt::BottomRightCorner:	r = option->rect.adjusted(-2, -2, 0, 0);	break;
	}
	paintThinFrame(painter, r, option->palette, 60, -20);
	paintThinFrame(painter, r.adjusted(1, 1, -1, -1), option->palette, -20, 60);
	switch (corner) {
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

extern void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget);

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBoxV2 *option)
#else
void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBox *option)
#endif
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
		button.rect.adjust(-1, -1, 1, 1);
		// ### needs QPalette::Window ?
		paintCommandButtonPanel(painter, &button, 0);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
        } else if (option->version >= 2 && option->selectedPosition == QStyleOptionToolBoxV2::PreviousIsSelected) {
		r.setHeight(2);
		paintThinFrame(painter, r, option->palette, 60, -20);
#endif
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

#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
void paintToolBoxTabLabel(QPainter *painter, const QStyleOptionToolBox *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionToolBoxV2 opt;

    if (option->version >= 2) {
        opt = *((const QStyleOptionToolBoxV2 *) option);
    } else {
        opt = *option;
    }
    if ((option->state & QStyle::State_Selected) || !(option->state & (QStyle::State_Sunken | QStyle::State_MouseOver))) {
        opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
    }
    ((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_ToolBoxTabLabel, &opt, painter, widget);
}
#endif

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
		QColor color = option->palette.color(QPalette::Highlight);
		color.setAlphaF(0.2 * color.alphaF());
		painter->fillRect(option->rect, color);
		color = option->palette.color(QPalette::Highlight);
		color.setAlphaF(0.8 * color.alphaF());
		painter->setPen(QPen(color /*, 1.0, Qt::DotLine*/));
		painter->drawRect(option->rect.adjusted(0, 0, -1, -1));
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
#include "sk_effects.h"

#if 0
#define FG_ROLE_PROGRESS QPalette::WindowText
#define BG_ROLE_PROGRESS QPalette::Window
#elif 1
#define FG_ROLE_PROGRESS QPalette::Text
#define BG_ROLE_PROGRESS QPalette::Base
#else
#define FG_ROLE_PROGRESS QPalette::QPalette::Highlight
#define BG_ROLE_PROGRESS QPalette::Base
#endif
#define FG_ROLE_CHUNK QPalette::HighlightedText
#define BG_ROLE_CHUNK QPalette::Highlight

/*-----------------------------------------------------------------------*/

static void paintRotatedText(QPainter *painter, const QRect &rect, int alignment, const QString &text, int angle)
{
    QMatrix matrix;
    QPointF center = QRectF(rect).center();
    matrix.translate(center.x(), center.y());
    matrix.rotate(-angle);
    matrix.translate(-center.x(), -center.y());
    QRect r = matrix.mapRect(rect);
    QRect textRect = painter->fontMetrics().boundingRect(r, alignment, text);
    QPixmap pixmap(textRect.size() + QSize(4, 4));
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setPen(painter->pen());
    p.setFont(painter->font());
    p.drawText(QRect(2, 2, pixmap.width() - 2, pixmap.height() - 2), alignment, text);
    painter->save();
    painter->setMatrix(matrix, true);
    painter->drawPixmap(r.x() + ((r.width() - pixmap.width()) >> 1), r.y() + ((r.height() - pixmap.height()) >> 1), pixmap);
    painter->restore();
}


/*-----------------------------------------------------------------------*/

static bool isPasswordStrengthIndicator(const QWidget *widget)
{
    return widget && widget->parentWidget() && widget->parentWidget()->parentWidget()
     && widget->parentWidget()->parentWidget()->inherits("KNewPasswordDialog");
}


static bool isDiskSpaceIndicator(const QWidget *widget)
{
    return false && widget && widget->inherits("StatusBarSpaceInfo");
}


/*-----------------------------------------------------------------------*/

static bool progressBarContentsCentered(const QStyleOptionProgressBarV2 *option, const QWidget *widget)
{
    const bool vertical = option->version >= 2 && option->orientation == Qt::Vertical;
    if (vertical) {
        return false;
    }
    if (isPasswordStrengthIndicator(widget) || isDiskSpaceIndicator(widget)) {
        return false;
    }
    return true;
}


static QRect progressBarContentsRect(const QStyleOptionProgressBarV2 *option, bool contentsCentered)
{
    // configuration options
    const int border = 2;

    QRect contentsRect = option->rect.adjusted(border, border, -border, -border);
    if (option->minimum < option->maximum) {
        if (option->progress > option->minimum) {
            if (option->progress < option->maximum) {
                // progress
                qreal progress = qreal(option->progress - option->minimum) / (option->maximum - option->minimum);
                if (option->version >= 2 && option->orientation == Qt::Vertical) {
                    if (contentsCentered) {
                        int adjustment = (contentsRect.height() / 2) * (1.0 - progress);
                        contentsRect.adjust(0, adjustment, 0, -adjustment);
                    } else {
                        int contentsHeight = qMax(1, int(contentsRect.height() * progress + 0.5));
                        if (option->version >= 2 && option->invertedAppearance) {
                            contentsRect.setHeight(contentsHeight);
                        } else {
                            contentsRect.setTop(contentsRect.top() + contentsRect.height() - contentsHeight);
                        }
                    }
                } else {
                    if (contentsCentered) {
                        int adjustment = (contentsRect.width() / 2) * (1.0 - progress);
                        contentsRect.adjust(adjustment, 0, -adjustment, 0);
                    } else {
                        int contentsWidth = qMax(1, int(contentsRect.width() * progress + 0.5));
                        if ((option->version >= 2 && option->invertedAppearance) ^ (option->direction != Qt::LeftToRight)) {
                            contentsRect.setLeft(contentsRect.left() + contentsRect.width() - contentsWidth);
                        } else {
                            contentsRect.setWidth(contentsWidth);
                        }
                    }
                }
            } else {
                // finished
            }
        } else {
            // starting
            contentsRect = QRect();
        }
    } else if (option->minimum == option->maximum) {
        // busy indicator
    } else {
        // invalid values
        contentsRect = QRect();
    }
    return contentsRect;
}


/*-----------------------------------------------------------------------*/

void paintProgressBarGroove(QPainter *painter, const QStyleOptionProgressBar *option)
{
    painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.brush(BG_ROLE_PROGRESS));
}


void paintProgressBarLabel(QPainter *painter, const QStyleOptionProgressBarV2 *option, const QWidget *widget, const QStyle *style)
{
    if (!option->textVisible) {
        return;
    }
    const bool vertical = option->version >= 2 && option->orientation == Qt::Vertical;
    Qt::Alignment alignment = option->textAlignment;
    if (vertical) {
        if (!(alignment & (Qt::AlignTop | Qt::AlignBottom | Qt::AlignVCenter))) {
            alignment |= Qt::AlignVCenter;
        }
        alignment &= ~(Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignHCenter;
    } else {
        if (!(alignment & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter))) {
            alignment |= Qt::AlignHCenter;
        }
        alignment &= ~(Qt::AlignTop | Qt::AlignBottom);
        alignment |= Qt::AlignVCenter;
    }
    // FIXME currently forces centering
    if (true) {
        alignment &= ~(Qt::AlignLeft | Qt::AlignRight);
        alignment &= ~(Qt::AlignTop | Qt::AlignBottom);
        alignment |= Qt::AlignCenter;
    }
    QRect r = option->rect.adjusted(6, 0, -6, 0);
    QRect labelRect = option->fontMetrics.boundingRect(r, alignment, option->text);
    if (!labelRect.isEmpty()) {
        const bool contentsCentered = progressBarContentsCentered(option, widget);
        const QRect contentsRect = progressBarContentsRect(option, contentsCentered);
        if (contentsRect.intersects(labelRect)) {
            painter->save();
            if (vertical) {
                QMatrix mat;
                QPointF c = QRectF(r).center();
                mat.translate(c.x(), c.y());
                mat.rotate(option->bottomToTop ? -90 : 90);
                mat.translate(-c.x(), -c.y());
                r = mat.mapRect(r);
                painter->setMatrix(mat, true);
            }
#if 0
            QImage outlineBuffer(labelRect.size() + QSize(8, 8), QImage::Format_ARGB32_Premultiplied);
            outlineBuffer.fill(0);
            QPainter outlinePainter(&outlineBuffer);
            outlinePainter.setPen(option->palette.color(FG_ROLE_CHUNK).value() > 150 ? Qt::black : Qt::white);
            style->drawItemText(&outlinePainter, QRect(0, 0, outlineBuffer.width(), outlineBuffer.height()), Qt::AlignCenter, option->palette, true, option->text, QPalette::NoRole);
            outlinePainter.end();
            blurImage(outlineBuffer, 4);
            outlinePainter.begin(&outlineBuffer);
            style->drawItemText(&outlinePainter, QRect(0, 0, outlineBuffer.width(), outlineBuffer.height()), Qt::AlignCenter, option->palette, true, option->text, FG_ROLE_CHUNK);
            outlinePainter.end();
#endif
            if (vertical) {
                QMatrix mat;
                QPointF c = QRectF(r).center();
                mat.translate(c.x(), c.y());
                mat.rotate(option->bottomToTop ? 90 : -90);
                mat.translate(-c.x(), -c.y());
                painter->setClipRegion(mat.mapRect(contentsRect));
            } else {
                painter->setClipRegion(contentsRect);
            }
#if 0
            painter->drawImage(QRect(labelRect.x() - 4, labelRect.y() - 4, labelRect.width() + 8, labelRect.height() + 8), outlineBuffer);
#else
            style->drawItemText(painter, r, alignment, option->palette, true, option->text, FG_ROLE_CHUNK);
#endif
            painter->restore();
        }
        painter->save();
        QRegion region = option->rect;
        region -= contentsRect;
        painter->setClipRegion(region);
        if (vertical) {
            painter->setPen(option->palette.color(FG_ROLE_PROGRESS));
            paintRotatedText(painter, r, alignment, option->text, option->bottomToTop ? 90 : -90);
        } else {
            style->drawItemText(painter, r, alignment, option->palette, option->state & QStyle::State_Enabled, option->text, FG_ROLE_PROGRESS);
        }
        painter->restore();
    }
}


/*-----------------------------------------------------------------------*/

enum AnimationMode {
    NoAnimation,
    FloatAnimation,
    LiquidAnimation
};


static QColor progressBarFillColor(const QStyleOptionProgressBarV2 *option, const QWidget *widget)
{
    QColor fillColor = option->palette.color(BG_ROLE_CHUNK);
    if (isPasswordStrengthIndicator(widget)) {
        int p = option->minimum < option->maximum ? 100 * (option->progress - option->minimum) / (option->maximum - option->minimum) : 0;
        fillColor.setHsv(p * 85 / 100, 200, 240 - p);
    } else if (isDiskSpaceIndicator(widget)) {
        int p = option->minimum < option->maximum ? 100 * (option->progress - option->minimum) / (option->maximum - option->minimum) : 0;
        if (p < 75) p = 100; else p = (100 - p) * 4;
        fillColor.setHsv(p * 85 / 100, 200, 240 - p);
    }
    return fillColor;
}


void paintProgressBarContents(QPainter *painter, const QStyleOptionProgressBarV2 *option, const QWidget *widget, const QStyle *style)
{
    // configuration
    const bool busyIndicator = option->minimum == option->maximum;
    const bool vertical = option->version >= 2 && option->orientation == Qt::Vertical;
    const AnimationMode animationMode = busyIndicator ? FloatAnimation : vertical ? LiquidAnimation : FloatAnimation;
    const int chunkWidth = 4 * qMax(1, style->pixelMetric(QStyle::PM_ProgressBarChunkWidth, option, widget));
    const bool reverseAnimation = busyIndicator;
    const int animationSpeed = (busyIndicator ? 1000 : 1000) * (reverseAnimation ? -1 : 1);
    const int floatAlpha = busyIndicator ? 100 : 255;
    const bool contentsCentered = progressBarContentsCentered(option, widget);
    const QRect contentsRect = progressBarContentsRect(option, contentsCentered);
    const int cornerSmoothing = 0;
    const int centerSmoothing = busyIndicator ? 100 : 0;
    const int borderSmoothing = 0;
    const bool contentsFrame = false;
    const int gradientSharpness = 0;
    const int gradientRatio = 50;
    const int gradientAngle = 0;

    // contents
    if (!contentsRect.isEmpty()) {
        QColor fillColor = progressBarFillColor(option, widget);
        qreal rounding = -1;
        if (!busyIndicator && option->progress < option->maximum) {
            rounding = contentsCentered ? 0.6 : 0.5;
        }
        switch (animationMode) {
            case NoAnimation: {
                painter->fillRect(contentsRect, fillColor);
                break;
            }
            case FloatAnimation: {
                QColor floatColor;
                if (option->palette.color(FG_ROLE_CHUNK).value() > fillColor.value()) {
                    floatColor = fillColor.lighter(105);
                } else {
                    floatColor = fillColor.darker(105);
                }
                floatColor.setAlpha(floatAlpha);
                int m = QTime(0, 0).msecsTo(QTime::currentTime()) / (animationSpeed / chunkWidth);
                QPoint startPoint = contentsCentered ? contentsRect.topLeft() : contentsRect.center();
                startPoint += vertical ? QPoint(0, chunkWidth - 1 - m % chunkWidth) : QPoint(m % chunkWidth, 0);
                QLinearGradient fillGradient(startPoint, startPoint + (vertical ? QPoint(0, chunkWidth) : QPoint(chunkWidth, chunkWidth * sin(gradientAngle * M_PI / 180))));
                fillGradient.setSpread(QGradient::RepeatSpread);
                const qreal delta = gradientRatio * gradientSharpness * 0.000049999;
                fillGradient.setColorAt(0.0, fillColor);
                fillGradient.setColorAt(0.0 + delta, fillColor);
                fillGradient.setColorAt(gradientRatio / 100.0 - delta, floatColor);
                fillGradient.setColorAt(gradientRatio / 100.0 + delta, floatColor);
                fillGradient.setColorAt(gradientRatio / 50.0 - delta, fillColor);
                fillGradient.setColorAt(1.0, fillColor);
                if (contentsCentered) {
                    painter->save();
                    if (vertical) {
                        painter->setClipRect(QRect(contentsRect.x(), contentsRect.y() + (contentsRect.height() >> 1), contentsRect.width(), contentsRect.height() - (contentsRect.height() >> 1)));
                    } else {
                        painter->setClipRect(QRect(contentsRect.x(), contentsRect.y(), contentsRect.width() >> 1, contentsRect.height()));
                    }
                    painter->translate(QRectF(contentsRect).center());
                    painter->scale(vertical ? 1 : -1, vertical ? -1 : 1);
                    painter->translate(-QRectF(contentsRect).center());
                    painter->fillRect(contentsRect, fillGradient);
                    painter->restore();
                    painter->save();
                    if (vertical) {
                        painter->setClipRect(QRect(contentsRect.x(), contentsRect.y(), contentsRect.width(), contentsRect.height() >> 1));
                    } else {
                        painter->setClipRect(QRect(contentsRect.x() + (contentsRect.width() >> 1), contentsRect.y(), contentsRect.width() - (contentsRect.width() >> 1), contentsRect.height()));
                    }
                    painter->fillRect(contentsRect, fillGradient);
                    painter->restore();
                } else {
                    painter->fillRect(contentsRect, fillGradient);
                }
                if (contentsCentered && centerSmoothing > 0) {
                    QColor centerColor = fillColor;
                    const int contentsSize = vertical ? contentsRect.height() : contentsRect.width();
                    const int centerSize = qMin(3 * chunkWidth,  contentsSize >> 1);
                    const int delta = (contentsSize - centerSize) >> 1;
                    const QRect centerRect = vertical ? contentsRect.adjusted(0, delta, 0, -delta) : contentsRect.adjusted(delta, 0, -delta, 0);
                    QLinearGradient centerGradient(centerRect.topLeft(), vertical ? centerRect.bottomLeft() : centerRect.topRight());
                    centerGradient.setColorAt(0.0, Qt::transparent);
                    centerGradient.setColorAt(0.4, centerColor);
                    centerGradient.setColorAt(0.6, centerColor);
                    centerGradient.setColorAt(1.0, Qt::transparent);
                    painter->fillRect(centerRect, centerGradient);
                }
                break;
            }
            case LiquidAnimation: {
                painter->fillRect(contentsRect, fillColor);
                if (rounding >= 0) {
                    int m = QTime(0, 0).msecsTo(QTime::currentTime());
                    rounding = (sin(m / 100.0) + 1) / 2;
                }
            }
        }
        if (rounding >= 0) {
            QLinearGradient lineGradient(contentsRect.topLeft(), vertical ? contentsRect.topRight() : contentsRect.bottomLeft());
            QColor innerColor = blend_color(option->palette.color(BG_ROLE_PROGRESS), fillColor, rounding);
            QColor outerColor = blend_color(fillColor, option->palette.color(BG_ROLE_PROGRESS), rounding);
            lineGradient.setColorAt(0.0, outerColor);
            lineGradient.setColorAt(0.5, innerColor);
            lineGradient.setColorAt(1.0, outerColor);
            if (contentsCentered || (option->version >= 2 && option->invertedAppearance)) {
                if (vertical) {
                    painter->fillRect(QRect(contentsRect.x(), contentsRect.bottom(), contentsRect.width(), 1), lineGradient);
                } else {
                    painter->fillRect(QRect(contentsRect.left(), contentsRect.y(), 1, contentsRect.height()), lineGradient);
                }
            }
            if (contentsCentered || !(option->version >= 2 && option->invertedAppearance)) {
                if (vertical) {
                    painter->fillRect(QRect(contentsRect.x(), contentsRect.top(), contentsRect.width(), 1), lineGradient);
                } else {
                    painter->fillRect(QRect(contentsRect.right(), contentsRect.y(), 1, contentsRect.height()), lineGradient);
                }
            }
        }
        if (cornerSmoothing > 0) {
            painter->save();
            QColor color = option->palette.color(BG_ROLE_PROGRESS);
            color.setAlpha(cornerSmoothing);
            painter->setPen(color);
            painter->drawPoint(contentsRect.left(), contentsRect.top());
            painter->drawPoint(contentsRect.left(), contentsRect.bottom());
            painter->drawPoint(contentsRect.right(), contentsRect.top());
            painter->drawPoint(contentsRect.right(), contentsRect.bottom());
            painter->restore();
        }
        if (borderSmoothing > 0) {
            painter->save();
            QColor frameColor = option->palette.color(BG_ROLE_PROGRESS);
            frameColor.setAlpha(borderSmoothing);
            painter->setPen(frameColor);
            painter->drawRect(contentsRect.adjusted(0, 0, -1, -1));
            painter->restore();
        }
        if (contentsFrame) {
            paintThinFrame(painter, contentsRect, option->palette, -20, 60, BG_ROLE_CHUNK);
        }
    }

    // overlay gradient
    QLinearGradient glassyGradient(option->rect.topLeft(), vertical ? option->rect.topRight() : option->rect.bottomLeft());
    glassyGradient.setColorAt(0.0, QColor(255, 255, 255, 0));
    glassyGradient.setColorAt(0.47, QColor(0, 0, 0, 2));
    glassyGradient.setColorAt(0.475, QColor(0, 0, 0, 21));
    glassyGradient.setColorAt(1.0, QColor(255, 255, 255, 0));
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
	QColor color = option->palette.color(QPalette::Disabled, QPalette::Window);
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


void paintScrollAreaCorner(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle */*style*/)
{
	QStyleOption opt;
	opt = *option;
	opt.type = QStyleOption::SO_Default;
	if (qobject_cast<const QAbstractScrollArea *>(widget)) {
            // ### work around bug in Qt 4.5
            if (option->rect.y() + option->rect.height() > widget->rect().height()
             || option->rect.x() + option->rect.width() > widget->rect().width()) {
                return;
            }
		opt.type = QStyleOption::SO_Slider;
		opt.state &= ~QStyle::State_Enabled;
		if (widget->isEnabled()) {
			opt.state |= QStyle::State_Enabled;
		}
	}
	paintScrollArea(painter, &opt);
}


extern void paintSliderGroove(QPainter *painter, QRect &rect, const QStyleOptionSlider *option);

void paintScrollBarPage(QPainter *painter, const QStyleOptionSlider *option)
{
    const bool paintGroove = true;

    paintScrollArea(painter, option);
    if (paintGroove) {
        QRect rect = option->rect.adjusted(1, 1, -1, -1);
        paintSliderGroove(painter, rect, option);
    }
}


void paintScrollBarAddLine(QPainter *painter, const QStyleOptionSlider *option)
{
	paintScrollArea(painter, option);
//	paintThinFrame(painter, option->rect, option->palette, -40, 120);
	if (option->minimum != option->maximum) {
		QStyleOptionSlider opt = *option;
		opt.fontMetrics = QApplication::fontMetrics();
                opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
		paintScrollArrow(painter, &opt, option->orientation == Qt::Horizontal ? (option->direction == Qt::LeftToRight ? Qt::RightArrow : Qt::LeftArrow) : Qt::DownArrow, false);
	}
}


void paintScrollBarSubLine(QPainter *painter, const QStyleOptionSlider *option)
{
	paintScrollArea(painter, option);
//	paintThinFrame(painter, option->rect, option->palette, -40, 120);
	if (option->minimum != option->maximum) {
		QStyleOptionSlider opt = *option;
		opt.fontMetrics = QApplication::fontMetrics();
                opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
                paintScrollArrow(painter, &opt, option->orientation == Qt::Horizontal ? (option->direction == Qt::LeftToRight ? Qt::LeftArrow : Qt::RightArrow) : Qt::UpArrow, false);
	}
}


void paintScrollBarFirst(QPainter *painter, const QStyleOptionSlider *option)
{
    paintScrollBarSubLine(painter, option);
    if (option->minimum != option->maximum) {
        painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.color(QPalette::WindowText));
    }
}


void paintScrollBarLast(QPainter *painter, const QStyleOptionSlider *option)
{
    paintScrollBarAddLine(painter, option);
    if (option->minimum != option->maximum) {
        painter->fillRect(option->rect.adjusted(2, 2, -2, -2), option->palette.color(QPalette::WindowText));
    }
}


/*-----------------------------------------------------------------------*/

class ComplexControlLayout
{
    public:
        struct SubControlItem {
            QStyle::SubControl subControl;
            QStyle::ControlElement controlElement;
            char layoutSpec;
        };

    protected:
        ComplexControlLayout(const SubControlItem *controlItem, uint controlCount,
            const QStyleOptionComplex *opt, const QWidget *w = 0, const QStyle *s = 0)
            : subControlItem(controlItem), subControlCount(controlCount),
            option(opt), widget(w), style(s), layoutCount(0)
        {
            /* */
        }
        ~ComplexControlLayout() { }

    public:
        QStyle::SubControl hitTestComplexControl(const QPoint &position) const;
        QRect subControlRect(QStyle::SubControl subControl) const;
        void paintComplexControl(QPainter *painter) const;

    protected:
        struct LayoutItem {
            QStyle::SubControl subControl;
            QRect rect;
        };

    protected:
        void addLayoutItem(QStyle::SubControl subControl, const QRect &rect);

    protected:
        static const uint maxLayoutCount = 16;

    protected:
        const SubControlItem * const subControlItem;
        const uint subControlCount;
        const QStyleOptionComplex * const option;
        const QWidget * const widget;
        const QStyle * const style;
        uint layoutCount;
        LayoutItem layoutItem[maxLayoutCount];
};


/*-----------------------------------------------------------------------*/

void ComplexControlLayout::addLayoutItem(QStyle::SubControl subControl, const QRect &rect)
{
    if (layoutCount < maxLayoutCount) {
        layoutItem[layoutCount].subControl = subControl;
        layoutItem[layoutCount].rect = style->visualRect(option->direction, option->rect, rect);
        ++layoutCount;
    }
}


QStyle::SubControl ComplexControlLayout::hitTestComplexControl(const QPoint &position) const
{
    for (uint i = 0; i < subControlCount; ++i) {
        for (uint j = 0; j < layoutCount; ++j) {
            if (layoutItem[j].subControl == subControlItem[i].subControl) {
                if (layoutItem[j].rect.contains(position)) {
                    return layoutItem[j].subControl;
                }
            }
        }
    }
    return QStyle::SC_None;
}


QRect ComplexControlLayout::subControlRect(QStyle::SubControl subControl) const
{
    QRect rect;
    for (uint i = 0; i < layoutCount; ++i) {
        if (layoutItem[i].subControl == subControl) {
            rect |= layoutItem[i].rect;
        }
    }
    return rect;
}


void ComplexControlLayout::paintComplexControl(QPainter *painter) const
{
    for (int i = subControlCount; --i >= 0; ) {
        if (subControlItem[i].controlElement != QStyle::CE_CustomBase
        && option->subControls & subControlItem[i].subControl) {
            for (uint j = 0; j < layoutCount; ++j) {
                if (layoutItem[j].subControl == subControlItem[i].subControl) {
                    QStyleOptionSlider opt = *static_cast<const QStyleOptionSlider *>(ComplexControlLayout::option);
                    opt.rect = layoutItem[j].rect;
                    if (!(option->activeSubControls & subControlItem[i].subControl)) {
                        opt.state &= ~(QStyle::State_Sunken | QStyle::State_MouseOver);
                    }
                    style->drawControl(subControlItem[i].controlElement, &opt, painter, widget);
                }
            }
        }
    }
}


/*-----------------------------------------------------------------------*/

static const ComplexControlLayout::SubControlItem scrollBarSubControlItem[] = {
    /* hitTest in forward order, paint in reverse order */
    { QStyle::SC_ScrollBarSlider, QStyle::CE_ScrollBarSlider, '*' },
    { QStyle::SC_ScrollBarAddLine, QStyle::CE_ScrollBarAddLine, '>' },
    { QStyle::SC_ScrollBarSubLine, QStyle::CE_ScrollBarSubLine, '<' },
    { QStyle::SC_ScrollBarFirst, QStyle::CE_ScrollBarFirst, '[' },
    { QStyle::SC_ScrollBarLast, QStyle::CE_ScrollBarLast, ']' },
    { QStyle::SC_ScrollBarAddPage, QStyle::CE_ScrollBarAddPage, ')' },
    { QStyle::SC_ScrollBarSubPage, QStyle::CE_ScrollBarSubPage, '(' },
    { QStyle::SC_ScrollBarGroove, QStyle::CE_CustomBase, '#' }
};


class ScrollBarLayout : public ComplexControlLayout
{
    public:
        ScrollBarLayout(const QStyleOptionSlider *opt, const QWidget *w = 0, const QStyle *s = 0)
            : ComplexControlLayout(scrollBarSubControlItem, array_elements(scrollBarSubControlItem), opt, w, s)
        {
            /* */
        }
        ~ScrollBarLayout() { }

    public:
        void initLayout(const char *layoutSpec);
        void initLayout(ArrowPlacementMode placementMode);

    protected:
        void addLayoutItem(const char layoutSpec, int pos, int size);
};


/*-----------------------------------------------------------------------*/

void ScrollBarLayout::addLayoutItem(char c, int pos, int size)
{
    const QStyleOptionSlider *option = static_cast<const QStyleOptionSlider *>(ComplexControlLayout::option);

    if (size > 0) {
        for (uint i = 0; i < subControlCount; ++i) {
            if (subControlItem[i].layoutSpec == c) {
                QRect rect;
                if (option->orientation == Qt::Horizontal) {
                    rect = QRect(option->rect.left() + pos, option->rect.top(), size, option->rect.height());
                } else {
                    rect = QRect(option->rect.left(), option->rect.top() + pos, option->rect.width(), size);
                }
                ComplexControlLayout::addLayoutItem(subControlItem[i].subControl, rect);
                return;
            }
        }
    }
}


void ScrollBarLayout::initLayout(ArrowPlacementMode placementMode)
{
    static const char *layoutSpec[] = {
        "(*)", // NoArrowsMode
        "(<*>)", // SkulptureMode
        "<(*)>", // WindowsMode
        "<(*)<>", // KDEMode
        "(*)<>", // PlatinumMode
        "<>(*)" // NextMode
    };
    initLayout(layoutSpec[uint(placementMode)]);
}


void ScrollBarLayout::initLayout(const char *layoutSpec)
{
    const QStyleOptionSlider *option = static_cast<const QStyleOptionSlider *>(ComplexControlLayout::option);
    uint range = option->maximum - option->minimum;
    int pos = option->orientation == Qt::Horizontal ? option->rect.left() : option->rect.top();
    int maxSize = option->orientation == Qt::Horizontal ? option->rect.width() : option->rect.height();
    int endPos = pos + maxSize;
    int groovePos = pos, grooveSize = maxSize;
    int pagePos = pos, pageSize = maxSize;
    int buttonSize = style->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget);
    buttonSize = qMin(maxSize >> 1, buttonSize);

    if (qstrcmp(layoutSpec, "(*)")) {
        if (!qstrcmp(layoutSpec, "<(*)<>")) {
            if (maxSize < 4 * buttonSize) {
                layoutSpec = "<(*)>";
            }
        }
        if (maxSize < 3 * buttonSize) {
            layoutSpec = "(<*>)";
        }
    }

    if (layoutSpec && range != 0) {
        // layout items before slider
        const char *p = layoutSpec;
        char c;
        while ((c = *p)) {
            if (c == '*') {
                pagePos = pos;
                break;
            } else if (c == '(') {
                groovePos = pos;
            } else {
                addLayoutItem(c, pos, buttonSize);
                pos += buttonSize;
            }
            ++p;
        }

        // layout items after slider
        while (*p) {
            ++p;
        }
        --p;
        pos = endPos;
        while (p >= layoutSpec) {
            c = *p;
            if (c == '*') {
                pageSize = pos - pagePos;
                break;
            } else if (c == ')') {
                grooveSize = pos - groovePos;
            } else {
                pos -= buttonSize;
                addLayoutItem(c, pos, buttonSize);
            }
            --p;
        }
    }
    if (layoutCount > maxLayoutCount - 4) {
        layoutCount = maxLayoutCount - 4;
    }
    if (range != 0) {
        int sliderSize = (qint64(option->pageStep) * grooveSize) / (range + option->pageStep);
        int sliderMin = style->pixelMetric(QStyle::PM_ScrollBarSliderMin, option, widget);

        if (sliderMin > grooveSize >> 1) {
            sliderMin = grooveSize >> 1;
            if (sliderSize > sliderMin) {
                sliderSize = sliderMin;
            }
        }
        if (sliderSize < sliderMin || range > INT_MAX / 2) {
            sliderSize = sliderMin;
        }
        if (grooveSize != pageSize) {
            if (sliderSize > grooveSize - buttonSize) {
                sliderSize = grooveSize - buttonSize;
            }
        }
        int sliderPos = groovePos + style->sliderPositionFromValue(option->minimum, option->maximum, option->sliderPosition, grooveSize - sliderSize, option->upsideDown);

        addLayoutItem('(', pagePos, sliderPos - pagePos);
        addLayoutItem(')', sliderPos + sliderSize, (pagePos + pageSize) - (sliderPos + sliderSize));
        addLayoutItem('*', sliderPos, sliderSize);
    } else {
        addLayoutItem('*', groovePos, grooveSize);
    }
    addLayoutItem('#', groovePos, grooveSize);
}


/*-----------------------------------------------------------------------*/

extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole = QPalette::Window);
extern void paintSliderHandle(QPainter *painter, const QRect &rect, const QStyleOptionSlider *option);


void paintScrollBarSlider(QPainter *painter, const QStyleOptionSlider *option)
{
    if (option->minimum == option->maximum) {
        paintScrollArea(painter, option);
    } else {
        paintSliderHandle(painter, option->rect, option);
    }
}


/*-----------------------------------------------------------------------*/

void paintScrollBar(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode)
{
    // paint scrollbar
    ScrollBarLayout layout(option, widget, style);
    layout.initLayout(option->orientation == Qt::Horizontal ? horizontalArrowMode : verticalArrowMode);
    layout.paintComplexControl(painter);

    // get frame this scrollbar is in
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

    // paint shadow
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
        paintRecessedFrameShadow(painter, rect.adjusted(1, 1, -1, -1), RF_Small);
    }
}


QRect subControlRectScrollBar(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode)
{
    ScrollBarLayout layout(option, widget, style);
    layout.initLayout(option->orientation == Qt::Horizontal ? horizontalArrowMode : verticalArrowMode);
    return layout.subControlRect(subControl);
}


QStyle::SubControl hitTestComplexControlScrollBar(const QStyleOptionSlider *option, const QPoint &position, const QWidget *widget, const QStyle *style, ArrowPlacementMode horizontalArrowMode, ArrowPlacementMode verticalArrowMode)
{
    ScrollBarLayout layout(option, widget, style);
    layout.initLayout(option->orientation == Qt::Horizontal ? horizontalArrowMode : verticalArrowMode);
    return layout.hitTestComplexControl(position);
}


/*
 * skulpture_shadows.cpp
 *
 */

#include "skulpture_p.h"
#include <QtGui/QAbstractScrollArea>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
#include <QtGui/QMdiArea>
#endif
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtCore/QEvent>
#include <QtGui/QPainter>


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
    Q_FOREACH (QObject *child, shadows) {
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
    Q_FOREACH (QObject *child, shadows) {
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
    // this fixes shadows in frames that change frameStyle() after polish()
    if (QFrame *frame = qobject_cast<QFrame *>(parentWidget())) {
        if (frame->frameStyle() != (QFrame::StyledPanel | QFrame::Sunken)) {
            return;
        }
    }
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
	int c2 = (rf == RF_Small) ? 24 : 36;
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
	setObjectName(QLatin1String("WidgetShadow"));
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                        if (parent && !qobject_cast<QMdiArea *>(parent) && qobject_cast<QMdiArea *>(parent->parentWidget())) {
				parent = parent->parentWidget();
			}
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
			if (parent && !qobject_cast<QMdiArea *>(parent) && qobject_cast<QMdiArea *>(parent->parentWidget())) {
				parent = parent->parentWidget();
			}
#endif
			if (parent) {
				QRect geo(widget_->x() - 10, widget_->y() - 5, widget_->frameGeometry().width() + 20, widget_->frameGeometry().height() + 15);
				setGeometry(geo & parent->rect());
			}
			show();
		}
	}
}


/*
 * skulpture_shapes.cpp
 *
 */

#include "skulpture_p.h"
#include "sk_factory.h"


/*-----------------------------------------------------------------------*/
/**
 * ShapeFactory - create a QPainterPath from a description
 *
 * The shape description is a bytecode stream that allows simple arithmetic,
 * conditionals, and looping.
 *
 * Syntax is as follows:
 *	Description:	Instruction* End
 *	Instruction:	Close | Move X Y | Line X Y | Quad X1 Y1 X2 Y2 | Cubic X1 Y1 X2 Y2 X3 Y3
 *	Instruction:	Nop | SetVar X | Begin Instruction* End | While Condition Instruction | If Condition Instruction1 [ Else Instruction2 ]
 *	Condition:	Not Condition | And Condition1 Condition2 | Or Condition1 Condition2 | EQ X Y | NE X Y | LT X Y | GE X Y | GT X Y | LE X Y
 *	Expression:	Number | GetVar | Add X Y | Sub X Y | Mul X Y | Div X Y | Min X Y | Max X Y | Mix V X Y | Cond Condition X Y
 *
 * TODO
 *
 * Mod, MultiLine, DoWhile, Integer/Float, Switch, Exp, Sin, Cos, Tan, Atan, Atan2, Pi
 * Colors, Gradients, Functions (Call/Ret), Frames, Text
 *
 */


/*-----------------------------------------------------------------------*/

void ShapeFactory::executeCode(Code code)
{
	qreal v[6];

	switch (code) {
		case Move:
		case Line:
			v[0] = evalValue();
			v[1] = evalValue();
			if (code == Move) {
				path.moveTo(v[0], v[1]);
			} else {
				path.lineTo(v[0], v[1]);
			}
			break;
		case Close:
			path.closeSubpath();
			break;
		case Quad:
		case Cubic: {
			for (int n = 0; n < (code == Quad ? 4 : 6); ++n) {
				v[n] = evalValue();
			}
			if (code == Quad) {
				path.quadTo(v[0], v[1], v[2], v[3]);
			} else {
				path.cubicTo(v[0], v[1], v[2], v[3], v[4], v[5]);
			}
			break;
		}
		default:
			AbstractFactory::executeCode(code);
			break;
	}
}


void ShapeFactory::skipCode(Code code)
{
	switch (code) {
		case Move:
		case Line:
			skipValue();
			skipValue();
			break;
		case Close:
			break;
		case Quad:
		case Cubic: {
			for (int n = 0; n < (code == Quad ? 4 : 6); ++n) {
				skipValue();
			}
			break;
		}
		default:
			AbstractFactory::skipCode(code);
			break;
	}
}


/*-----------------------------------------------------------------------*/

QPainterPath ShapeFactory::createShape(ShapeFactory::Description description, qreal var[])
{
	ShapeFactory factory;

	factory.setDescription(description);
	for (int n = MinVar; n <= MaxVar; ++n) {
		factory.setVar(n, var[n]);
	}
	factory.create();
	for (int n = MinVar; n <= MaxVar; ++n) {
		var[n] = factory.getVar(n);
	}
	return factory.getPath();
}


QPainterPath ShapeFactory::createShape(ShapeFactory::Description description)
{
	ShapeFactory factory;

	factory.setDescription(description);
	factory.create();
	return factory.getPath();
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
#include <QtGui/QApplication>


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
 * Additionally, this class is responsible for blanking the mouse pointer:
 *
 *      * when the tablet pen leaves proximity
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
            Q_FOREACH (QWidget *child, children) {
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
    Q_FOREACH (QWidget *child, children) {
		if (child->isVisible() && hasShortcut(child)) {
			child->update();
		}
	}
}


bool ShortcutHandler::eventFilter(QObject *watched, QEvent *event)
{
    if (!watched->isWidgetType()) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
        switch (event->type()) {
            case QEvent::TabletEnterProximity:
                if (tabletCursorState != TabletCursor) {
                    if (tabletCursorState != DefaultCursor) {
                        QApplication::restoreOverrideCursor();
                    }
                    tabletCursorState = DefaultCursor;
                }
                break;
            case QEvent::TabletLeaveProximity:
                if (tabletCursorState != BlankCursor) {
                    if (tabletCursorState != DefaultCursor) {
                        QApplication::restoreOverrideCursor();
                    }
                    QApplication::setOverrideCursor(Qt::BlankCursor);
                    tabletCursorState = BlankCursor;
                }
                break;
            default:
                break;
        }
#endif
        return QObject::eventFilter(watched, event);
    }
	QWidget *widget = reinterpret_cast<QWidget *>(watched);

	switch (event->type()) {
                case QEvent::MouseMove:
                    if (tabletCursorState != DefaultCursor) {
                        QApplication::restoreOverrideCursor();
                        tabletCursorState = DefaultCursor;
                    }
                    break;
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
	: QObject(parent), tabletCursorState(DefaultCursor)
{
	init();
}


ShortcutHandler::~ShortcutHandler()
{
    if (tabletCursorState != DefaultCursor) {
        tabletCursorState = DefaultCursor;
        QApplication::restoreOverrideCursor();
    }
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
            color = color.darker(120);
#else
            color = color.dark(120);
#endif
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                    color = color.lighter(102);
#else
                    color = color.light(102);
#endif
                } else if (option->state & QStyle::State_MouseOver) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                    color = color.lighter(104);
#else
                    color = color.light(104);
#endif
		}
	} else {
		color = option->palette.color(QPalette::Window);
	}
	painter->fillRect(rect, color);

#if 1 // slider gradient
	if ((option->state & QStyle::State_Enabled) && !(option->state & QStyle::State_Sunken)) {
            QLinearGradient gradient(rect.topLeft(), option->orientation == Qt::Horizontal ? rect.bottomLeft() : rect.topRight());
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

void paintSlider(QPainter *painter, const QStyleOptionSlider *option, const QWidget *widget, const QStyle *style)
{
	// groove
	if (option->subControls & QStyle::SC_SliderGroove) {
	//	painter->fillRect(option->rect, option->palette.color(QPalette::Window).darker(105));
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
		QStyleOptionSlider slider = *option;
		slider.subControls = QStyle::SC_SliderTickmarks;
		// ### for now, just use common tickmarks
                QPalette palette = slider.palette;
                QColor col = palette.color(QPalette::WindowText);
                col.setAlpha(51);
                palette.setColor(QPalette::WindowText, col);
                slider.palette = palette;
                if (option->orientation == Qt::Horizontal) {
			slider.rect.adjust(-1, 0, -1, 0);
		} else {
			slider.rect.adjust(0, -1, 0, -1);
		}
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_Slider, &slider, painter, widget);
		slider.rect = option->rect;
                palette.setColor(QPalette::WindowText, QColor(255, 255, 255, 77));
		slider.palette = palette;
		((QCommonStyle *) style)->QCommonStyle::drawComplexControl(QStyle::CC_Slider, &slider, painter, widget);
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

QRect subControlRectSlider(const QStyleOptionSlider *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
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
#include <QtGui/QApplication>
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


void paintIndicatorSpinMinus(QPainter *painter, const QStyleOption *option)
{
    paintScrollArrow(painter, option, Qt::LeftArrow, true);
}


void paintIndicatorSpinPlus(QPainter *painter, const QStyleOption *option)
{
    paintScrollArrow(painter, option, Qt::RightArrow, true);
}


/*-----------------------------------------------------------------------*/

QRect subControlRectSpinBox(const QStyleOptionSpinBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
{
    int fw = option->frame ? style->pixelMetric(QStyle::PM_SpinBoxFrameWidth, option, widget) : 0;
    int bw;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
    if (option->buttonSymbols == QAbstractSpinBox::NoButtons) {
        bw = 0;
    } else
#endif
    {
        bw = qMax(style->pixelMetric(QStyle::PM_ScrollBarExtent, option, widget), qApp->globalStrut().width());
    }
    bool strutMode = qApp->globalStrut().height() > (option->rect.height() >> 1);
    QRect rect;

    switch (subControl) {
        case QStyle::SC_SpinBoxUp:
        case QStyle::SC_SpinBoxDown: {
            int h = option->rect.height() - 2 * fw;
            int t = option->rect.top() + fw;
            int l = option->rect.right() - bw - fw + 1;
            if (strutMode) {
                if (subControl == QStyle::SC_SpinBoxUp) {
                    l -= bw;
                }
            } else {
                if (subControl == QStyle::SC_SpinBoxDown) {
                    t += (h >> 1);
                }
                h = (h + 1) >> 1;
            }
            rect = QRect(l, t, bw, h);
            break;
        }
        case QStyle::SC_SpinBoxEditField: {
            if (strutMode) {
                bw *= 2;
            }
            rect = option->rect.adjusted(fw, fw, -fw - bw, -fw);
            break;
        }
        case QStyle::SC_SpinBoxFrame:
        default: // avoid warning
            rect = option->rect;
            break;
    }
    return style->visualRect(option->direction, option->rect, rect);
}


/*-----------------------------------------------------------------------*/

void paintComplexControlArea(QPainter *painter, const QStyleOption *option)
{
    // configuration
    const bool paintBackground = true;
    const bool paintSeparator = true;

    // background
    QColor color;
    if (paintBackground && option->state & QStyle::State_Enabled) {
        color = option->palette.color(QPalette::Window);
        // ### should arrow areas have hover highlight?
        if (false && option->state & QStyle::State_MouseOver) {
#if QT_VERSION >= QT_VERSION_CHECK(4, 3, 0)
            color = color.lighter(110);
#else
            color = color.light(110);
#endif
        } else {
#if QT_VERSION >= QT_VERSION_CHECK(4, 3, 0)
            color = color.lighter(107);
#else
            color = color.light(107);
#endif
        }
    } else {
        color = option->palette.color(QPalette::Base);
    }
    painter->fillRect(option->rect, color);

    // separator
    if (paintSeparator) {
        QRect rect = option->rect;
        if (option->direction == Qt::LeftToRight) {
            rect.setWidth(1);
        } else {
            rect.setLeft(rect.left() + rect.width() - 1);
        }
        painter->fillRect(rect, shaded_color(option->palette.color(QPalette::Window), -5));
    }
}


static void paintSpinBoxUpDown(QPainter *painter, const QStyleOptionSpinBox *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
{
    QStyleOption opt;
    opt.QStyleOption::operator=(*option);
    opt.rect = style->subControlRect(QStyle::CC_SpinBox, option, subControl, widget);
    if (!(option->activeSubControls & subControl)) {
        opt.state &= ~(QStyle::State_Sunken | QStyle::State_On | QStyle::State_MouseOver);
    }

    // button background
    paintComplexControlArea(painter, &opt);

    // button symbol
    QStyle::PrimitiveElement pe;
    if (!(option->stepEnabled & (subControl == QStyle::SC_SpinBoxUp ? QAbstractSpinBox::StepUpEnabled : QAbstractSpinBox::StepDownEnabled))) {
        opt.state &= ~(QStyle::State_Enabled | QStyle::State_MouseOver);
        opt.palette.setCurrentColorGroup(QPalette::Disabled);
    }
    // micro adjustments
    if (subControl == QStyle::SC_SpinBoxUp) {
        opt.rect.translate(0, 1);
    } else if (opt.rect.height() & 1) {
        opt.rect.translate(0, -1);
    }
    switch (option->buttonSymbols) {
        case QAbstractSpinBox::PlusMinus:
            pe = subControl == QStyle::SC_SpinBoxUp ? QStyle::PE_IndicatorSpinPlus : QStyle::PE_IndicatorSpinMinus;
            break;
        default:
            pe = subControl == QStyle::SC_SpinBoxUp ? QStyle::PE_IndicatorSpinUp : QStyle::PE_IndicatorSpinDown;
            break;
    }
    style->drawPrimitive(pe, &opt, painter, widget);
}


void paintSpinBox(QPainter *painter, const QStyleOptionSpinBox *option, const QWidget *widget, const QStyle *style)
{
    // up/down controls
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
    if (option->buttonSymbols != QAbstractSpinBox::NoButtons)
#endif
    {
        for (uint sc = QStyle::SC_SpinBoxUp; sc != QStyle::SC_SpinBoxFrame; sc <<= 1) {
            if (option->subControls & sc) {
                paintSpinBoxUpDown(painter, option, (QStyle::SubControl) sc, widget, style);
            }
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
#include <QtGui/QTabWidget>


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
#if 0
enum TabState
{
	TS_New,
	TS_Removed,
	TS_Inactive,
	TS_HoverAnim,
	TS_Hover,
	TS_ActiveAnim,
	TS_Active,
	TS_Moved,
	TS_LabelChanged	// text, icon or color changed
	TS_StateChanged	// disabled changed
};


class TabAnim
{
	qreal pos;
	qreal speed;
	int color;
};


class Tab
{
	public:
		TabState state;
		TabAnim anim;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
		QStyleOptionTabV3 oldOption;
#else
		QStyleOptionTabV2 oldOption;
#endif
};


/*-----------------------------------------------------------------------*/

class TabBarState
{
	public:
		TabBarState() : active_tab(0), hover_tab(0) { }

	public:
		QList<Tab> tabs;
		Tab *active_tab;
		Tab *hover_tab;
		int hover_x;
		int hover_y;
		int hover_counter;
};


const TabBarState *SkulpturePrivate::tabBarState(const QWidget *widget)
{
	if (qobject_cast<const QTabBar *>(widget)) {
		if ((int i = tabBarStates.indexOf(widget))) {
			return tabBarStates.at(i);
		}
		// add state if not found
		TabBarState *state = new TabBarState;
		if (state) {

		}
	}
	return 0;
}
#endif

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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
	r = r.united(option->tabBarRect);
#else
        r = r.unite(option->tabBarRect);
#endif
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


void paintTabWidgetFrame(QPainter *painter, const QStyleOptionTabWidgetFrame *option, const QWidget *widget)
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
	paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -40, 160);
#if 1
        painter->save();
        painter->setPen(QPen(QColor(0, 0, 0, 20), 1));
        painter->drawLine(option->rect.x() + 1, option->rect.bottom(), option->rect.right() - 1, option->rect.bottom());
        painter->drawLine(option->rect.right(), option->rect.top() + 1, option->rect.right(), option->rect.bottom());
        painter->restore();
#endif
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

void paintTabBarTabShape(QPainter *painter, const QStyleOptionTab *option, const QWidget *widget, const QStyle *style)
{
	Q_UNUSED(style);
        const QColor tabBackgroundColor = option->palette.color(QPalette::Active, QPalette::Window);
        bool mouse = (option->state & QStyle::State_MouseOver) && !(option->state & QStyle::State_Selected) && (option->state & QStyle::State_Enabled);
	QRect c_rect = option->rect;
	bool konq = false;

	if (widget && widget->parentWidget()) {
		if (!qstrcmp(widget->metaObject()->className(), "KTabBar") && !qstrcmp(widget->parentWidget()->metaObject()->className(), "KonqFrameTabs")) {
			konq = true;
		}
	}
	if (konq || (widget && !(qobject_cast<const QTabWidget *>(widget->parentWidget())))
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
            || (option->version >= 3 && ((QStyleOptionTabV3 *) option)->documentMode)
#endif
        ) {
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
			painter->fillRect(c_rect.adjusted(0, 0, -1, 0), tabBackgroundColor);
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.bottomLeft());
#if 0
				QColor c = option->palette.color(QPalette::Highlight);
				gradient.setColorAt(0.0, c);
				gradient.setColorAt(0.2, QColor(255, 255, 255, 20));
				gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
#else
				gradient.setColorAt(0.0, shaded_color(tabBackgroundColor, 20));
				gradient.setColorAt(1.0, shaded_color(tabBackgroundColor, 0));
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

			painter->fillRect(c_rect.adjusted(1, mouse ? 1 : 2, -1, -1), mouse ? tabBackgroundColor.darker(104) : tabBackgroundColor.darker(108));
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
			painter->fillRect(c_rect.adjusted(0, 0, -1, 0), tabBackgroundColor);
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.bottomLeft());
				gradient.setColorAt(0.0, shaded_color(tabBackgroundColor, 0));
				gradient.setColorAt(1.0, shaded_color(tabBackgroundColor, -5));
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

			painter->fillRect(c_rect.adjusted(1, 1, -1, mouse ? -1 : -2), mouse ? tabBackgroundColor.darker(104) : tabBackgroundColor.darker(108));
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
			painter->fillRect(c_rect.adjusted(0, 0, 0, -1), tabBackgroundColor);
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.topRight());
				gradient.setColorAt(0.0, shaded_color(tabBackgroundColor, 20));
				gradient.setColorAt(1.0, shaded_color(tabBackgroundColor, 0));
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
			painter->fillRect(c_rect.adjusted(mouse ? 1 : 2, 1, 1, -1), mouse ? tabBackgroundColor.darker(104) : tabBackgroundColor.darker(108));
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
			painter->fillRect(c_rect.adjusted(0, 0, 0, -1), tabBackgroundColor);
			if (option->state & QStyle::State_Enabled) {
				QLinearGradient gradient(c_rect.topLeft(), c_rect.topRight());
				gradient.setColorAt(0.0, shaded_color(tabBackgroundColor, 0));
				gradient.setColorAt(1.0, shaded_color(tabBackgroundColor, 10));
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
			painter->fillRect(c_rect.adjusted(-2, 1, mouse ? -1 : -2, -1), mouse ? tabBackgroundColor.darker(104) : tabBackgroundColor.darker(108));
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

#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
void paintIndicatorTabClose(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style)
{
    int offset = 0;
    QTabBar::Shape shape = QTabBar::RoundedNorth;

    if (widget) {
        if (QTabBar *tabbar = qobject_cast<QTabBar *>(widget->parentWidget())) {
            offset = TAB_SHIFT;
            shape = tabbar->shape();
            for (int i = 0; i < tabbar->count(); ++i) {
                if (tabbar->tabRect(i).contains(widget->mapToParent(QPoint(1, 1)))) {
                    if (i == tabbar->currentIndex()) {
                        offset = 0;
                    } else if (tabbar->tabRect(i).contains(tabbar->mapFromGlobal(QCursor::pos()))) {
                        offset = 0;
                    }
                    break;
                }
            }
            if (false /*tabPos(shape) == East || tabPos(shape) == South*/) {
                offset += 1;
            }
        }
    }
    QIcon::Mode iconMode = QIcon::Normal;
    painter->save();
    if (option->state & QStyle::State_Enabled) {
        if (option->state & QStyle::State_MouseOver || option->state & QStyle::State_Sunken) {
            // paintThinFrame(painter, option->rect, option->palette, 90, -30);
            iconMode = QIcon::Active;
            //painter->fillRect(option->rect.adjusted(1, 1, -1, -1), QColor(220, 0, 0));
            if (!(option->state & QStyle::State_Sunken)) {
                // paintThinFrame(painter, option->rect.adjusted(1, 1, -1, -1), option->palette, -60, 180);
            } else {
                //iconMode = QIcon::Selected;
            }
        } else {
            painter->setOpacity(0.7);
        }
    } else {
        //iconMode = QIcon::Disabled;
        painter->setOpacity(0.7);
    }
    QRect r = QRect(option->rect.center() - QPoint(option->state & QStyle::State_Sunken ? 3 : 4, option->state & QStyle::State_Sunken ? 3 : 4), QSize(10, 10));
    if (offset) {
        Affinity affinity;
        tabAffinity(shape, affinity, offset);
        r.translate(affinity.x1 + affinity.x2, affinity.y1 + affinity.y2);
    }
    painter->drawPixmap(r, style->standardIcon(QStyle::SP_TitleBarCloseButton, option, widget).pixmap(10, 10, iconMode));
    painter->restore();
}
#endif

void paintTabBarTabLabel(QPainter *painter, const QStyleOptionTab *option, const QWidget *widget, const QStyle *style)
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    QStyleOptionTabV3 opt;
#else
    QStyleOptionTabV2 opt;
#endif

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
		opt.rect.adjust(-2, 1, -1, 1);
		break;
	case South:
		opt.rect.adjust(-2, 0, -1, 0);
		break;
	case West:
	case East:
		painter->save();
		QMatrix mat;
		if (tabPos(option->shape) == West) {
			opt.rect.adjust(3, 0, 3, 0);
		} else {
			opt.rect.adjust(-1, 0, -1, 0);
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
#include <QtGui/QTextCursor>
#include <QtGui/QScrollBar>
#include <QtGui/QApplication>


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::removeCursorLine(QAbstractScrollArea *edit)
{
    Q_UNUSED (edit);

    if (oldEdit) {
        oldEdit->viewport()->update(QRect(0, oldCursorTop, oldCursorWidth, oldCursorHeight));
        oldEdit = 0;
    }
}


void SkulptureStyle::Private::updateCursorLine(QAbstractScrollArea *edit, const QRect &cursorRect)
{
    const int highlightMargin = qMin(2, widgetSize);
    QRect cursorLine = cursorRect;
    cursorLine.setLeft(0);
    cursorLine.setWidth(edit->viewport()->width());
    cursorLine.adjust(0, -highlightMargin, 0, highlightMargin);
    if (edit != oldEdit || cursorLine.top() != oldCursorTop
     || cursorLine.width() != oldCursorWidth
     || cursorLine.height() != oldCursorHeight
     || edit->viewport()->height() != oldHeight) {
        removeCursorLine(edit);
        oldEdit = edit;
        oldCursorTop = cursorLine.top();
        oldCursorWidth = cursorLine.width();
        oldCursorHeight = cursorLine.height();
        oldHeight = edit->viewport()->height();
        edit->viewport()->update(cursorLine);
    }
}


void SkulptureStyle::Private::paintCursorLine(QAbstractScrollArea *edit)
{
    if (edit == oldEdit) {
        QRect cursorLine = QRect(0, oldCursorTop, oldCursorWidth, oldCursorHeight);
        QPainter painter(edit->viewport());
        QPalette palette = edit->palette();
        QColor color = palette.color(QPalette::Highlight);
        color.setAlpha(40);
        painter.fillRect(cursorLine, color);
        if (edit->window()->testAttribute(Qt::WA_KeyboardFocusChange)) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
            color = palette.color(QPalette::Highlight).darker(120);
#else
            color = palette.color(QPalette::Highlight).dark(120);
#endif
            color.setAlpha(120);
            painter.fillRect(cursorLine.adjusted(0, cursorLine.height() - 3, 0, -2), color);
        }
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void SkulptureStyle::Private::handleCursor(QPlainTextEdit *edit)
{
    if (edit->hasFocus() && !edit->isReadOnly()) {
        QStyleOption option;
        option.initFrom(edit);
        int cursorWidth = q->SkulptureStyle::pixelMetric(PM_TextCursorWidth, &option, edit);
        if (edit->cursorWidth() != cursorWidth) {
            edit->setCursorWidth(cursorWidth);
        }
        updateCursorLine(edit, edit->cursorRect());
    } else {
        if (edit == oldEdit) {
            removeCursorLine(edit);
        }
    }
}
#endif

void SkulptureStyle::Private::handleCursor(QTextEdit *edit)
{
    if (edit->hasFocus() && !edit->isReadOnly()) {
        QStyleOption option;
        option.initFrom(edit);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        int cursorWidth = q->SkulptureStyle::pixelMetric(PM_TextCursorWidth, &option, edit);
#else
        int cursorWidth = q->SkulptureStyle::pixelMetric((QStyle::PixelMetric)((int) PM_CustomBase + 1), &option, edit);
#endif
        if (edit->cursorWidth() != cursorWidth) {
            edit->setCursorWidth(cursorWidth);
        }
#endif
        updateCursorLine(edit, edit->cursorRect());
    } else {
        if (edit == oldEdit) {
            removeCursorLine(edit);
        }
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
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                if (margin < 12) {
                    format.setTopMargin(widgetSize - ((textShift + 1) >> 1));
                    format.setBottomMargin(widgetSize + ((textShift + 1) >> 1));
                }
#endif
                root->setFrameFormat(format);
	//	edit->insertPlainText(QLatin1String(""));
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
}


void SkulptureStyle::drawItemText(QPainter * painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    int textShift = 0;

    if (!(alignment & (Qt::AlignTop | Qt::AlignBottom))) {
        textShift = d->verticalTextShift(painter->fontMetrics());
        if (textShift & 1 && !(rectangle.height() & 1)) {
            textShift += 1;
        }
    }
    ParentStyle::drawItemText(painter, textShift == 0 || textShift == -1 ? rectangle : rectangle.adjusted(0, (-textShift) >> 1, 0, (-textShift) >> 1), alignment, palette, enabled, text, textRole);
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


/*-----------------------------------------------------------------------*/

#define PAINT_SEPARATOR 0


/*-----------------------------------------------------------------------*/

extern void paintCachedGrip(QPainter *painter, const QStyleOption *option, QPalette::ColorRole bgrole);

void paintToolBarSeparator(QPainter *painter, const QStyleOptionToolBar *option)
{
#if 0
	int d = 3;
	QRect rect(QRect(option->rect).center() - QPoint(d / 2, d / 2), QSize(d, d));
	QStyleOption iOption;
	iOption.QStyleOption::operator=(*option);
	if (option->state & QStyle::State_Horizontal) {
		iOption.rect = rect.adjusted(1, 0, 1, 0);
	} else {
		iOption.rect = rect.adjusted(0, 1, 0, 1);
	}
	iOption.palette.setCurrentColorGroup(QPalette::Disabled);
//	iOption.state &= ~QStyle::State_Enabled;
	iOption.palette.setColor(QPalette::Button, iOption.palette.color(QPalette::Window));
	paintCachedGrip(painter, &iOption, QPalette::Window);
#else
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
#endif
}

/*-----------------------------------------------------------------------*/

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
#if 0
    QLinearGradient gradient(option->rect.topLeft(), option->rect.bottomLeft());
    gradient.setColorAt(0.0, option->palette.color(QPalette::Window).lighter(105));
    gradient.setColorAt(1.0, option->palette.color(QPalette::Window));
    painter->fillRect(option->rect, gradient);
    QRect r;
//    r = option->rect;
//    r.setTop(r.top() + r.height() - 1);
//    painter->fillRect(r, option->palette.color(QPalette::Window).darker(105));
    r = option->rect;
    r.setHeight(1);
    painter->fillRect(r, option->palette.color(QPalette::Window).darker(105));
//	painter->fillRect(option->rect, option->palette.color(QPalette::Window));
//	paintThinFrame(painter, option->rect, option->palette, -20, 60);
#endif
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
#if 0
        if (option->state & QStyle::State_Horizontal) {
            paintThinFrame(painter, option->rect.adjusted(-1, 0, 1, 0), option->palette, 80, -30);
            painter->fillRect(option->rect.adjusted(0, 1, 0, -1), QColor(200, 210, 230));
            QLinearGradient toolBarGradient(option->rect.topLeft(), option->rect.bottomLeft());
            toolBarGradient.setColorAt(0.0, QColor(0, 0, 0, 30));
            toolBarGradient.setColorAt(0.05, QColor(0, 0, 0, 0));
#if 0
            toolBarGradient.setColorAt(0.5, QColor(0, 0, 0, 10));
            toolBarGradient.setColorAt(0.51, QColor(0, 0, 0, 30));
#endif
            toolBarGradient.setColorAt(0.8, QColor(0, 0, 0, 20));
            toolBarGradient.setColorAt(1.0, QColor(0, 0, 0, 0));
            painter->fillRect(option->rect.adjusted(0, 1, 0, -1), toolBarGradient);
        } else {
            paintThinFrame(painter, option->rect.adjusted(0, -1, 0, 1), option->palette, 80, -30);
        }
#else
//        painter->fillRect(option->rect, QColor(0, 0, 0, 10));
#endif
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
// FIXME
#if (QT_VERSION < QT_VERSION_CHECK(4, 3, 0))
#define HasMenu Menu
#endif

extern void paintMenuTitle(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style);

void paintToolButton(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style)
{
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
		} else {
                    const QToolButton *button = qobject_cast<const QToolButton *>(widget);

                    if (button && button->isDown() && button->toolButtonStyle() == Qt::ToolButtonTextBesideIcon) {
                        if (widget->parentWidget() && widget->parentWidget()->inherits("KMenu")) {
                            paintMenuTitle(painter, option, widget, style);
                            return;
                        }
                    }
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


void paintToolButtonLabel(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style)
{
    QStyleOptionToolButton opt = *option;
    if (option->state & QStyle::State_AutoRaise) {
        if (!(option->state & QStyle::State_Enabled) || !(option->state & QStyle::State_MouseOver)) {
            opt.palette.setColor(QPalette::ButtonText, opt.palette.color(QPalette::WindowText));
        }
    }
    ((QCommonStyle *) style)->QCommonStyle::drawControl(QStyle::CE_ToolButtonLabel, &opt, painter, widget);
}


/*-----------------------------------------------------------------------*/

QRect subControlRectToolButton(const QStyleOptionToolButton *option, QStyle::SubControl subControl, const QWidget *widget, const QStyle *style)
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
	return ((QCommonStyle *) style)->QCommonStyle::subControlRect(QStyle::CC_ToolButton, option, subControl, widget);
}


QSize sizeFromContentsToolButton(const QStyleOptionToolButton *option, const QSize &contentsSize, const QWidget *widget, const QStyle *style, int toolButtonSize)
{
	QSize size = contentsSize + QSize(4, 4);

        if (toolButtonSize >= 0) {
            if (option->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                size += QSize(toolButtonSize, qMax(2, toolButtonSize));
            } else {
                size += QSize(toolButtonSize, toolButtonSize);
            }
        } else {
            size += QSize(4, 4);
        }
	if (widget && !qstrcmp(widget->metaObject()->className(), "KAnimatedButton")) {
            return contentsSize + QSize(4, 4);
	}
        if (widget && !qstrcmp(widget->metaObject()->className(), "QtColorButton")) {
            return contentsSize + QSize(12, 12);
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

    Q_FOREACH (QToolButton *toolbutton, toolbuttons) {
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
#include <QtGui/QSlider>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
#include <QtGui/QDialogButtonBox>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
#include <QtGui/QWizard>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
#include <QtGui/QFormLayout>
#endif


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
        VarParent,
        Value
    };

    const char * const label;
    int id;
    int type;
    int value;
};


static const struct StyleSetting styleHintSettings[] =
{
    /* entries are stricly sorted in Qt order for future lookup table */
    { "General/EtchDisabledText", QStyle::SH_EtchDisabledText, StyleSetting::Bool, 1 },
///    { "General/DitherDisabledText", QStyle::SH_DitherDisabledText, StyleSetting::Bool, 0 },
    { "ScrollBar/MiddleClickAbsolutePosition", QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, StyleSetting::Bool, 1 },
//    { "ScrollBar/ScrollWhenPointerLeavesControl", QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl, StyleSetting::Parent, 0 },
//    { "TabWidget/SelectMouseType", QStyle::SH_TabBar_SelectMouseType, StyleSetting::Parent, 0 },
///    { "TabWidget/TabBarAlignment", QStyle::SH_TabBar_Alignment, StyleSetting::Alignment, Qt::AlignCenter },
//    { "ItemView/HeaderArrowAlignment", QStyle::SH_Header_ArrowAlignment, StyleSetting::Parent, 0 },
    { "Slider/SnapToValue", QStyle::SH_Slider_SnapToValue, StyleSetting::Bool, 1 },
//    { "Slider/SloppyKeyEvents", QStyle::SH_Slider_SloppyKeyEvents, StyleSetting::Parent, 0 },
//    { "ProgressDialog/CenterCancelButton", QStyle::SH_ProgressDialog_CenterCancelButton, StyleSetting::Parent, 0 },
//    { "ProgressDialog/TextLabelAlignment", QStyle::SH_ProgressDialog_TextLabelAlignment, StyleSetting::Parent, 0 },
    { "PrintDialog/RightAlignButtons", QStyle::SH_PrintDialog_RightAlignButtons, StyleSetting::Bool, 1 },
//    { "Window/SpaceBelowMenuBar", QStyle::SH_MainWindow_SpaceBelowMenuBar, StyleSetting::Parent, 0 },
    { "FontDialog/SelectAssociatedText", QStyle::SH_FontDialog_SelectAssociatedText, StyleSetting::Bool, 1 },
    { "Menu/AllowActiveAndDisabled", QStyle::SH_Menu_AllowActiveAndDisabled, StyleSetting::Bool, 1 },
//    { "Menu/SpaceActivatesItem", QStyle::SH_Menu_SpaceActivatesItem, StyleSetting::Parent, 0 },
// ### dynamic { "Menu/SubMenuPopupDelay", QStyle::SH_Menu_SubMenuPopupDelay, StyleSetting::Milliseconds, 100 },
///    { "ItemView/FrameOnlyAroundContents", QStyle::SH_ScrollView_FrameOnlyAroundContents, StyleSetting::Bool, 0 },
    { "Menu/AltKeyNavigation", QStyle::SH_MenuBar_AltKeyNavigation, StyleSetting::Bool, 1 },
    { "ComboBox/ListMouseTracking", QStyle::SH_ComboBox_ListMouseTracking, StyleSetting::Bool, 1 },
    { "Menu/MouseTracking", QStyle::SH_Menu_MouseTracking, StyleSetting::Bool, 1 },
    { "Menu/BarMouseTracking", QStyle::SH_MenuBar_MouseTracking, StyleSetting::Bool, 1 },
//    { "ItemView/ChangeHighlightOnFocus", QStyle::SH_ItemView_ChangeHighlightOnFocus, StyleSetting::Parent, 0 },
//    { "Window/ShareActivation", QStyle::SH_Widget_ShareActivation, StyleSetting::Parent, 0 },
//    { "MDI/Workspace/FillSpaceOnMaximize", QStyle::SH_Workspace_FillSpaceOnMaximize, StyleSetting::Parent, 0 },
//    { "ComboBox/Popup", QStyle::SH_ComboBox_Popup, StyleSetting::Parent, 0 },
///    { "MDI/TitleBar/NoBorder", QStyle::SH_TitleBar_NoBorder, StyleSetting::Bool, 0 },
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
///    { "Slider/StopMouseOverSlider", QStyle::SH_Slider_StopMouseOverSlider, StyleSetting::Bool, 1 },
#else
///    { "Slider/StopMouseOverSlider", QStyle::SH_ScrollBar_StopMouseOverSlider, StyleSetting::Bool, 1 },
#endif
//    { "General/BlinkCursorWhenTextSelected", QStyle::SH_BlinkCursorWhenTextSelected, StyleSetting::Parent, 0 },
//    { "General/FullWidthSelection", QStyle::SH_RichText_FullWidthSelection, StyleSetting::Bool, 1 },
//    { "Menu/Scrollable", QStyle::SH_Menu_Scrollable, StyleSetting::Parent, 0 },
//    { "GroupBox/TextLabelVerticalAlignment", QStyle::SH_GroupBox_TextLabelVerticalAlignment, StyleSetting::Parent, 0 },
// ### dynamic { "GroupBox/TextLabelColor", QStyle::SH_GroupBox_TextLabelColor, StyleSetting::Color, 0xFF000000 },
//    { "Menu/SloppySubMenus", QStyle::SH_Menu_SloppySubMenus, StyleSetting::Parent, 0 },
// ### dynamic { "ItemView/GridLineColor", QStyle::SH_Table_GridLineColor, StyleSetting::Color, 0xFFD0D0D0 },
// ### dynamic { "LineEdit/PasswordCharacter", QStyle::SH_LineEdit_PasswordCharacter, StyleSetting::Char, 10039 },
//    { "Dialog/DefaultButton", QStyle::SH_DialogButtons_DefaultButton, StyleSetting::Parent, 0 },
//    { "ToolBox/SelectedPageTitleBold", QStyle::SH_ToolBox_SelectedPageTitleBold, StyleSetting::Bool, 1 },
//    { "TabWidget/TabBarPreferNoArrows", QStyle::SH_TabBar_PreferNoArrows, StyleSetting::Parent, 0 },
//    { "ScrollBar/LeftClickAbsolutePosition", QStyle::SH_ScrollBar_LeftClickAbsolutePosition, StyleSetting::Parent, 0 },
//    { "ItemView/Compat/ExpansionSelectMouseType", QStyle::SH_Q3ListViewExpand_SelectMouseType, StyleSetting::Parent, 0 },
// ### dynamic { "General/UnderlineShortcut", QStyle::SH_UnderlineShortcut, StyleSetting::Bool, 0 },
//    { "SpinBox/AnimateButton", QStyle::SH_SpinBox_AnimateButton, StyleSetting::Parent, 0 },
//    { "SpinBox/KeyPressAutoRepeatRate", QStyle::SH_SpinBox_KeyPressAutoRepeatRate, StyleSetting::Parent, 0 },
//    { "SpinBox/ClickAutoRepeatRate", QStyle::SH_SpinBox_ClickAutoRepeatRate, StyleSetting::Parent, 0 },
//    { "Menu/FillScreenWithScroll", QStyle::SH_Menu_FillScreenWithScroll, StyleSetting::Parent, 0 },
//    { "ToolTip/Opacity", QStyle::SH_ToolTipLabel_Opacity, StyleSetting::Parent, 0 },
//    { "Menu/DrawMenuBarSeparator", QStyle::SH_DrawMenuBarSeparator, StyleSetting::Parent, 0 },
//    { "MDI/TitleBar/ModifyNotification", QStyle::SH_TitleBar_ModifyNotification, StyleSetting::Parent, 0 },
//    { "Button/FocusPolicy", QStyle::SH_Button_FocusPolicy, StyleSetting::Parent, 0 },
//    { "Menu/DismissOnSecondClick", QStyle::SH_MenuBar_DismissOnSecondClick, StyleSetting::Parent, 0 },
//    { "MessageBox/UseBorderForButtonSpacing", QStyle::SH_MessageBox_UseBorderForButtonSpacing, StyleSetting::Parent, 0 },
    { "MDI/TitleBar/AutoRaise", QStyle::SH_TitleBar_AutoRaise, StyleSetting::Bool, 1 },
    { "ToolBar/PopupDelay", QStyle::SH_ToolButton_PopupDelay, StyleSetting::Milliseconds, 250 },
// ### dynamic { "General/FocusFrameMask", QStyle::SH_FocusFrame_Mask, StyleSetting::Parent, 0 },
// ### dynamic { "General/RubberBandMask", QStyle::SH_RubberBand_Mask, StyleSetting::Parent, 0 },
// ### dynamic { "General/WindowFrameMask", QStyle::SH_WindowFrame_Mask, StyleSetting::Parent, 0 },
//    { "SpinBox/DisableControlsOnBounds", QStyle::SH_SpinControls_DisableOnBounds, StyleSetting::Parent, 0 },
//    { "Dial/BackgroundRole", QStyle::SH_Dial_BackgroundRole, StyleSetting::Parent, 0 },
//    { "ComboBox/LayoutDirection", QStyle::SH_ComboBox_LayoutDirection, StyleSetting::Parent, 0 },
//    { "ItemView/EllipsisLocation", QStyle::SH_ItemView_EllipsisLocation, StyleSetting::Parent, 0 },
//    { "ItemView/ShowDecorationSelected", QStyle::SH_ItemView_ShowDecorationSelected, StyleSetting::Parent, 0 },
// ### from KDE { "ItemView/ActivateItemOnSingleClick", QStyle::SH_ItemView_ActivateItemOnSingleClick, StyleSetting::Bool, 1 },
//    { "ScrollBar/ContextMenu", QStyle::SH_ScrollBar_ContextMenu, StyleSetting::Parent, 0 },
//    { "ScrollBar/RollBetweenButtons", QStyle::SH_ScrollBar_RollBetweenButtons, StyleSetting::Parent, 0 },
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
//    { "Slider/AbsoluteSetButtons", QStyle::SH_Slider_AbsoluteSetButtons, StyleSetting::Parent, 0 },
//    { "Slider/PageSetButtons", QStyle::SH_Slider_PageSetButtons, StyleSetting::Parent, 0 },
//    { "Menu/KeyboardSearch", QStyle::SH_Menu_KeyboardSearch, StyleSetting::Parent, 0 },
//    { "TabWidget/ElideMode", QStyle::SH_TabBar_ElideMode, StyleSetting::Parent, 0 },
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
//    { "Dialog/ButtonLayout", QStyle::SH_DialogButtonLayout, StyleSetting::Value, QDialogButtonBox::KdeLayout },
#else
    { "Dialog/ButtonLayout", QStyle::SH_DialogButtonLayout, StyleSetting::Value, QDialogButtonBox::KdeLayout },
#endif
//    { "ComboBox/PopupFrameStyle", QStyle::SH_ComboBox_PopupFrameStyle, StyleSetting::Parent, 0 },
    { "MessageBox/AllowTextInteraction", QStyle::SH_MessageBox_TextInteractionFlags, StyleSetting::Bool, 1 },
// ### from KDE { "Dialog/ButtonsHaveIcons", QStyle::SH_DialogButtonBox_ButtonsHaveIcons, StyleSetting::Bool, 0 },
//    { "General/SpellCheckUnderlineStyle", QStyle::SH_SpellCheckUnderlineStyle, StyleSetting::Parent, 0 },
    { "MessageBox/CenterButtons", QStyle::SH_MessageBox_CenterButtons, StyleSetting::Bool, 0 },
//    { "Menu/SelectionWrap", QStyle::SH_Menu_SelectionWrap, StyleSetting::Parent, 0 },
//    { "ItemView/MovementWithoutUpdatingSelection", QStyle::SH_ItemView_MovementWithoutUpdatingSelection, StyleSetting::Parent, 0 },
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
// ### dynamic { "General/ToolTipMask", QStyle::SH_ToolTip_Mask, StyleSetting::Parent, 0 },
//    { "General/FocusFrameAboveWidget", QStyle::SH_FocusFrame_AboveWidget, StyleSetting::Parent, 0 },
//    { "General/FocusIndicatorTextCharFormat", QStyle::SH_TextControl_FocusIndicatorTextCharFormat, StyleSetting::Parent, 0 },
//    { "Dialog/WizardStyle", QStyle::SH_WizardStyle, StyleSetting::Value, QWizard::ModernStyle },
    { "ItemView/ArrowKeysNavigateIntoChildren", QStyle::SH_ItemView_ArrowKeysNavigateIntoChildren, StyleSetting::Bool, 1 },
// ### dynamic { "General/MenuMask", QStyle::SH_Menu_Mask, StyleSetting::Parent, 0 },
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//    { "Menu/FlashTriggeredItem", QStyle::SH_Menu_FlashTriggeredItem, StyleSetting::Parent, 0 },
//    { "Menu/FadeOutOnHide", QStyle::SH_Menu_FadeOutOnHide, StyleSetting::Parent, 0 },
//    { "SpinBox/ClickAutoRepeatThreshold", QStyle::SH_SpinBox_ClickAutoRepeatThreshold, StyleSetting::Parent, 0 },
//    { "ItemView/PaintAlternatingRowColorsForEmptyArea", QStyle::SH_ItemView_PaintAlternatingRowColorsForEmptyArea, StyleSetting::Parent, 0 },
//    { "FormLayout/WrapPolicy", QStyle::SH_FormLayoutWrapPolicy, StyleSetting::Value, QFormLayout::DontWrapRows },
//    { "TabWidget/DefaultTabPosition", QStyle::SH_TabWidget_DefaultTabPosition, StyleSetting::Parent, 0 },
//    { "ToolBar/Movable", QStyle::SH_ToolBar_Movable, StyleSetting::Parent, 0 },
    { "FormLayout/FieldGrowthPolicy", QStyle::SH_FormLayoutFieldGrowthPolicy, StyleSetting::Value, QFormLayout::ExpandingFieldsGrow },
//    { "FormLayout/FormAlignment", QStyle::SH_FormLayoutFormAlignment, StyleSetting::Alignment, Qt::AlignLeft | Qt::AlignTop },
    { "FormLayout/LabelAlignment", QStyle::SH_FormLayoutLabelAlignment, StyleSetting::Alignment, Qt::AlignRight | Qt::AlignTop },
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
//    { "ItemView/DrawDelegateFrame", QStyle::SH_, StyleSetting::Parent, 0 },
//    { "TabWidget/CloseButtonPosition", QStyle::SH_TabBar_CloseButtonPosition, StyleSetting::Parent, 0 },
//    { "DockWidget/ButtonsHaveFrame", QStyle::SH_DockWidget_ButtonsHaveFrame, StyleSetting::Parent, 0 },
#endif
    { 0, -1, 0, 0 }
};


extern int getRubberBandMask(QStyleHintReturnMask *mask, const QStyleOption *option, const QWidget *widget);
extern int getWindowFrameMask(QStyleHintReturnMask *mask, const QStyleOptionTitleBar *option, const QWidget *widget);

int SkulptureStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
//	return ParentStyle::styleHint(hint, option, widget, returnData);
	// TODO implement caching
	const StyleSetting *setting = &styleHintSettings[0];
	QVariant value;

	switch (hint) {
            case QStyle::SH_Menu_SubMenuPopupDelay: {
                return d->subMenuDelay;
            }
            case QStyle::SH_TabBar_Alignment: {
                return d->centerTabs ? Qt::AlignHCenter : Qt::AlignLeft;
            }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
            case QStyle::SH_Slider_StopMouseOverSlider: {
#else
            case QStyle::SH_ScrollBar_StopMouseOverSlider: {
#endif
                // this works around a bug with Qt 4.4.2
                return qobject_cast<const QSlider *>(widget) != 0;
            }
            case QStyle::SH_ItemView_ActivateItemOnSingleClick: {
                // ### use KDE setting
                return d->useSingleClickToActivateItems;
            }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
            case QStyle::SH_DialogButtonBox_ButtonsHaveIcons: {
                // ### use KDE setting
                return 0;
            }
#endif
#if 1
            case QStyle::SH_GroupBox_TextLabelColor: {
                QPalette palette;
                if (option) {
                    palette = option->palette;
                } else if (widget) {
                    palette = widget->palette();
                }
                return palette.color(QPalette::WindowText).rgba();
            }
#endif
            case QStyle::SH_Table_GridLineColor: {
                QPalette palette;
                if (option) {
                    palette = option->palette;
                } else if (widget) {
                    palette = widget->palette();
                }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
                return palette.color(QPalette::Base).darker(120).rgba();
#else
                return palette.color(QPalette::Base).dark(120).rgba();
#endif
            }
            case QStyle::SH_LineEdit_PasswordCharacter: {
                QFontMetrics fm = option ? option->fontMetrics : (widget ? widget->fontMetrics() : QFontMetrics(QFont()));
                for (int i = 0; i < d->passwordCharacters.length(); ++i) {
                    if (fm.inFont(d->passwordCharacters.at(i))) {
                        return d->passwordCharacters.at(i).unicode();
                    }
                }
                return int('*');
            }
            case QStyle::SH_UnderlineShortcut: {
                if (d->hideShortcutUnderlines) {
                    return (d->shortcut_handler->underlineShortcut(widget));
                }
                return true;
            }
            case QStyle::SH_RubberBand_Mask: {
                QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData);
                if (mask) {
                    return getRubberBandMask(mask, option, widget);
                }
                return 0;
            }
            case QStyle::SH_WindowFrame_Mask: {
                QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData);
                const QStyleOptionTitleBar *titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
                if (mask && titleBarOption) {
                    return getWindowFrameMask(mask, titleBarOption, widget);
                }
                return 0;
            }
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
	if (setting && d->settings && setting->type != StyleSetting::Parent && !d->settings->contains(QLatin1String(setting->label))) {
		d->settings->setValue(QLatin1String(setting->label), value);
	}
#endif
	if (setting) {
		if (d->settings) {
			value = d->settings->value(QLatin1String(setting->label), value);
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
/*
 * Read color out of current settings group
 *
 * color - where to store the resulting color
 * s - Settings
 * colorName - name of color, such as "checkMark", first letter should be not be capitalized
 * n - for multiple color entries, returns the "n"th color (n = 1, 2, ...) otherwise use n = 0.
 *
 * This will do the following:
 *
 * 1. check if "custom<ColorName>Color<n>" is set to "false" -> return false with "color" unmodified
 * 2. read color entry from "<colorName>Color<n>"
 * 3. read color opacity from "<colorName>Color<n>Opacity"
 * 4. return true with "color" modified accordingly
 *
 */

static bool readSettingsColor(QColor &color, const QSettings &s, const QString &colorName, int n = 0)
{
    QString cName = colorName + QLatin1String("Color");
    if (n > 0) {
        cName += QString::number(n);
    }
    if (s.value(QLatin1String("custom") + cName.at(0).toUpper() + cName.mid(1), true).toBool() == false) {
        return false;
    }
    QString val = s.value(cName).toString();
    if (!val.isEmpty()) {
        QColor c = QColor(val);
        if (c.isValid()) {
            color = c;
            int opacity = s.value(cName + QLatin1String("Opacity"), -1).toInt();
            if (opacity >= 0 && opacity <= 255) {
                color.setAlpha(opacity);
            }
            return true;
        }
    }
    return false;
}


/*-----------------------------------------------------------------------*/
/*
 * Read gradient out of current settings group
 *
 * gradient - where to store the resulting gradient (only the color stops are modified)
 * s - Settings
 * gradientName - name of gradient, such as "activeTabTop", first letter should be not be capitalized
 *
 * This returns true with "gradient" colors modified accordingly, or false, if no color was found.
 *
 * Limitations:
 *
 * Maximum number of gradients per background: 9
 * Maximum number of colors per gradient: 2
 *
 */

#define MIN_STOP 0      // must be 0
#define MAX_STOP 100

static inline qreal topToStop(int v)
{
    if (v <= MIN_STOP) {
        return qreal(0.00000);
    } else if (v >= MAX_STOP) {
        return qreal(0.99999);
    }
    return v / qreal(MAX_STOP) - qreal(0.00001);
}


static inline qreal bottomToStop(int v)
{
    if (v <= 0) {
        return qreal(0.00001);
    } else if (v >= MAX_STOP) {
        return qreal(1.00000);
    }
    return v / qreal(MAX_STOP) + qreal(0.00001);
}


static bool readSettingsGradient(QGradient &gradient, const QSettings &s, const QString &gradientName)
{
    QColor background(s.value(gradientName + QLatin1String("background")).toString());
    if (!background.isValid()) {
        return false;
    }

    bool hasTop = false;
    bool hasBottom = false;
    int numGradients = s.value(gradientName + QLatin1String("numGradients"), 0).toInt();
    numGradients = qMin(numGradients, 9); // single digit limit
    for (int i = 1; i <= numGradients; ++i) {
        QString gradientPrefix = gradientName + QChar('g', 0) + QChar('0' + i, 0);

        int top = s.value(gradientPrefix + QLatin1String("Top"), -1).toInt();
        int bottom = s.value(gradientPrefix + QLatin1String("Bottom"), -1).toInt();
        if (top == MIN_STOP) {
            hasTop = true;
        }
        if (bottom == MAX_STOP) {
            hasBottom = true;
        }
        if (top >= 0 && bottom >= 0) {
            QColor c[9 + 1];
            int k = 0;
            for (int n = 1; n <= 9; ++n) { // single digit limit
                if (readSettingsColor(c[n], s, gradientPrefix, n)) {
                    ++k;
                } else {
                    // force continous color numbering
                    break;
                }
            }
            // k colors are found, spread lineary between top ... bottom
            if (k > 1) {
                // FIXME blindly assumes k == 2 for now
                gradient.setColorAt(topToStop(top), c[1]);
                gradient.setColorAt(bottomToStop(bottom), c[2]);
            } else if (k == 1) {
                gradient.setColorAt(topToStop(top), c[1]);
                gradient.setColorAt(bottomToStop(bottom), c[1]);
            }
        }
    }
    // FIXME does not detect "holes" between gradients
    // that are to be "filled" with background
    if (!hasTop) {
        gradient.setColorAt(0, background);
    }
    if (!hasBottom) {
        gradient.setColorAt(1, background);
    }
    return true;
}


/*-----------------------------------------------------------------------*/
/*
 * Read domino settings out of current settings group
 *
 */

void SkulptureStyle::Private::readDominoSettings(const QSettings &s)
{
    static const char * const gradientName[] = {
        "tabTop", "tabBottom", "activeTabTop", "activeTabBottom",
        "btn", "checkItem", "header", "scrollBar", "scrollBarGroove"
    };
    static const char * const colorName[] = {
        "checkMark", "groupBoxBackground", "popupMenu",
        "selMenuItem", "toolTip"
    };

    for (uint i = 0; i < array_elements(gradientName); ++i) {
        QGradient gradient;
        if (readSettingsGradient(gradient, s, QLatin1String(gradientName[i]) + QLatin1String("Surface_"))) {
#if 0
            printf("domino: gradient[%s]=", gradientName[i]);
            QGradientStops stops = gradient.stops();
            for (int i = 0; i < stops.size(); ++i) {
                QGradientStop stop = stops.at(i);
                QColor color = stop.second;
                printf("(%.6g=#%2x%2x%2x/a=#%2x)", stop.first, color.red(), color.green(), color.blue(), color.alpha());
            }
            printf("\n");
#endif
            // only save stops from the gradient
//            dominoGradientStops[i] = gradient.stops();
        }
    }
    for (uint i = 0; i < array_elements(colorName); ++i) {
        QColor color;
        if (readSettingsColor(color, s, QLatin1String(colorName[i]))) {
#if 0
            printf("domino: color[%s]=#%2x%2x%2x/a=#%2x\n", colorName[i], color.red(), color.green(), color.blue(), color.alpha());
#endif
//            dominoCustomColors[i] = color;
        }
    }
    animateProgressBars = s.value(QLatin1String("animateProgressBar"), animateProgressBars).toBool();
    centerTabs = s.value(QLatin1String("centerTabs"), centerTabs).toBool();
#if 0
    readSettingsColor(buttonContourColor, s, QLatin1String("buttonContour"));
    readSettingsColor(buttonDefaultContourColor, s, QLatin1String("buttonDefaultContour"));
    readSettingsColor(buttonMouseOverContourColor, s, QLatin1String("buttonMouseOverContour"));
    readSettingsColor(buttonPressedContourColor, s, QLatin1String("buttonPressedContour"));
    readSettingsColor(indicatorButtonColor, s, QLatin1String("indicatorButton"));
    readSettingsColor(indicatorColor, s, QLatin1String("indicator"));
    readSettingsColor(rubberBandColor, s, QLatin1String("rubberBand"));
    readSettingsColor(textEffectButtonColor, s, QLatin1String("textEffectButton"));
    readSettingsColor(textEffectColor, s, QLatin1String("textEffect"));

    highlightToolBtnIcon = s.value(QLatin1String("highlightToolBtnIcon"), highlightToolBtnIcon).toBool();
    indentPopupMenuItems = s.value(QLatin1String("indentPopupMenuItems"), indentPopupMenuItems).toBool();
    smoothScrolling = s.value(QLatin1String("smoothScrolling"), smoothScrolling).toBool();
    tintGroupBoxBackground = s.value(QLatin1String("tintGroupBoxBackground"), tintGroupBoxBackground).toBool();
    indicateFocus = s.value(QLatin1String("indicateFocus"), indicateFocus).toBool();

    drawButtonSunkenShadow = s.value(QLatin1String("drawButtonSunkenShadow"), drawButtonSunkenShadow).toBool();
    drawFocusUnderline = s.value(QLatin1String("drawFocusUnderline"), drawFocusUnderline).toBool();
    drawPopupMenuGradient = s.value(QLatin1String("drawPopupMenuGradient"), drawPopupMenuGradient).toBool();
    drawTextEffect = s.value(QLatin1String("drawTextEffect"), drawTextEffect).toBool();
    drawToolButtonAsButton = s.value(QLatin1String("drawToolButtonAsButton"), drawToolButtonAsButton).toBool();
    drawTriangularExpander = s.value(QLatin1String("drawTriangularExpander"), drawTriangularExpander).toBool();
#endif
}


/*-----------------------------------------------------------------------*/

void SkulptureStyle::Private::readSettings(const QSettings &s)
{
    // defaults
    animateProgressBars = true;
    verticalArrowMode = SkulptureMode;
    horizontalArrowMode = SkulptureMode;
    hideShortcutUnderlines = true;
    centerTabs = false;
    makeDisabledWidgetsTransparent = true;
    transparentPlacesPanel = false;
    forceSpacingAndMargins = false;
    useIconColumnForCheckIndicators = false;
    useSelectionColorForCheckedIndicators = false;
    useSelectionColorForSelectedMenuItems = false;
    useSingleClickToActivateItems = true;

    dialogMargins = -1;
    horizontalSpacing = -1;
    labelSpacing = -1;
    menuBarSize = -1;
    menuItemSize = -1;
    pushButtonSize = -1;
    scrollBarSize = -1;
    scrollBarLength = -1;
    sliderSize = -1;
    sliderLength = -1;
    tabBarSize = -1;
    toolButtonSize = -1;
    verticalSpacing = -1;
    widgetMargins = -1;
    widgetSize = -1;
    textShift = 0;

    buttonGradient = 0;
    buttonRoundness = 0;

    passwordCharacters = QString(QChar(ushort(10039)));
    textCursorWidth = 0;

    subMenuDelay = 100;

    // legacy settings import: domino 0.4
    QString dominoConfigFile = s.value(QLatin1String("LegacyImport/DominoConfiguration")).toString();
    if (!dominoConfigFile.isEmpty()) {
        QSettings s(dominoConfigFile, QSettings::IniFormat);
        s.beginGroup(QLatin1String("Settings"));
        readDominoSettings(s);
        s.endGroup();
    }

    // native settings
    animateProgressBars = s.value(QLatin1String("ProgressBar/AnimateProgressBars"), animateProgressBars).toBool();
    if (s.contains(QLatin1String("ScrollBar/AllowScrollBarSliderToCoverArrows"))
        && !s.contains(QLatin1String("ScrollBar/VerticalArrowMode"))) {
        verticalArrowMode = s.value(QLatin1String("ScrollBar/AllowScrollBarSliderToCoverArrows"), true).toBool() ? SkulptureMode : WindowsMode;
    } else {
        QString mode = s.value(QLatin1String("ScrollBar/VerticalArrowMode"), QLatin1String("Covered")).toString();
        if (mode == QLatin1String("Top")) {
            verticalArrowMode = NextMode;
        } else if (mode == QLatin1String("Bottom")) {
            verticalArrowMode = PlatinumMode;
        } else if (mode == QLatin1String("BottomTop") || mode == QLatin1String("Bottom/Top")) {
            verticalArrowMode = WindowsMode;
        } else if (mode == QLatin1String("KDEMode")) {
            verticalArrowMode = KDEMode;
        } else if (mode == QLatin1String("Covered")) {
            verticalArrowMode = SkulptureMode;
        } else if (mode == QLatin1String("None")) {
            verticalArrowMode = NoArrowsMode;
        } else {
            verticalArrowMode = SkulptureMode;
        }
    }
    if (s.contains(QLatin1String("ScrollBar/AllowScrollBarSliderToCoverArrows"))
        && !s.contains(QLatin1String("ScrollBar/HorizontalArrowMode"))) {
        horizontalArrowMode = s.value(QLatin1String("ScrollBar/AllowScrollBarSliderToCoverArrows"), true).toBool() ? SkulptureMode : WindowsMode;
    } else {
        QString mode = s.value(QLatin1String("ScrollBar/HorizontalArrowMode"), QLatin1String("Covered")).toString();
        if (mode == QLatin1String("Left")) {
            horizontalArrowMode = NextMode;
        } else if (mode == QLatin1String("Right")) {
            horizontalArrowMode = PlatinumMode;
        } else if (mode == QLatin1String("RightLeft") || mode == QLatin1String("Right/Left")) {
            horizontalArrowMode = WindowsMode;
        } else if (mode == QLatin1String("KDEMode")) {
            horizontalArrowMode = KDEMode;
        } else if (mode == QLatin1String("Covered")) {
            horizontalArrowMode = SkulptureMode;
        } else if (mode == QLatin1String("None")) {
            horizontalArrowMode = NoArrowsMode;
        } else {
            horizontalArrowMode = SkulptureMode;
        }
    }
    hideShortcutUnderlines = s.value(QLatin1String("General/HideShortcutUnderlines"), hideShortcutUnderlines).toBool();
    makeDisabledWidgetsTransparent = s.value(QLatin1String("General/MakeDisabledWidgetsTransparent"), makeDisabledWidgetsTransparent).toBool();
    transparentPlacesPanel = s.value(QLatin1String("Views/TransparentPlacesPanel"), transparentPlacesPanel).toBool();
    forceSpacingAndMargins = s.value(QLatin1String("Layout/ForceSpacingAndMargins"), forceSpacingAndMargins).toBool();
    useIconColumnForCheckIndicators = s.value(QLatin1String("Menus/UseIconColumnForCheckIndicators"), useIconColumnForCheckIndicators).toBool();
    useSelectionColorForCheckedIndicators = s.value(QLatin1String("General/UseSelectionColorForCheckedIndicators"), useSelectionColorForCheckedIndicators).toBool();
    useSelectionColorForSelectedMenuItems = s.value(QLatin1String("Menus/UseSelectionColorForSelectedMenuItems"), useSelectionColorForSelectedMenuItems).toBool();
    useSingleClickToActivateItems = s.value(QLatin1String("General/UseSingleClickToActivateItems"), useSingleClickToActivateItems).toBool();

    dialogMargins = s.value(QLatin1String("Layout/DialogMargins"), dialogMargins).toInt();
    horizontalSpacing = s.value(QLatin1String("Layout/HorizontalSpacing"), horizontalSpacing).toInt();
    labelSpacing = s.value(QLatin1String("Layout/LabelSpacing"), labelSpacing).toInt();
    menuBarSize = s.value(QLatin1String("Layout/MenuBarSize"), menuBarSize).toInt();
    menuItemSize = s.value(QLatin1String("Layout/MenuItemSize"), menuItemSize).toInt();
    pushButtonSize = s.value(QLatin1String("Layout/PushButtonSize"), pushButtonSize).toInt();
    scrollBarSize = s.value(QLatin1String("Layout/ScrollBarSize"), scrollBarSize).toInt();
    scrollBarLength = s.value(QLatin1String("Layout/ScrollBarLength"), scrollBarLength).toInt();
    sliderSize = s.value(QLatin1String("Layout/SliderSize"), sliderSize).toInt();
    sliderLength = s.value(QLatin1String("Layout/SliderLength"), sliderLength).toInt();
    tabBarSize = s.value(QLatin1String("Layout/TabBarSize"), tabBarSize).toInt();
    toolButtonSize = s.value(QLatin1String("Layout/ToolButtonSize"), toolButtonSize).toInt();
    verticalSpacing = s.value(QLatin1String("Layout/VerticalSpacing"), verticalSpacing).toInt();
    widgetMargins = s.value(QLatin1String("Layout/WidgetMargins"), widgetMargins).toInt();
    widgetSize = s.value(QLatin1String("Layout/WidgetSize"), widgetSize).toInt();
    textShift = s.value(QLatin1String("General/TextShift"), textShift).toInt();

    buttonGradient = s.value(QLatin1String("General/ButtonGradientIntensity"), buttonGradient).toInt();
    buttonRoundness = s.value(QLatin1String("General/ButtonRoundness"), buttonRoundness).toInt();

    passwordCharacters = s.value(QLatin1String("General/PasswordCharacters"), passwordCharacters).toString();
    styleSheetFileName = s.value(QLatin1String("General/StyleSheetFileName"), QString()).toString();
    textCursorWidth = s.value(QLatin1String("General/TextCursorWidth"), (double) textCursorWidth).toDouble();

    subMenuDelay = s.value(QLatin1String("Menus/SubMenuDelay"), subMenuDelay).toInt();

    // apply defaults
    if (widgetSize < 0) {
        widgetSize = 2;
    }
    if (pushButtonSize < 0) {
        pushButtonSize = 2;
    }
    if (tabBarSize < 0) {
        tabBarSize = 2;
    }
    if (menuItemSize < 0) {
        menuItemSize = 2;
    }
}


int SkulptureStyle::skulpturePrivateMethod(SkulptureStyle::SkulpturePrivateMethod id, void *data)
{
    switch (id) {
        case SPM_SupportedMethods: {
            return SPM_SetSettingsFileName;
        }
        case SPM_SetSettingsFileName: {
            SkMethodDataSetSettingsFileName *md = (SkMethodDataSetSettingsFileName *) data;
            if (md && md->version >= 1) {
                QSettings s(md->fileName, QSettings::IniFormat);
                if (s.status() == QSettings::NoError) {
                    d->readSettings(s);
                    return 1;
                }
            }
            return 0;
        }
        default:
            return 0;
    }
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

void paintCommandButtonPanel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget);
void paintPushButtonBevel(QPainter *painter, const QStyleOptionButton *option, const QWidget *widget, const QStyle *style);
void paintTabWidgetFrame(QPainter *painter, const QStyleOptionTabWidgetFrame *option, const QWidget *widget);
void paintIndicatorCheckBox(QPainter *painter, const QStyleOptionButton *option);
void paintIndicatorItemViewItemCheck(QPainter *painter, const QStyleOption *option);
void paintQ3CheckListIndicator(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget, const QStyle *style);
void paintQ3CheckListExclusiveIndicator(QPainter *painter, const QStyleOptionQ3ListView *option, const QWidget *widget, const QStyle *style);
void paintIndicatorRadioButton(QPainter *painter, const QStyleOptionButton *option);
void paintIndicatorSpinDown(QPainter *painter, const QStyleOption *option);
void paintIndicatorSpinUp(QPainter *painter, const QStyleOption *option);
void paintIndicatorSpinMinus(QPainter *painter, const QStyleOption *option);
void paintIndicatorSpinPlus(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowDown(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowLeft(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowRight(QPainter *painter, const QStyleOption *option);
void paintIndicatorArrowUp(QPainter *painter, const QStyleOption *option);
void paintHeaderSortIndicator(QPainter *painter, const QStyleOptionHeader *option);
void paintStyledFrame(QPainter *painter, const QStyleOptionFrame *frame, const QWidget *widget, const QStyle *style);
void paintFrameLineEdit(QPainter *painter, const QStyleOptionFrame *frame);
void paintPanelLineEdit(QPainter *painter, const QStyleOptionFrame *frame, const QWidget *widget, const QStyle *style);
void paintFrameDockWidget(QPainter *painter, const QStyleOptionFrame *frame);
void paintFrameWindow(QPainter *painter, const QStyleOptionFrame *frame);
void paintToolBarSeparator(QPainter *painter, const QStyleOptionToolBar *option);
void paintToolBarHandle(QPainter *painter, const QStyleOptionToolBar *option);
void paintScrollArea(QPainter *painter, const QStyleOption *option);
void paintPanelToolBar(QPainter *painter, const QStyleOptionToolBar *option);
void paintIndicatorMenuCheckMark(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style);
void paintFrameGroupBox(QPainter *painter, const QStyleOptionFrame *option);
void paintFrameFocusRect(QPainter *painter, const QStyleOptionFocusRect *option, const QWidget *widget);
void paintPanelButtonTool(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style);
void paintSizeGrip(QPainter *painter, const QStyleOption *option);
void paintScrollAreaCorner(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
void paintPanelItemViewItem(QPainter *painter, const QStyleOptionViewItemV4 *option, const QWidget *widget, const QStyle *style);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
void paintIndicatorTabClose(QPainter *painter, const QStyleOption *option, const QWidget *widget, const QStyle *style);
#endif

void paintMenuBarEmptyArea(QPainter *painter, const QStyleOption *option);
void paintPanelMenuBar(QPainter *painter, const QStyleOptionFrame *frame);
void paintMenuBarItem(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style);
void paintFrameMenu(QPainter *painter, const QStyleOptionFrame *frame);
void paintMenuItem(QPainter *painter, const QStyleOptionMenuItem *option, const QWidget *widget, const QStyle *style);

void paintTabBarTabShape(QPainter *painter, const QStyleOptionTab *option, const QWidget *widget, const QStyle *style);
void paintTabBarTabLabel(QPainter *painter, const QStyleOptionTab *option, const QWidget *widget, const QStyle *style);
void paintFrameTabBarBase(QPainter *painter, const QStyleOptionTabBarBase *option);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBoxV2 *option);
void paintToolBoxTabLabel(QPainter *painter, const QStyleOptionToolBox *option, const QWidget *widget, const QStyle *style);
#else
void paintToolBoxTabShape(QPainter *painter, const QStyleOptionToolBox *option);
#endif
void paintHeaderEmptyArea(QPainter *painter, const QStyleOption *option);
void paintHeaderSection(QPainter *painter, const QStyleOptionHeader *option, const QWidget *widget, const QStyle *style);
void paintHeaderLabel(QPainter *painter, const QStyleOptionHeader *option, const QWidget *widget, const QStyle *style);
void paintIndicatorBranch(QPainter *painter, const QStyleOption *option);

void paintScrollBarSlider(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarAddLine(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarSubLine(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarFirst(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarLast(QPainter *painter, const QStyleOptionSlider *option);
void paintScrollBarPage(QPainter *painter, const QStyleOptionSlider *option);
void paintProgressBarGroove(QPainter *painter, const QStyleOptionProgressBar *option);
void paintProgressBarContents(QPainter *painter, const QStyleOptionProgressBarV2 *option, const QWidget *widget, const QStyle *style);
void paintProgressBarLabel(QPainter *painter, const QStyleOptionProgressBarV2 *option, const QWidget *widget, const QStyle *style);
void paintSplitter(QPainter *painter, const QStyleOption *option);
void paintDockWidgetTitle(QPainter *painter, const QStyleOptionDockWidget *option, const QWidget *widget, const QStyle *style);
void paintRubberBand(QPainter *paint, const QStyleOptionRubberBand *option);
void paintComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *option, const QWidget *widget, const QStyle *style);
void paintToolButtonLabel(QPainter *painter, const QStyleOptionToolButton *option, const QWidget *widget, const QStyle *style);


/*-----------------------------------------------------------------------*/

SkulptureStyle::Private::Private()
{
	init();
}


SkulptureStyle::Private::~Private()
{
	delete shortcut_handler;
	delete settings;
}


void SkulptureStyle::Private::init()
{
	shortcut_handler = new ShortcutHandler(this);
	timer = 0;
	updatingShadows = false;
        oldEdit = 0;
#if 0
	settings = new QSettings(QSettings::IniFormat,
		QSettings::UserScope,
		QLatin1String("SkulptureStyle"),
		QLatin1String(""));
#else
	settings = 0;
#endif

	QSettings s(QSettings::IniFormat, QSettings::UserScope, QLatin1String("SkulptureStyle"), QLatin1String(""));
	readSettings(s);

	register_draw_entries();
}


void SkulptureStyle::Private::register_draw_entries()
{
	for (uint i = 0; i < sizeof(draw_primitive_entry) / sizeof(Private::DrawElementEntry); ++i) {
		draw_primitive_entry[i].func = 0;
	}
	for (uint i = 0; i < sizeof(draw_element_entry) / sizeof(Private::DrawElementEntry); ++i) {
		draw_element_entry[i].func = 0;
	}

        /* entries are stricly sorted in Qt order for future lookup table */

#define register_primitive(pe, f, so) draw_primitive_entry[QStyle::PE_ ## pe].type = QStyleOption::SO_ ## so; draw_primitive_entry[QStyle::PE_ ## pe].func = (drawElementFunc *) paint ## f;

        /* PRIMITIVE ELEMENT */
// Qt 3.x compatibility
//###	register_primitive(Q3CheckListController, Nothing, Default);
	register_primitive(Q3CheckListExclusiveIndicator, Q3CheckListExclusiveIndicator, Default);
	register_primitive(Q3CheckListIndicator, Q3CheckListIndicator, Default);
	register_primitive(Q3DockWindowSeparator, ToolBarSeparator, Default);
//###	register_primitive(Q3Separator, Q3Separator, Default);
// Qt 4.0 Frames
	register_primitive(Frame, StyledFrame, Frame);
	register_primitive(FrameDefaultButton, Nothing, Button);
	register_primitive(FrameDockWidget, FrameDockWidget, Frame);
	register_primitive(FrameFocusRect, FrameFocusRect, FocusRect);
	register_primitive(FrameGroupBox, FrameGroupBox, Frame);
	register_primitive(FrameLineEdit, FrameLineEdit, Frame);
	register_primitive(FrameMenu, FrameMenu, Default); // ### Qt 4.3 calls FrameMenu with SO_ToolBar for a toolbar
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
	register_primitive(FrameStatusBarItem, Nothing, Default);
#else
	register_primitive(FrameStatusBar, Nothing, Default);
#endif
	register_primitive(FrameTabWidget, TabWidgetFrame, TabWidgetFrame);
	register_primitive(FrameWindow, FrameWindow, Frame);
	register_primitive(FrameButtonBevel, PanelButtonTool, Default);
	register_primitive(FrameButtonTool, PanelButtonTool, Default);
	register_primitive(FrameTabBarBase, FrameTabBarBase, TabBarBase);
// Qt 4.0 Panels
	register_primitive(PanelButtonCommand, CommandButtonPanel, Button);
	register_primitive(PanelButtonBevel, PanelButtonTool, Default);
	register_primitive(PanelButtonTool, PanelButtonTool, Default);
	register_primitive(PanelMenuBar, PanelMenuBar, Frame);
	register_primitive(PanelToolBar, PanelToolBar, Frame);
	register_primitive(PanelLineEdit, PanelLineEdit, Frame);
// Qt 4.0 Indicators
	register_primitive(IndicatorArrowDown, IndicatorArrowDown, Default);
	register_primitive(IndicatorArrowLeft, IndicatorArrowLeft, Default);
	register_primitive(IndicatorArrowRight, IndicatorArrowRight, Default);
	register_primitive(IndicatorArrowUp, IndicatorArrowUp, Default);
	register_primitive(IndicatorBranch, IndicatorBranch, Default);
	register_primitive(IndicatorButtonDropDown, PanelButtonTool, Default);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
	register_primitive(IndicatorItemViewItemCheck, IndicatorItemViewItemCheck, Default);
#else
	register_primitive(IndicatorViewItemCheck, IndicatorItemViewItemCheck, Default);
#endif
	register_primitive(IndicatorCheckBox, IndicatorCheckBox, Button);
	register_primitive(IndicatorDockWidgetResizeHandle, Splitter, Default);
	register_primitive(IndicatorHeaderArrow, HeaderSortIndicator, Header);
	register_primitive(IndicatorMenuCheckMark, IndicatorMenuCheckMark, MenuItem);
//	register_primitive(IndicatorProgressChunk, , );
	register_primitive(IndicatorRadioButton, IndicatorRadioButton, Button);
	register_primitive(IndicatorSpinDown, IndicatorSpinDown, Default);
	register_primitive(IndicatorSpinMinus, IndicatorSpinMinus, Default);
	register_primitive(IndicatorSpinPlus, IndicatorSpinPlus, Default);
	register_primitive(IndicatorSpinUp, IndicatorSpinUp, Default);
	register_primitive(IndicatorToolBarHandle, ToolBarHandle, ToolBar);
	register_primitive(IndicatorToolBarSeparator, ToolBarSeparator, Default);
//	register_primitive(PanelTipLabel, , );
//	register_primitive(IndicatorTabTear, , );
// Qt 4.2 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
	register_primitive(PanelScrollAreaCorner, ScrollAreaCorner, Default);
// ###	register_primitive(Widget, , );
#endif
// Qt 4.3 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
// TODO register_primitive(IndicatorColumnViewArrow, , );
#endif
// Qt 4.4 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//	register_primitive(IndicatorItemViewItemDrop, , );
	register_primitive(PanelItemViewItem, PanelItemViewItem, ViewItem);
//	register_primitive(PanelItemViewRow, , );
//	register_primitive(PanelStatusBar, , );
#endif
// Qt 4.5 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
	register_primitive(IndicatorTabClose, IndicatorTabClose, Default);
//      register_primitive(PanelMenu, , );
#endif

#define register_element(ce, f, so) draw_element_entry[QStyle::CE_ ## ce].type = QStyleOption::SO_ ## so; draw_element_entry[QStyle::CE_ ## ce].func = (drawElementFunc *) paint ## f;

	/* CONTROL ELEMENT */
// Qt 4.0 Buttons
//	register_element(PushButton, , );
	register_element(PushButtonBevel, PushButtonBevel, Button);
//	register_element(PushButtonLabel, , );
//	register_element(CheckBox, , );
//	register_element(CheckBoxLabel, , );
//	register_element(RadioButton, , );
//	register_element(RadioButtonLabel, , );
// Qt 4.0 Controls
//	register_element(TabBarTab, , );
	register_element(TabBarTabShape, TabBarTabShape, Tab);
	register_element(TabBarTabLabel, TabBarTabLabel, Tab);
//	register_element(ProgressBar, , ProgressBar);
	register_element(ProgressBarGroove, ProgressBarGroove, ProgressBar);
	register_element(ProgressBarContents, ProgressBarContents, ProgressBar);
	register_element(ProgressBarLabel, ProgressBarLabel, ProgressBar);
// Qt 4.0 Menus
	register_element(MenuItem, MenuItem, MenuItem);
//	register_element(MenuScroller, , );
//	register_element(MenuVMargin, , );
//	register_element(MenuHMargin, , );
//	register_element(MenuTearoff, , );
	register_element(MenuEmptyArea, Nothing, Default);
	register_element(MenuBarItem, MenuBarItem, MenuItem);
	register_element(MenuBarEmptyArea, MenuBarEmptyArea, Default);
// Qt 4.0 more Controls
	register_element(ToolButtonLabel, ToolButtonLabel, ToolButton);
//	register_element(Header, , );
	register_element(HeaderSection, HeaderSection, Header);
	register_element(HeaderLabel, HeaderLabel, Header);
//	register_element(Q3DockWindowEmptyArea, , );
//	register_element(ToolBoxTab, , );
        register_element(SizeGrip, SizeGrip, Default);
	register_element(Splitter, Splitter, Default);
	register_element(RubberBand, RubberBand, RubberBand);
	register_element(DockWidgetTitle, DockWidgetTitle, DockWidget);
// Qt 4.0 ScrollBar
	register_element(ScrollBarAddLine, ScrollBarAddLine, Slider);
	register_element(ScrollBarSubLine, ScrollBarSubLine, Slider);
	register_element(ScrollBarAddPage, ScrollBarPage, Slider);
	register_element(ScrollBarSubPage, ScrollBarPage, Slider);
	register_element(ScrollBarSlider, ScrollBarSlider, Slider);
	register_element(ScrollBarFirst, ScrollBarFirst, Slider);
	register_element(ScrollBarLast, ScrollBarLast, Slider);
// Qt 4.0 even more Controls
//	register_element(FocusFrame, , );
	register_element(ComboBoxLabel, ComboBoxLabel, ComboBox);
// Qt 4.1 additions
//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
	register_element(ToolBar, PanelToolBar, ToolBar);
//#endif
// Qt 4.3 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
	register_element(ToolBoxTabShape, ToolBoxTabShape, ToolBox);
	register_element(ToolBoxTabLabel, ToolBoxTabLabel, ToolBox);
	register_element(HeaderEmptyArea, HeaderEmptyArea, Default);
	register_element(ColumnViewGrip, Splitter, Default);
#endif
// Qt 4.4 additions
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//	register_element(ItemViewItem, , );
#endif
}


/*-----------------------------------------------------------------------*/

//#include "skulpture_p.moc"


