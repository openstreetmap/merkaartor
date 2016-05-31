#include "Painter.h"
#include "SvgCache.h"

#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QMatrix>
#include <QDomElement>
#include <QFileInfo>
#include <QDir>

Painter::Painter()
: ZoomLimitSet(false), ZoomUnder(0), ZoomUpper(10e6),
  DrawBackground(false), BackgroundScale(0), BackgroundOffset(3),
  DrawForeground(false), ForegroundScale(0), ForegroundOffset(2),
  ForegroundDashSet(false),
  DrawTouchup(false), TouchupScale(0), TouchupOffset(1),
  TouchupDashSet(false),
  ForegroundFill(false), ForegroundFillUseIcon(false),
  DrawTrafficDirectionMarks(false),
  DrawIcon(false), IconScale(0), IconOffset(0),
  DrawLabel(false), LabelScale(0), LabelOffset(0),
  DrawLabelBackground(false), LabelHalo(false), LabelArea(false),
  BackgroundInterior(false), BackgroundExterior(false)
{
}

Painter::Painter(const Painter& f)
: ZoomLimitSet(f.ZoomLimitSet), ZoomUnder(f.ZoomUnder), ZoomUpper(f.ZoomUpper),
  DrawBackground(f.DrawBackground), BackgroundColor(f.BackgroundColor),
  BackgroundScale(f.BackgroundScale), BackgroundOffset(f.BackgroundOffset),
  DrawForeground(f.DrawForeground), ForegroundColor(f.ForegroundColor),
  ForegroundScale(f.ForegroundScale), ForegroundOffset(f.ForegroundOffset),
  ForegroundDashSet(f.ForegroundDashSet), ForegroundDash(f.ForegroundDash), ForegroundWhite(f.ForegroundWhite),
  DrawTouchup(f.DrawTouchup), TouchupColor(f.TouchupColor),
  TouchupScale(f.TouchupScale), TouchupOffset(f.TouchupOffset),
  TouchupDashSet(f.TouchupDashSet),
  TouchupDash(f.TouchupDash), TouchupWhite(f.TouchupWhite),
  ForegroundFill(f.ForegroundFill), ForegroundFillFillColor(f.ForegroundFillFillColor), ForegroundFillUseIcon(f.ForegroundFillUseIcon),
  DrawTrafficDirectionMarks(f.DrawTrafficDirectionMarks), TrafficDirectionMarksColor(f.TrafficDirectionMarksColor),
  DrawIcon(f.DrawIcon), IconName(f.IconName), IconScale(f.IconScale), IconOffset(f.IconOffset),
  DrawLabel(f.DrawLabel), LabelTag(f.LabelTag), LabelColor(f.LabelColor), LabelScale(f.LabelScale), LabelOffset(f.LabelOffset),
  DrawLabelBackground(f.DrawLabelBackground), LabelBackgroundColor(f.LabelBackgroundColor), LabelBackgroundTag(f.LabelBackgroundTag),
  LabelFont(f.LabelFont), LabelHalo(f.LabelHalo), LabelArea(f.LabelArea),
  BackgroundInterior(f.BackgroundInterior), BackgroundExterior(f.BackgroundExterior),
  theSelector(f.theSelector)
{
}

Painter& Painter::operator=(const Painter& f)
{
    if (&f == this) return *this;

    ZoomLimitSet = f.ZoomLimitSet;
    ZoomUnder = f.ZoomUnder;
    ZoomUpper = f.ZoomUpper;
    DrawBackground = f.DrawBackground;
    BackgroundColor = f.BackgroundColor;
    BackgroundScale = f.BackgroundScale;
    BackgroundOffset = f.BackgroundOffset;
    DrawForeground = f.DrawForeground;
    ForegroundColor = f.ForegroundColor;
    ForegroundScale = f.ForegroundScale;
    ForegroundOffset = f.ForegroundOffset;
    ForegroundDashSet = f.ForegroundDashSet ;
    ForegroundDash = f.ForegroundDash;
    ForegroundWhite = f.ForegroundWhite;
    DrawTouchup = f.DrawTouchup;
    TouchupColor = f.TouchupColor;
    TouchupScale = f.TouchupScale;
    TouchupOffset = f.TouchupOffset;
    TouchupDashSet = f.TouchupDashSet;
    TouchupDash = f.TouchupDash;
    TouchupWhite = f.TouchupWhite;
    ForegroundFill = f.ForegroundFill;
    ForegroundFillFillColor = f.ForegroundFillFillColor;
    ForegroundFillUseIcon = f.ForegroundFillUseIcon;
    DrawTrafficDirectionMarks = f.DrawTrafficDirectionMarks;
    TrafficDirectionMarksColor = f.TrafficDirectionMarksColor;
    DrawIcon = f.DrawIcon;
    IconName = f.IconName;
    IconScale = f.IconScale;
    IconOffset = f.IconOffset;
    DrawLabel = f.DrawLabel;
    LabelColor = f.LabelColor;
    LabelScale = f.LabelScale;
    LabelOffset = f.LabelOffset;
    DrawLabelBackground = f.DrawLabelBackground;
    LabelBackgroundColor = f.LabelBackgroundColor;
    LabelFont = f.LabelFont;
    LabelTag = f.LabelTag;
    LabelBackgroundTag = f.LabelBackgroundTag;
    LabelHalo = f.LabelHalo;
    LabelArea = f.LabelArea;
    theSelector = f.theSelector;
    BackgroundInterior = f.BackgroundInterior;
    BackgroundExterior = f.BackgroundExterior;

    return *this;
}

Painter::~Painter()
{
}

QString paddedHexa(int i)
{
    QString r=QString::number(i,16);
    if (r.length() < 2)
        r = "0"+r;
    return r;
}

QString asXML(const QColor& c)
{
    return "#"+paddedHexa(c.red())+paddedHexa(c.green())+paddedHexa(c.blue())+paddedHexa(c.alpha());
}

QString colorAsXML(const QString& name, const QColor& c)
{
    return
        name+"Color=\""+asXML(c)+"\"\n";
}

QString boundaryAsXML(const QString& name, const QColor& c, qreal Scale, qreal Offset)
{
    return
        name+"Color=\""+asXML(c)+"\" "+name+"Scale=\""+QString::number(Scale)+"\" "+name+"Offset=\""+QString::number(Offset)+"\"\n";
}

QString iconAsXML(const QString& name, const QString& fn, qreal Scale, qreal Offset)
{
    return
        name+"=\""+fn+"\" "+name+"Scale=\""+QString::number(Scale)+"\" "+name+"Offset=\""+QString::number(Offset)+"\"\n";
}

QColor toColor(const QString& s)
{
    return
        QColor(
            s.mid(1,2).toInt(0,16),
            s.mid(3,2).toInt(0,16),
            s.mid(5,2).toInt(0,16),
            s.mid(7,2).toInt(0,16));
}


QString Painter::toXML(QString filename) const
{
    QString r;
    r += "<painter\n";
    if (ZoomLimitSet)
        r += " zoomUnder=\""+QString::number(ZoomUnder)+"\" zoomUpper=\""+QString::number(ZoomUpper)+"\"\n";
    if (DrawBackground)
        r += " " + boundaryAsXML("background",BackgroundColor, BackgroundScale, BackgroundOffset);
    if (BackgroundInterior)
        r += " interior=\"yes\"";
    if (BackgroundExterior)
        r += " exterior=\"yes\"";
    if (DrawForeground)
        r += " " + boundaryAsXML("foreground",ForegroundColor, ForegroundScale, ForegroundOffset);
    if (ForegroundDashSet && DrawForeground)
        r += " foregroundDashDown=\""+QString::number(ForegroundDash)+"\" foregroundDashUp=\""+QString::number(ForegroundWhite)+"\"\n";
    if (DrawTouchup)
        r += " " + boundaryAsXML("touchup",TouchupColor, TouchupScale, TouchupOffset);
    if (TouchupDashSet && DrawTouchup)
        r += " touchupDashDown=\""+QString::number(TouchupDash)+"\" touchupDashUp=\""+QString::number(TouchupWhite)+"\"\n";
    if (ForegroundFill)
        r += " fillColor=\""+::asXML(ForegroundFillFillColor)+"\"\n";
    if (ForegroundFillUseIcon)
        r += " fillWithIcon=\"yes\"";
    if (!IconName.isEmpty() && DrawIcon)
    {
        QString iconFilename;
        if (!IconName.startsWith(':')) {
            iconFilename = QFileInfo(filename).absoluteDir().relativeFilePath(QFileInfo(IconName).absoluteFilePath());
        } else {
            iconFilename = IconName;
        }
        r += " " + iconAsXML("icon",iconFilename, IconScale, IconOffset);
    }
    if (DrawTrafficDirectionMarks)
        r += " drawTrafficDirectionMarks=\"yes\" trafficDirectionMarksColor=\"" + ::asXML(TrafficDirectionMarksColor) +"\"\n";
    if (DrawLabel) {
        r += " " + boundaryAsXML("label",LabelColor, LabelScale, LabelOffset);
        r += " labelFont=\"" + LabelFont.toString() + "\"";
        r += " labelTag=\"" + LabelTag + "\"";
        if (LabelHalo)
            r += " labelHalo=\"yes\"";
        if (LabelArea)
            r += " labelArea=\"yes\"";
    }
    if (DrawLabelBackground) {
        r += " labelBackgroundColor=\""+::asXML(LabelBackgroundColor)+"\"";
        r += " labelBackgroundTag=\""+ LabelBackgroundTag +"\"\n";
    }
    r += ">\n";

    if (!theSelector.isEmpty())
        r += "  <selector expr=\""+theSelector+"\"/>\n";

    r += "</painter>\n";
    return r;
}

Painter Painter::fromXML(const QDomElement& e, QString filename)
{
    Painter FP;

    if (e.hasAttribute("zoomUnder") || e.hasAttribute("zoomUpper"))
        FP.zoomBoundary(e.attribute("zoomUnder","0").toDouble(),e.attribute("zoomUpper","10e6").toDouble());
    if (e.hasAttribute("foregroundColor"))
    {
        FP.foreground(
            toColor(e.attribute("foregroundColor")),e.attribute("foregroundScale").toDouble(),e.attribute("foregroundOffset").toDouble());
        if (e.hasAttribute("foregroundDashDown"))
            FP.foregroundDash(e.attribute("foregroundDashDown").toDouble(),e.attribute("foregroundDashUp").toDouble());
    }
    if (e.hasAttribute("fillWithIcon"))
        FP.foregroundUseIcon(e.attribute("fillWithIcon") == "yes");
    if (e.hasAttribute("backgroundColor"))
        FP.background(
            toColor(e.attribute("backgroundColor")),e.attribute("backgroundScale").toDouble(),e.attribute("backgroundOffset").toDouble());
    if (e.attribute("interior") == "yes")
        FP.BackgroundInterior = true;
    if (e.attribute("exterior") == "yes")
        FP.BackgroundExterior = true;
    if (e.hasAttribute("touchupColor"))
    {
        FP.touchup(
            toColor(e.attribute("touchupColor")),e.attribute("touchupScale").toDouble(),e.attribute("touchupOffset").toDouble());
        if (e.hasAttribute("touchupDashDown"))
            FP.touchupDash(e.attribute("touchupDashDown").toDouble(),e.attribute("touchupDashUp").toDouble());
    }
    if (e.hasAttribute("fillColor"))
        FP.foregroundFill(toColor(e.attribute("fillColor")));
    if (e.hasAttribute("icon"))
    {
        QString iconFilename = e.attribute("icon");
        if (!QFileInfo(iconFilename).isAbsolute())
            iconFilename = QFileInfo(filename).absolutePath().append("/").append(iconFilename);
        FP.setIcon(iconFilename,e.attribute("iconScale", "0.0").toDouble(),e.attribute("iconOffset", "0.0").toDouble());
    }
    if (e.attribute("drawTrafficDirectionMarks") == "yes")
        FP.drawTrafficDirectionMarks(true);
    if (e.hasAttribute("trafficDirectionMarksColor"))
        FP.TrafficDirectionMarksColor = toColor((e.attribute("trafficDirectionMarksColor")));
    if (e.hasAttribute("labelColor"))
    {
        FP.label(
            toColor(e.attribute("labelColor")),e.attribute("labelScale").toDouble(),e.attribute("labelOffset").toDouble());
        FP.setLabelFont(e.attribute("labelFont"));
        FP.labelTag(e.attribute("labelTag"));
        if (e.hasAttribute("labelHalo"))
            FP.labelHalo((e.attribute("labelHalo") == "yes"));
        if (e.hasAttribute("labelArea"))
            FP.labelArea((e.attribute("labelArea") == "yes"));
        if (e.hasAttribute("labelBackgroundColor"))
            FP.labelBackground(toColor(e.attribute("labelBackgroundColor")));
        if (e.hasAttribute("labelBackgroundTag"))
            FP.labelBackgroundTag(e.attribute("labelBackgroundTag"));
    }

    QDomNode n = e.firstChild();
    QList<QPair<QString,QString> > Pairs;
    while (!n.isNull())
    {
        if (n.isElement())
        {
            QDomElement t = n.toElement();
            if (t.tagName() == "selector")
            {
                if (!t.attribute("key").isEmpty())
                    Pairs.push_back(qMakePair(t.attribute("key"),t.attribute("value")));
                else
                {
                    FP.setSelector(t.attribute("expr"));
                    return FP;
                }
            }
        }
        n = n.nextSibling();
    }
    if (Pairs.size() == 1)
        FP.setSelector(Pairs[0].first+"="+Pairs[0].second);
    else if (Pairs.size())
    {
        bool Same = true;
        for (int i=1; i<Pairs.size(); ++i)
            if (Pairs[0].first != Pairs[i].first)
                Same = false;
        if (Same)
        {
            QStringList Options;
            for (int i=0; i<Pairs.size(); ++i)
                Options.push_back(Pairs[i].second);
            FP.setSelector("["+ Pairs[0].first +"] isoneof ("+ Options.join(",") + ")");
        }
        else
        {
            QStringList Options;
            for (int i=0; i<Pairs.size(); ++i)
                Options.push_back(Pairs[i].first+"="+Pairs[i].second);
            FP.setSelector(Options.join(" or "));
        }
    }

    return FP;
}

QString Painter::userName() const
{
    if (!theSelector.isEmpty())
        return theSelector;
    return "Unnamed";
}

QPair<qreal, qreal> Painter::zoomBoundaries() const
{
    if (ZoomLimitSet)
        return qMakePair(ZoomUnder,ZoomUpper);
    return qMakePair((qreal)0.0,(qreal)0.0);
}

QColor Painter::fillColor() const
{
    if (!ForegroundFill)
        return QColor();
    return ForegroundFillFillColor;
}

bool Painter::isFilled() const
{
    return ForegroundFill;
}

Painter& Painter::drawTrafficDirectionMarks(bool b)
{
    DrawTrafficDirectionMarks = b;
    return *this;
}

Painter& Painter::zoomBoundary(qreal anUnder, qreal anUpper)
{
    ZoomLimitSet = true;
    ZoomUnder = anUnder;
    ZoomUpper = anUpper;
    return *this;
}

Painter& Painter::fillActive(bool b)
{
    ForegroundFill = b;
    if (ForegroundFill && !ForegroundFillFillColor.isValid())
        ForegroundFillFillColor.setRgb(0,0,0);
    return *this;
}

Painter& Painter::foregroundFill(QColor FillColor)
{
    ForegroundFill = true;
    ForegroundFillFillColor = FillColor;
    return *this;
}

Painter& Painter::backgroundActive(bool b)
{
    DrawBackground = b;
    return *this;
}

Painter& Painter::background(QColor Color, qreal Scale, qreal Offset)
{
    DrawBackground = true;
    BackgroundColor = Color;
    BackgroundScale = Scale;
    BackgroundOffset = Offset;
    return *this;
}

LineParameters Painter::backgroundBoundary() const
{
    LineParameters P;
    P.Draw = DrawBackground;
    P.Color = BackgroundColor;
    P.Proportional = BackgroundScale;
    P.Fixed = BackgroundOffset;
    P.Dashed = false;
    P.DashOn = P.DashOff = 0;
    return P;
}

Painter& Painter::touchupActive(bool b)
{
    DrawTouchup = b;
    return *this;
}

Painter& Painter::touchupDash(qreal Dash, qreal White)
{
    TouchupDashSet = true;
    TouchupDash = Dash;
    TouchupWhite = White;
    return *this;
}

Painter& Painter::touchup(QColor Color, qreal Scale, qreal Offset)
{
    DrawTouchup = true;
    TouchupColor = Color;
    TouchupScale = Scale;
    TouchupOffset = Offset;
    TouchupDashSet = false;
    return *this;
}

Painter& Painter::foregroundActive(bool b)
{
    DrawForeground = b;
    return *this;
}

Painter& Painter::foregroundDash(qreal Dash, qreal White)
{
    ForegroundDashSet = true;
    ForegroundDash = Dash;
    ForegroundWhite = White;
    return *this;
}

Painter& Painter::foreground(QColor Color, qreal Scale, qreal Offset)
{
    DrawForeground = true;
    ForegroundColor = Color;
    ForegroundScale = Scale;
    ForegroundOffset = Offset;
    ForegroundDashSet = false;
    return *this;
}

Painter& Painter::foregroundUseIcon(bool b)
{
    ForegroundFillUseIcon = b;
    return *this;
}

void Painter::clearForegroundDash()
{
    ForegroundDashSet = false;
}

Painter& Painter::labelActive(bool b)
{
    DrawLabel = b;
    return *this;
}

Painter& Painter::labelHalo(bool b)
{
    LabelHalo = b;
    return *this;
}

Painter& Painter::labelArea(bool b)
{
    LabelArea = b;
    return *this;
}

Painter& Painter::labelTag(const QString& val)
{
    LabelTag = val;
    return *this;
}

Painter& Painter::labelBackgroundTag(const QString& val)
{
    LabelBackgroundTag = val;
    return *this;
}

Painter& Painter::label(QColor Color, qreal Scale, qreal Offset)
{
    DrawLabel = true;
    LabelColor = Color;
    LabelScale = Scale;
    LabelOffset = Offset;
    return *this;
}

Painter& Painter::labelBackgroundActive(bool b)
{
    DrawLabelBackground = b;
    if (DrawLabelBackground && !LabelBackgroundColor.isValid())
        LabelBackgroundColor.setRgb(0,0,0);
    return *this;
}

Painter& Painter::labelBackground(QColor bgColor)
{
    DrawLabelBackground = true;
    LabelBackgroundColor = bgColor;
    return *this;
}

Painter& Painter::setLabelFont(const QString& descFont)
{
    LabelFont.fromString(descFont);
    return *this;
}

QColor Painter::labelBackgroundColor() const
{
    if (!DrawLabelBackground)
        return QColor();
    return LabelBackgroundColor;
}

QFont Painter::getLabelFont() const
{
    return LabelFont;
}

QString Painter::getLabelTag() const
{
    return LabelTag;
}

bool Painter::getLabelHalo() const
{
    return LabelHalo;
}

bool Painter::getLabelArea() const
{
    return LabelArea;
}

QString Painter::getLabelBackgroundTag() const
{
    return LabelBackgroundTag;
}

bool Painter::getBackgroundInterior() const
{
    return BackgroundInterior;
}

bool Painter::getBackgroundExterior() const
{
    return BackgroundExterior;
}

LineParameters Painter::foregroundBoundary() const
{
    LineParameters P;
    P.Draw = DrawForeground;
    P.Color = ForegroundColor;
    P.Proportional = ForegroundScale;
    P.Fixed = ForegroundOffset;
    P.Dashed = ForegroundDashSet;
    P.DashOn = ForegroundDash;
    P.DashOff = ForegroundWhite;
    if (!P.Dashed)
        P.DashOff = P.DashOn = 0;
    return P;
}

void Painter::clearTouchupDash()
{
    TouchupDashSet = false;
}

LineParameters Painter::touchupBoundary() const
{
    LineParameters P;
    P.Draw = DrawTouchup;
    P.Color = TouchupColor;
    P.Proportional = TouchupScale;
    P.Fixed = TouchupOffset;
    P.Dashed = TouchupDashSet;
    P.DashOn = TouchupDash;
    P.DashOff = TouchupWhite;
    if (!P.Dashed)
        P.DashOff = P.DashOn = 0;
    return P;
}

LineParameters Painter::labelBoundary() const
{
    LineParameters P;
    P.Draw = DrawLabel;
    P.Color = LabelColor;
    P.Proportional = LabelScale;
    P.Fixed = LabelOffset;
    P.Dashed = false;
    if (!P.Dashed)
        P.DashOff = P.DashOn = 0.0;
    return P;
}

Painter& Painter::setIcon(const QString& Name, qreal Scale, qreal Offset)
{
    DrawIcon = true;
    IconName = Name;
    IconScale = Scale;
    IconOffset = Offset;
    return *this;
}

IconParameters Painter::icon() const
{
    IconParameters P;
    P.Draw = DrawIcon;
    P.Name = IconName;
    P.Proportional = IconScale;
    P.Fixed = IconOffset;
    return P;
}

Painter& Painter::iconActive(bool b)
{
    DrawIcon = b;
    return *this;
}

void Painter::setSelector(const QString& anExpression)
{
    theSelector = anExpression;
}

bool Painter::matchesZoom(qreal PixelPerM) const
{
    if (ZoomLimitSet)
        return (ZoomUnder <= PixelPerM) && (PixelPerM <= ZoomUpper);
    return true;
}

/* GlobalPainter */

GlobalPainter::GlobalPainter()
    : DrawBackground(false), DrawNodes(false)
{
}

GlobalPainter::GlobalPainter(const GlobalPainter& f)
    : DrawBackground(f.DrawBackground), BackgroundColor(f.BackgroundColor)
    , DrawNodes(f.DrawNodes), NodesColor(f.NodesColor), NodesProportional(f.NodesProportional), NodesFixed(f.NodesFixed)
{
}

GlobalPainter& GlobalPainter::operator=(const GlobalPainter& f)
{
    if (&f == this) return *this;

    DrawBackground = f.DrawBackground;
    BackgroundColor = f.BackgroundColor;

    DrawNodes = f.DrawNodes;
    NodesColor = f.NodesColor;
    NodesProportional = f.NodesProportional;
    NodesFixed = f.NodesFixed;

    return *this;
}

GlobalPainter::~GlobalPainter()
{
}

QString GlobalPainter::toXML() const
{
    QString r;
    r += "<global\n";
    if (DrawBackground)
        r += " " + colorAsXML("background",BackgroundColor);
    if (DrawNodes)
        r += " " + boundaryAsXML("nodes",NodesColor, NodesProportional, NodesFixed);
    r += "/>\n";
    return r;
}

GlobalPainter GlobalPainter::fromXML(const QDomElement& e)
{
    GlobalPainter FP;

    if (e.hasAttribute("backgroundColor")) {
        FP.backgroundActive(true);
        FP.background(toColor(e.attribute("backgroundColor")));
    }
    if (e.hasAttribute("nodesColor")) {
        FP.nodesActive(true);
        FP.NodesColor = toColor(e.attribute("nodesColor"));
        FP.NodesProportional = e.attribute("nodesScale").toDouble();
        FP.NodesFixed = e.attribute("nodesOffset").toDouble();
    }

    return FP;
}

GlobalPainter& GlobalPainter::backgroundActive(bool b)
{
    DrawBackground = b;
    return *this;
}

bool GlobalPainter::getDrawBackground() const
{
    return DrawBackground;
}

GlobalPainter& GlobalPainter::background(QColor Color)
{
    DrawBackground = true;
    BackgroundColor = Color;
    return *this;
}

QColor GlobalPainter::getBackgroundColor() const
{
    return BackgroundColor;
}

GlobalPainter & GlobalPainter::nodesActive(bool b)
{
    DrawNodes = b;
    return *this;
}

GlobalPainter & GlobalPainter::nodes(QColor Color)
{
    DrawNodes = true;
    NodesColor = Color;
    return *this;
}

bool GlobalPainter::getDrawNodes() const
{
    return DrawNodes;
}

QColor GlobalPainter::getNodesColor() const
{
    return NodesColor;
}
