#ifndef XQCONTACT_H
#define XQCONTACT_H

// INCLUDES
#include <QObject>
#include <QVariant>
#include <QString>
#include <QList>

// FORWARD DECLARATIONS
class XQContactsPrivate;
class XQContactFieldPrivate;
class XQContactFieldAddress;
class XQContactFieldEmail;
class XQContactFieldPicture;
class XQContactFieldPhoneNumber;
class XQContactFieldUrl;
class XQContactFieldSipId;

// CLASS DECLARATION
class XQContactField
{
public:
    enum ContactFieldType {
        ContactFieldTypeAddress,
        ContactFieldTypePostOffice,
        ContactFieldTypeExtendedAddress,
        ContactFieldTypeLocality,
        ContactFieldTypeRegion,
        ContactFieldTypePostCode,
        ContactFieldTypeCountry,
        ContactFieldTypeCompanyName,
        ContactFieldTypePhoneNumber,
        ContactFieldTypeGivenName,
        ContactFieldTypeFamilyName,
        ContactFieldTypeAdditionalName,
        ContactFieldTypeSuffixName,
        ContactFieldTypePrefixName,
        ContactFieldTypeEMail,
        ContactFieldTypeNote,
        ContactFieldTypeBirthday,
        ContactFieldTypeUrl,
        ContactFieldTypePicture,
        ContactFieldTypeJobTitle,
        ContactFieldTypeSecondName,
        ContactFieldTypeSIPID,
        ContactFieldTypeAssistant,
        ContactFieldTypeAnniversary,
        ContactFieldTypeSpouse,
        ContactFieldTypeChildren,
        ContactFieldTypeDepartmentName,
        ContactFieldTypeUnknown = -1
    };
    
    XQContactField();
    XQContactField(XQContactField::ContactFieldType fieldType, const QVariant& value);
    XQContactField(const XQContactField& other);
    virtual ~XQContactField();

    bool isNull() const;
    
    void setType(XQContactField::ContactFieldType fieldType);
    XQContactField::ContactFieldType type() const;
    void setLabel(const QString& value);
    QString label() const;
    void setValue(const QVariant& value);
    QVariant value() const;
    void setId(int id);
    int id() const;
    
    XQContactFieldAddress toFieldAddress() const;
    XQContactFieldEmail toFieldEmail() const;
    XQContactFieldPicture toFieldPicture() const;
    XQContactFieldPhoneNumber toFieldPhoneNumber() const;
    XQContactFieldUrl toFieldUrl() const;
    XQContactFieldSipId toFieldSipId() const;

    XQContactField &operator=(const XQContactField& other);

protected:
    virtual void detach();
    
protected: // Data
    XQContactFieldPrivate *d;
    
    friend class XQContactsPrivate;
    friend class XQContact;
};

class XQContactFieldAddress : public XQContactField
{
public:
    XQContactFieldAddress();
    XQContactFieldAddress(XQContactField::ContactFieldType fieldType, const QVariant& value);
    XQContactFieldAddress(const XQContactFieldAddress& other);

    void setHome(bool enable);
    bool isHome() const;
    void setWork(bool enable);
    bool isWork() const;
    void setPreferred(bool enable);
    bool isPreferred() const;
};

class XQContactFieldEmail : public XQContactField
{
public:
    XQContactFieldEmail();
    XQContactFieldEmail(const QVariant& value);
    XQContactFieldEmail(const XQContactFieldEmail& other);

    void setHome(bool enable);
    bool isHome() const;
    void setWork(bool enable);
    bool isWork() const;
    void setPreferred(bool enable);
    bool isPreferred() const;

protected:
    void detach();
};

class XQContactFieldPicture : public XQContactField
{
public:
    enum ContactFieldPictureType {
        ContactFieldPictureTypeNotSet = 0,
        ContactFieldPictureTypeGif,
        ContactFieldPictureTypeCgm,
        ContactFieldPictureTypeWmf,
        ContactFieldPictureTypeBmp,
        ContactFieldPictureTypeDib,
        ContactFieldPictureTypePs,
        ContactFieldPictureTypePmb,
        ContactFieldPictureTypePdf,
        ContactFieldPictureTypePict,
        ContactFieldPictureTypeTiff,
        ContactFieldPictureTypeJpeg,
        ContactFieldPictureTypeMet,
        ContactFieldPictureTypeMpeg,
        ContactFieldPictureTypeMpeg2,
        ContactFieldPictureTypeAvi,
        ContactFieldPictureTypeQTime
    };

    XQContactFieldPicture();
    XQContactFieldPicture(const QVariant& value);
    XQContactFieldPicture(const XQContactFieldPicture& other);

    void setPictureType(ContactFieldPictureType type);
    ContactFieldPictureType pictureType() const;
    
    bool isPath() const;
    bool isImage() const;
    QImage valueAsImage() const;

protected:
    void detach();
};

class XQContactFieldPhoneNumber : public XQContactField
{
public:
    XQContactFieldPhoneNumber();
    XQContactFieldPhoneNumber(const QVariant& value);
    XQContactFieldPhoneNumber(const XQContactFieldPhoneNumber& other);

    void setHome(bool enable);
    bool isHome() const;
    void setWork(bool enable);
    bool isWork() const;
    void setPreferred(bool enable);
    bool isPreferred() const;
    void setVoice(bool enable);
    bool isVoice() const;
    void setCellular(bool enable);
    bool isCellular() const;
    void setPager(bool enable);
    bool isPager() const;
    void setBbs(bool enable);
    bool isBbs() const;
    void setModem(bool enable);
    bool isModem() const;
    void setCar(bool enable);
    bool isCar() const;
    void setIsdn(bool enable);
    bool isIsdn() const;
    void setVideo(bool enable);
    bool isVideo() const;
    void setMessage(bool enable);
    bool isMessage() const;
    void setFax(bool enable);
    bool isFax() const;

protected:
    void detach();
};

class XQContactFieldUrl : public XQContactField
{
public:
    XQContactFieldUrl();
    XQContactFieldUrl(const QVariant& value);
    XQContactFieldUrl(const XQContactFieldUrl& other);

    void setHome(bool enable);
    bool isHome() const;
    void setWork(bool enable);
    bool isWork() const;

protected:
    void detach();
};

class XQContactFieldSipId : public XQContactField
{
public:
    enum ContactFieldSipType {
        ContactFieldSipTypeNotSet = 0,
        ContactFieldSipTypePoc,
        ContactFieldSipTypeSwis,
        ContactFieldSipTypeVoip
    };

    XQContactFieldSipId();
    XQContactFieldSipId(const QVariant& value);
    XQContactFieldSipId(const XQContactFieldSipId& other);

    void setSipType(ContactFieldSipType type);
    ContactFieldSipType sipType() const;

protected:
    void detach();
};

class XQContact
{
public:
    XQContact();
    XQContact(const XQContact &other);
    ~XQContact();
    
    void addField(XQContactField field);
    bool removeField(XQContactField field);
    bool removeFieldByType(XQContactField::ContactFieldType fieldType);
    XQContactField fieldByType(XQContactField::ContactFieldType fieldType) const;
    QList<XQContactField> fieldsByType(XQContactField::ContactFieldType fieldType) const;
    QList<XQContactFieldAddress> addressFields() const;
    QList<XQContactField> fields() const;
    long int id() const;
    bool isNull() const;
    XQContact &operator=(const XQContact& other);
    
    friend class XQContactsPrivate;
    
private:
    QList<XQContactField> contactFields;
    bool                    isContactNull;
    long int                contactId;
};

#endif // XQCONTACT_H

// End of file

