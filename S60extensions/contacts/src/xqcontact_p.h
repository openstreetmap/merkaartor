#ifndef XQCONTACT_P_H
#define XQCONTACT_P_H

// INCLUDES
#include "xqcontact.h"
#include <QString>
#include <QList>
#include <QVariant>

// CLASS DECLARATION
class XQContactFieldPrivate
{
public:
    enum ContactFieldProperty {
        ContactFieldPropertyHome,
        ContactFieldPropertyWork,
        ContactFieldPropertyPref,
        ContactFieldPropertyVoice,
        ContactFieldPropertyCell,
        ContactFieldPropertyPager,
        ContactFieldPropertyBbs,
        ContactFieldPropertyModem,
        ContactFieldPropertyCar,
        ContactFieldPropertyIsdn,
        ContactFieldPropertyVideo,
        ContactFieldPropertyMsg,
        ContactFieldPropertyFax,
        ContactFieldPropertyPoc,
        ContactFieldPropertySwis,
        ContactFieldPropertyVoip,
        ContactFieldPropertyX509,
        ContactFieldPropertyPGP,
        ContactFieldPropertyDom,
        ContactFieldPropertyGif,
        ContactFieldPropertyCgm,
        ContactFieldPropertyWmf,
        ContactFieldPropertyBmp,
        ContactFieldPropertyDib,
        ContactFieldPropertyPs,
        ContactFieldPropertyPmb,
        ContactFieldPropertyPdf,
        ContactFieldPropertyPict,
        ContactFieldPropertyTiff,
        ContactFieldPropertyJpeg,
        ContactFieldPropertyMet,
        ContactFieldPropertyMpeg,
        ContactFieldPropertyMpeg2,
        ContactFieldPropertyAvi,
        ContactFieldPropertyQTime,
        ContactFieldPropertyUnknown = -1
    };
    
    XQContactFieldPrivate();
    ~XQContactFieldPrivate();
    
    void setProperty(XQContactFieldPrivate::ContactFieldProperty property, bool enable);
    bool isProperty(XQContactFieldPrivate::ContactFieldProperty property) const;

public: // Data
    QAtomicInt ref;
    int id;
    XQContactField::ContactFieldType type;
    QString label;
    QVariant value;
    QList<ContactFieldProperty> properties;
};

#endif // XQCONTACT_P_H

// End of file

