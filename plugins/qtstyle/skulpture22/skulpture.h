/*
 * Skulpture - Classical Three-Dimensional Artwork for Qt 4
 *
 * Copyright (c) 2007, 2008 Christoph Feck <christoph@maxiom.de>
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

#ifndef SKULPTURE_H
#define SKULPTURE_H 1

#include <QtGui/QCommonStyle>


/*-----------------------------------------------------------------------*/

// Skulpture is a subclass of QCommonStyle in order to
// ease transition to KStyle later

class SkulptureStyle : public QCommonStyle
{
    Q_OBJECT

    typedef QCommonStyle ParentStyle;

    public:
        SkulptureStyle();
        virtual ~SkulptureStyle();

        QPalette standardPalette() const;
        void polish(QPalette &palette);

        void polish(QWidget *widget);
        void unpolish(QWidget *widget);
        void polish(QApplication *application);
        void unpolish(QApplication *application);

        void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment,
                            const QPixmap &pixmap) const;
        void drawItemText(QPainter * painter, const QRect &rectangle, int alignment,
                          const QPalette &palette, bool enabled, const QString &text,
                          QPalette::ColorRole textRole = QPalette::NoRole) const;
        QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap & pixmap) const;
        QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment,
                           bool enabled, const QString &text ) const;

        int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                      QStyleHintReturn *returnData) const;
        int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const;
        QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;
        QSize sizeFromContents (ContentsType type, const QStyleOption *option, const QSize &contentsSize,
                                const QWidget *widget) const;
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                             SubControl subControl, const QWidget *widget) const;
        SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                         const QPoint &position, const QWidget *widget) const;

        QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                    const QStyleOption *option) const;
        QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                               const QWidget *widget) const;

        void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                           const QWidget *widget) const;
        void drawControl(ControlElement control, const QStyleOption *option, QPainter *painter,
                         const QWidget *widget) const;
        void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                                QPainter *painter, const QWidget *widget) const;

    public:
        // internal
        enum SkulpturePrivateMethod {
            SPM_SupportedMethods = 0,
            SPM_SetSettingsFileName = 1
        };

    public Q_SLOTS:
        int skulpturePrivateMethod(SkulpturePrivateMethod id, void *data = 0);

    protected Q_SLOTS:
//#if (QT_VERSION >= 0x040100)
        QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option,
                                         const QWidget *widget) const;
//#endif
#if (QT_VERSION >= 0x040300)
        int layoutSpacingImplementation(QSizePolicy::ControlType control1,
                                        QSizePolicy::ControlType control2, Qt::Orientation orientation,
                                         const QStyleOption *option, const QWidget *widget) const;
#endif

    private:
        void init();

        class Private;
        Private * const d;
};


/*-----------------------------------------------------------------------*/

#endif // SKULPTURE_H


