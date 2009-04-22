#ifndef XQSETTINGSMANAGER_H
#define XQSETTINGSMANAGER_H

// INCLUDES
#include "xqsettingsitem.h"

// FORWARD DECLARATIONS
class XQSettingsManagerPrivate;

// CLASS DECLARATION
class XQSettingsManager : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        NotFoundError,
        AlreadyExistsError,
        UnknownError = -1
    };
        
    XQSettingsManager(XQSettingsItem item = XQSettingsItem(),
            QObject *parent = 0);
    XQSettingsManager(QObject *parent);
    ~XQSettingsManager();
      
    bool createItem(const XQPropertyItem& item);
    bool deleteItem(const XQPropertyItem& item);
    bool setItemValue(const XQPropertyItem& item);
    
    XQSettingsItem item(const XQSettingsItem& item) const;
    bool startMonitoring(const XQSettingsItem& item);
    bool stopMonitoring(const XQSettingsItem& item);
    XQSettingsManager::Error error() const;
      
Q_SIGNALS:
    void valueChanged(const XQSettingsItem& item);
    void itemDeleted(const XQSettingsItem& item);
    
private:
    friend class XQSettingsManagerPrivate;
    XQSettingsManagerPrivate *d;

};

#endif // XQSETTINGSMANAGER_H

// End of file

