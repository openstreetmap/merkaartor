/*
 * skulpture_p.h - Classical Three-Dimensional Artwork for Qt 4
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

#ifndef SKULPTURE_PRIVATE_H
#define SKULPTURE_PRIVATE_H 1


/*-----------------------------------------------------------------------*/

#include <QtCore/QObject>
class QWidget;

class ShortcutHandler : public QObject
{
	Q_OBJECT

        enum TabletCursorState
        {
            DefaultCursor,
            TabletCursor,
            BlankCursor
        };

	public:
		explicit ShortcutHandler(QObject *parent = 0);
		virtual ~ShortcutHandler();

		bool underlineShortcut(const QWidget *widget) const;

	protected:
		bool eventFilter(QObject *watched, QEvent *event);

	private:
		void init() { }

	private:
		QList<QWidget *> alt_pressed;
                TabletCursorState tabletCursorState;
};


/*-----------------------------------------------------------------------*/

#if (QT_VERSION < 0x040300)
#define QT_VERSION_CHECK(maj, min, rel) ((maj << 16) + (min << 8) + (rel))
#endif


/*-----------------------------------------------------------------------*/

enum ArrowPlacementMode
{
    NoArrowsMode,   // (*)
    SkulptureMode,  // (<*>)
    WindowsMode,    // <(*)>
    KDEMode,    // <(*)<>
    PlatinumMode,   // (*)<>
    NextMode // <>(*)
};


/*-----------------------------------------------------------------------*/

#include "skulpture.h"
#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QtCore/QHash>
#include <QtGui/QStyleOption>
class QPainter;
class QSettings;
class QTextEdit;
class QPlainTextEdit;
class QAbstractScrollArea;
class QLineEdit;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
class QFormLayout;
#endif
class QMenu;

class SkulptureStyle::Private : public QObject
{
	Q_OBJECT

	public:
		Private();
		~Private();

		SkulptureStyle *q;

		void setAnimated(QWidget *widget, bool animated);
		bool isAnimated(QWidget *widget);

	private:
		QList<QWidget *> animations;
		int timer;

	public:
		void readSettings(const QSettings &s);
                void readDominoSettings(const QSettings &s);
                QSettings *settings;

		bool animateProgressBars;
		bool hideShortcutUnderlines;
                bool centerTabs; // from domino
                bool makeDisabledWidgetsTransparent;
                bool transparentPlacesPanel;
                bool forceSpacingAndMargins;
                bool useIconColumnForCheckIndicators;
                bool useSelectionColorForCheckedIndicators;
                bool useSelectionColorForSelectedMenuItems;
                bool useSingleClickToActivateItems;

                ArrowPlacementMode verticalArrowMode;
                ArrowPlacementMode horizontalArrowMode;

                int dialogMargins;
                int horizontalSpacing;
                int labelSpacing;
                int menuBarSize;
                int menuItemSize;
                int pushButtonSize;
                int scrollBarSize;
                int scrollBarLength;
                int sliderSize;
                int sliderLength;
                int tabBarSize;
                int toolButtonSize;
                int verticalSpacing;
                int widgetMargins;
                int widgetSize;
                int textShift;

                int buttonGradient;
                int buttonRoundness;

                int subMenuDelay;

                QString passwordCharacters;
                QString styleSheetFileName;
                qreal textCursorWidth;

		enum Settings {
			Style,
			Metric,
			Icon
		};

		typedef void (drawElementFunc)(
			QPainter *painter,
			const QStyleOption *option,
			const QWidget *widget,
			const QStyle *style
		);

		struct DrawElementEntry
		{
			int type;
			drawElementFunc *func;
		};

		void register_settings(const char *label, ...);

		QSignalMapper mapper;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
		struct DrawElementEntry draw_primitive_entry[QStyle::PE_PanelMenu + 1];
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
		struct DrawElementEntry draw_primitive_entry[QStyle::PE_PanelStatusBar + 1];
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
		struct DrawElementEntry draw_primitive_entry[QStyle::PE_IndicatorColumnViewArrow + 1];
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 2, 0))
		struct DrawElementEntry draw_primitive_entry[QStyle::PE_Widget + 1];
#else
                struct DrawElementEntry draw_primitive_entry[QStyle::PE_IndicatorTabTear + 1];
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
		struct DrawElementEntry draw_element_entry[QStyle::CE_ItemViewItem + 1];
#elif (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
		struct DrawElementEntry draw_element_entry[QStyle::CE_ColumnViewGrip + 1];
#else
		struct DrawElementEntry draw_element_entry[QStyle::CE_ToolBar + 1];
#endif

		ShortcutHandler *shortcut_handler;

		void register_draw_entries();

		void installFrameShadow(QWidget *widget);
		void removeFrameShadow(QWidget *widget);
		void updateFrameShadow(QWidget *widget);

		void updateTextEditMargins(QTextEdit *edit);

                void removeCursorLine(QAbstractScrollArea *edit);
                void updateCursorLine(QAbstractScrollArea *edit, const QRect &cursorRect);
                void paintCursorLine(QAbstractScrollArea *edit);
                void handleCursor(QTextEdit *edit);
                void handleCursor(QPlainTextEdit *edit);

                int verticalTextShift(const QFontMetrics &fontMetrics);
                int textLineHeight(const QStyleOption *option, const QWidget *widget);
                void polishLayout(QLayout *layout);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
                void polishFormLayout(QFormLayout *layout);
#endif

                QList<QPointer<QWidget> > postEventWidgets;
                void addPostEventWidget(QWidget *widget);

		QAbstractScrollArea *oldEdit;
		int oldCursorTop;
		int oldCursorWidth;
		int oldCursorHeight;
		int oldHeight;
		QPalette oldPalette;

		bool updatingShadows;

                QLineEdit *hoverLineEdit;
                QLineEdit *focusLineEdit;
                QRect lineEditHoverCursorRect;

                struct MenuInfo {
                    QPointer<QMenu> menu;
                    QPointer<QAction> lastAction;
                    QPointer<QAction> visibleSubMenu;
                    QPointer<QAction> lastSubMenuAction;
                    int eventCount;
                    QPoint lastPos;
                    int delayTimer;
                };

                QHash<QMenu *, MenuInfo> menuHash;
                bool menuEventFilter(QMenu *menu, QEvent *event);

	protected:
		void timerEvent(QTimerEvent *event);
		bool eventFilter(QObject *watched, QEvent *event);

	protected Q_SLOTS:
		void textEditSourceChanged(QWidget *);
		void updateToolBarOrientation(Qt::Orientation);
                void processPostEventWidgets();

	private:
		void init();
};


/*-----------------------------------------------------------------------*/

class FrameShadow : public QWidget
{
	Q_OBJECT

	public:
		enum ShadowArea { Left, Top, Right, Bottom };

	public:
		explicit FrameShadow(QWidget *parent = 0);
		explicit FrameShadow(ShadowArea area, QWidget *parent = 0);
		virtual ~FrameShadow();

		void setShadowArea(ShadowArea area) { area_ = area; }
		ShadowArea shadowArea() const { return area_; }

		void updateGeometry();

	protected:
		bool event(QEvent *e);
		void paintEvent(QPaintEvent *);

	private:
		void init();

	private:
		ShadowArea area_;
};


/*-----------------------------------------------------------------------*/

class WidgetShadow : public QWidget
{
	Q_OBJECT

	public:
		explicit WidgetShadow(QWidget *parent = 0);

		void setWidget(QWidget *w) { widget_ = w; }
		QWidget *widget() const { return widget_; }

		void updateGeometry();
		void updateZOrder();

	public:
		bool event(QEvent *e);

	private:
		void init();

	private:
		QWidget *widget_;
};


/*-----------------------------------------------------------------------*/

struct SkMethodData
{
	int version;
};


struct SkMethodDataSetSettingsFileName : public SkMethodData
{
	// in version 1
	QString fileName;
};


/*-----------------------------------------------------------------------*/

class QPainterPath;

QColor shaded_color(const QColor &color, int shade);
QColor blend_color(const QColor &c0, const QColor &c1, qreal blend);
QGradient path_edge_gradient(const QRectF &rect, const QStyleOption *option, const QPainterPath &path, const QColor &color2, const QColor &color1);

void paintThinFrame(QPainter *painter, const QRect &rect, const QPalette &palette, int dark, int light, QPalette::ColorRole bgrole = QPalette::Window);

enum RecessedFrame { RF_Small, RF_Large, RF_None };

void paintRecessedFrame(QPainter *painter, const QRect &rect, const QPalette &palette, enum RecessedFrame rf, QPalette::ColorRole bgrole = QPalette::Window);
void paintRecessedFrameShadow(QPainter *painter, const QRect &rect, enum RecessedFrame rf);


/*-----------------------------------------------------------------------*/
// FIXME
#if (QT_VERSION < QT_VERSION_CHECK(4, 3, 0))
#define lighter light
#define darker dark
#endif


/*-----------------------------------------------------------------------*/

#define array_elements(a) (sizeof(a) / sizeof(a[0]))


/*-----------------------------------------------------------------------*/

#endif


