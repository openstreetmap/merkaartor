#ifndef IMERKMAINWINDOW_H
#define IMERKMAINWINDOW_H

class MapView;
class Document;
class QGPS;
class PropertiesDock;
class InfoDock;
class FeaturesDock;

class IMerkMainWindow
{
public:
    virtual MapView* view() = 0;
    virtual void invalidateView(bool UpdateDock = true) = 0;

    virtual Document* document() = 0;
    virtual QGPS* gps() = 0;
    virtual PropertiesDock* properties() = 0;
    virtual InfoDock* info() = 0;
    virtual FeaturesDock* features() = 0;

};

#endif // IMERKMAINWINDOW_H
