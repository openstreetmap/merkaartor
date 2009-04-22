#ifndef XQLOCATION_H
#define XQLOCATION_H

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQLocationPrivate;

// CLASS DECLARATION
class XQLocation : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        OpenError,
        DefaultModuleNotFoundError,
        ModuleNotFoundError,
        InvalidUpdateIntervalError,
        AccessDeniedError,
        InternalError,
        UnknownError = -1
    };

    enum DeviceStatus {
        StatusError,
        StatusDisabled,
        StatusInactive,
        StatusInitialising,
        StatusStandBy,
        StatusReady,
        StatusActive,
        StatusUnknown = -1
    };

    enum DataQuality {
        DataQualityLoss,
        DataQualityPartial,
        DataQualityNormal,
        DataQualityUnknown = -1
    };

    XQLocation(QObject *parent = 0);
    ~XQLocation();
    
    void setUpdateInterval(int interval);
    int updateInterval() const;

    XQLocation::DeviceStatus status() const;
    XQLocation::DataQuality dataQuality() const;
    
    XQLocation::Error open();
    void close();

Q_SIGNALS:
    void locationChanged(double latitude, double longitude, double altitude, float speed);
    void latitudeChanged(double latitude, float accuracy);
    void longitudeChanged(double longitude, float accuracy);
    void altitudeChanged(double altitude, float accuracy);
    void speedChanged(float speed);

    void statusChanged(XQLocation::DeviceStatus status);
    void dataQualityChanged(XQLocation::DataQuality dataQuality);
    void numberOfSatellitesInViewChanged(int numSatellites);
    void numberOfSatellitesUsedChanged(int numSatellites);

    void error(XQLocation::Error errorCode);

public Q_SLOTS:
    void requestUpdate();
    void startUpdates();
    void startUpdates(int msec);
    void stopUpdates();

private: // Data
	XQLocationPrivate *d;
	friend class XQLocationPrivate;
};

#endif // XQLOCATION_H

// End of file
