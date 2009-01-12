/*
 * skulpture_p.h
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
};


/*-----------------------------------------------------------------------*/

#include "skulpture.h"
#include <QtCore/QSignalMapper>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtGui/QStyleOption>
class QPainter;
class QSettings;
class QTextEdit;
class QPlainTextEdit;
class QAbstractScrollArea;

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
		QSet<QWidget *> animations;
		int timer;

	public:
		void readSettings(const QSettings &s);
		QSettings *settings;

		bool animateProgressBars;
		bool allowScrollBarSliderToCoverArrows;
		bool hideShortcutUnderlines;

		enum Settings {
			Style,
			Metric,
			Icon,
		};

		enum DrawElement {
			DE_Primitive = 0xF1000100,
			DE_Element = 0xF2000200,
			DE_Complex = 0xF3000300
		};

		typedef void (drawElementFunc)(
			QPainter *painter,
			const QStyleOption *option,
			int bgrole, int fgrole,	// not in QStyleOption
			void *data, int id,
			const QWidget *widget,
			const QStyle *style
		);

		struct DrawElementEntry
		{
			int type;
			drawElementFunc *func;
			int id;
			void *data;
		};

		void register_settings(const char *label, ...);

		QSignalMapper mapper;

		QHash<int, DrawElementEntry *> draw_hash;

		ShortcutHandler *shortcut_handler;

		void register_draw(DrawElement type, int which, drawElementFunc *func, int option_type = QStyleOption::SO_Default, void *data = 0, int id = 0);

		void installFrameShadow(QWidget *widget);
		void removeFrameShadow(QWidget *widget);
		void updateFrameShadow(QWidget *widget);

		void updateTextEditMargins(QTextEdit *edit);

		void highlightCurrentEditLine(QAbstractScrollArea *edit, const QRect &cursorRect);
		void highlightCurrentEditLine(QTextEdit *edit);
		void highlightCurrentEditLine(QPlainTextEdit *edit);

		QAbstractScrollArea *oldEdit;
		int oldCursorTop;
		int oldCursorWidth;
		int oldCursorHeight;
		int oldHeight;
		QPalette oldPalette;

		bool updatingShadows;

	protected:
		void timerEvent(QTimerEvent *event);
		bool eventFilter(QObject *watched, QEvent *event);

	protected Q_SLOTS:
		void textEditSourceChanged(QWidget *);
		void updateToolBarOrientation(Qt::Orientation);

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
// ### Remove for 4.4

#if (QT_VERSION < QT_VERSION_CHECK(4, 4, 0))
#define PM_TabBar_ScrollButtonOverlap	(QStyle::PixelMetric(int(PM_LayoutVerticalSpacing) + 1))
#define PM_TextCursorWidth			(QStyle::PixelMetric(int(PM_LayoutVerticalSpacing) + 2))
#endif

// ### does not work
//#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
//#if QT_PACKAGEDATE_STR < "2008-03"
//#error If you build with Qt 4.4.0, you have to use a more recent version.
//#endif
//#endif


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

QColor shaded_color(const QColor &color, int shade);
QColor blend_color(const QColor &c0, const QColor &c1, qreal blend);
QGradient path_edge_gradient(const QRectF &rect, const QStyleOption *option, const QPainterPath &path, const QColor &color2, const QColor &color1);

void paintThinFrame(QPainter *painter, const QRect &rect, const QPalette &palette, int dark, int light, QPalette::ColorRole bgrole = QPalette::Window);

enum RecessedFrame { RF_Small, RF_Large, RF_None };

void paintRecessedFrame(QPainter *painter, const QRect &rect, const QPalette &palette, enum RecessedFrame rf, QPalette::ColorRole bgrole = QPalette::Window);
void paintRecessedFrameShadow(QPainter *painter, const QRect &rect, enum RecessedFrame rf);


/*-----------------------------------------------------------------------*/

#endif


