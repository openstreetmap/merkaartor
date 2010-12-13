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
    double Proportional;
    double Fixed;
    double DashOn;
    double DashOff;
};

class IconParameters
{
public:
    bool Draw;
    QString Name;
    double Proportional;
    double Fixed;
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
    bool matchesZoom(double PixelPerM) const;
    Painter& backgroundActive(bool b);
    Painter& background(QColor Color, double Scale, double Offset);
    Painter& foregroundActive(bool b);
    Painter& foregroundUseIcon(bool b);
    Painter& foreground(QColor Color, double Scale, double Offset);
    Painter& foregroundDash(double Dash, double White);
    Painter& touchupActive(bool b);
    Painter& touchup(QColor Color, double Scale, double Offset);
    Painter& touchupDash(double Dash, double White);
    Painter& foregroundFill(QColor FillColor);
    Painter& zoomBoundary(double anUnder, double anUpper);
    Painter& drawTrafficDirectionMarks(bool b);
    Painter& trackPointIcon(const QString& Filename);
    Painter& fillActive(bool b);
    Painter& iconActive(bool b);
    Painter& setIcon(const QString& Name, double Scale, double Offset);
    Painter& labelActive(bool b);
    Painter& labelTag(const QString& val);
    Painter& label(QColor Color, double Scale, double Offset);
    Painter& setLabelFont(const QString& descFont);
    Painter& labelBackgroundActive(bool b);
    Painter& labelBackground(QColor bgColor);
    Painter& labelBackgroundTag(const QString& val);
    Painter& labelHalo(bool b);
    Painter& labelArea(bool b);

    QString userName() const;
    QPair<double, double> zoomBoundaries() const;
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
    double ZoomUnder, ZoomUpper;
    bool DrawBackground;
    QColor BackgroundColor;
    double BackgroundScale;
    double BackgroundOffset;
    bool BackgroundInterior, BackgroundExterior;
    bool DrawForeground;
    QColor ForegroundColor;
    double ForegroundScale;
    double ForegroundOffset;
    bool ForegroundDashSet;
    double ForegroundDash, ForegroundWhite;
    bool DrawTouchup;
    QColor TouchupColor;
    double TouchupScale;
    double TouchupOffset;
    bool TouchupDashSet;
    double TouchupDash, TouchupWhite;
    bool ForegroundFill;
    QColor ForegroundFillFillColor;
    bool ForegroundFillUseIcon;
    bool DrawTrafficDirectionMarks;
    QColor TrafficDirectionMarksColor;
    bool DrawIcon;
    QString IconName;
    double IconScale;
    double IconOffset;
    bool DrawLabel;
    QString LabelTag;
    QColor LabelColor;
    double LabelScale;
    double LabelOffset;
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
    double NodesProportional;
    double NodesFixed;
};

#endif

