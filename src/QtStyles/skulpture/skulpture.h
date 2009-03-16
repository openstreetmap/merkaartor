/*
 * skulpture.h
 *
 */

#ifndef SKULPTURE_H
#define SKULPTURE_H 1


/*-----------------------------------------------------------------------*/

#include <QtGui/QPlastiqueStyle>

class SkulptureStyle : public QPlastiqueStyle
{
	Q_OBJECT

	typedef QPlastiqueStyle ParentStyle;
//	typedef QCommonStyle ParentStyle;

	public:
		SkulptureStyle();
		virtual ~SkulptureStyle();

		QPalette standardPalette() const;
		void polish(QPalette &palette);

		void polish(QWidget *widget);
		void unpolish(QWidget *widget);
		void polish(QApplication *application);
		void unpolish(QApplication *application);

		void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
		void drawItemText(QPainter * painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
		QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap & pixmap) const;
		QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text ) const;

		int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const;
		int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;
		QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
		QSize sizeFromContents (ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const;
		QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const;
		SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget) const;

		QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const;
		QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option, const QWidget *widget) const;

		void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
		void drawControl(ControlElement control, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
		void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

	public:
		// internal, reserved for future use
		enum SkulpturePrivateMethod {
			SPM_SupportedMethods = 0,
			SPM_SetSettingsFileName = 1,
		};

	public Q_SLOTS:
		int skulpturePrivateMethod(SkulpturePrivateMethod id, void *data = 0);

	protected Q_SLOTS:
//#if (QT_VERSION >= QT_VERSION_CHECK(4, 1, 0))
		QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const;
//#endif
#if (QT_VERSION >= QT_VERSION_CHECK(4, 3, 0))
		int layoutSpacingImplementation(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget) const;
#endif

	private:
		void init();

		class Private;
		Private * const d;
};


/*-----------------------------------------------------------------------*/

#endif


