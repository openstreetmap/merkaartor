#ifndef MERKAARTOR_PAINTER_H_
#define MERKAARTOR_PAINTER_H_

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QFont>

#include <QList>
#include <QPair>
#include <QPointF>

class QFont;
class QDomElement;

class LineParameters
{
public:
    bool Draw;
    bool Dashed;
    QColor Color;
    qreal Proportional;
    qreal Fixed;
    qreal DashOn;
    qreal DashOff;
};

class IconParameters
{
public:
    bool Draw;
    QString Name;
    qreal Proportional;
    qreal Fixed;
};

class Painter
{
public:
    typedef enum {NoZoomLimit, GlobalZoom, RegionalZoom, LocalZoom } ZoomType;

    Painter();
    Painter(const Painter& f);
    Painter& operator=(const Painter& F);
    ~Painter();

    virtual void setSelector(const QString& aName);

    bool isFilled() const;
    bool matchesZoom(qreal PixelPerM) const;
    Painter& backgroundActive(bool b);
    Painter& background(QColor Color, qreal Scale, qreal Offset);
    Painter& foregroundActive(bool b);
    Painter& foregroundUseIcon(bool b);
    Painter& foreground(QColor Color, qreal Scale, qreal Offset);
    Painter& foregroundDash(qreal Dash, qreal White);
    Painter& touchupActive(bool b);
    Painter& touchup(QColor Color, qreal Scale, qreal Offset);
    Painter& touchupDash(qreal Dash, qreal White);
    Painter& foregroundFill(QColor FillColor);
    Painter& zoomBoundary(qreal anUnder, qreal anUpper);
    Painter& drawTrafficDirectionMarks(bool b);
    Painter& trackPointIcon(const QString& Filename);
    Painter& fillActive(bool b);
    Painter& iconActive(bool b);
    Painter& setIcon(const QString& Name, qreal Scale, qreal Offset);
    Painter& labelActive(bool b);
    Painter& labelTag(const QString& val);
    Painter& label(QColor Color, qreal Scale, qreal Offset);
    Painter& setLabelFont(const QString& descFont);
    Painter& labelBackgroundActive(bool b);
    Painter& labelBackground(QColor bgColor);
    Painter& labelBackgroundTag(const QString& val);
    Painter& labelHalo(bool b);
    Painter& labelArea(bool b);

    QString userName() const;
    QPair<qreal, qreal> zoomBoundaries() const;
    LineParameters backgroundBoundary() const;
    LineParameters foregroundBoundary() const;
    LineParameters labelBoundary() const;
    IconParameters icon() const;
    void clearForegroundDash();
    LineParameters touchupBoundary() const;
    void clearTouchupDash();
    QColor fillColor() const;
    QColor labelBackgroundColor() const;
    QFont getLabelFont() const;
    QString getLabelTag() const;
    QString getLabelBackgroundTag() const;
    bool getLabelHalo() const;
    bool getLabelArea() const;
    bool getBackgroundInterior() const;
    bool getBackgroundExterior() const;

    QString toXML(QString filename) const;
    static Painter fromXML(const QDomElement& e, QString filename);

public:
    bool ZoomLimitSet;
    qreal ZoomUnder, ZoomUpper;
    bool DrawBackground;
    QColor BackgroundColor;
    qreal BackgroundScale;
    qreal BackgroundOffset;
    bool BackgroundInterior, BackgroundExterior;
    bool DrawForeground;
    QColor ForegroundColor;
    qreal ForegroundScale;
    qreal ForegroundOffset;
    bool ForegroundDashSet;
    qreal ForegroundDash, ForegroundWhite;
    bool DrawTouchup;
    QColor TouchupColor;
    qreal TouchupScale;
    qreal TouchupOffset;
    bool TouchupDashSet;
    qreal TouchupDash, TouchupWhite;
    bool ForegroundFill;
    QColor ForegroundFillFillColor;
    bool ForegroundFillUseIcon;
    bool DrawTrafficDirectionMarks;
    QColor TrafficDirectionMarksColor;
    bool DrawIcon;
    QString IconName;
    qreal IconScale;
    qreal IconOffset;
    bool DrawLabel;
    QString LabelTag;
    QColor LabelColor;
    qreal LabelScale;
    qreal LabelOffset;
    bool DrawLabelBackground;
    QColor LabelBackgroundColor;
    QString LabelBackgroundTag;
    QFont LabelFont;
    bool LabelHalo;
    bool LabelArea;

    QString theSelector;
};

class GlobalPainter
{
public:
    GlobalPainter();
    GlobalPainter(const GlobalPainter& f);
    GlobalPainter& operator=(const GlobalPainter& F);
    ~GlobalPainter();

    GlobalPainter& backgroundActive(bool b);
    GlobalPainter& background(QColor Color);
    bool getDrawBackground() const;
    QColor getBackgroundColor() const;

    GlobalPainter& nodesActive(bool b);
    GlobalPainter& nodes(QColor Color);
    bool getDrawNodes() const;
    QColor getNodesColor() const;

    QString toXML() const;
    static GlobalPainter fromXML(const QDomElement& e);

public:
    bool DrawBackground;
    QColor BackgroundColor;

    bool DrawNodes;
    QColor NodesColor;
    qreal NodesProportional;
    qreal NodesFixed;
};

#endif

