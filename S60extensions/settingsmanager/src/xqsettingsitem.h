#ifndef XQSETTINGSITEM_H
#define XQSETTINGSITEM_H

// INCLUDES
#include <Qvariant>

// CLASS DECLARATION
class XQSettingsItem
{
public:
    enum Type {
        TypeRepository,
        TypeProperty
    };
    
    XQSettingsItem();
    XQSettingsItem(long int category, unsigned long int key,
            XQSettingsItem::Type type, const QVariant& value);
    virtual ~XQSettingsItem();
    
    long int category() const;
    unsigned long int key() const;
    XQSettingsItem::Type type() const;
    QVariant value() const;
    bool isNull() const;
    
    void setCategory(long int category);
    void setKey(unsigned long int key);
    void setType(XQSettingsItem::Type type);
    void setValue(const QVariant& value);

private:
    long int iCategory;
    unsigned long int iKey;
    XQSettingsItem::Type iType;
    QVariant iValue;
};

class XQRepositoryItem : public XQSettingsItem
{
public:   
    XQRepositoryItem(long int category, long int key,
            const QVariant& value = QVariant(QVariant::Invalid)); 
};

class XQPropertyItem : public XQSettingsItem
{
public:
    XQPropertyItem(long int category, long int key,
            const QVariant& value = QVariant(QVariant::Invalid));
};

#endif /* XQSETTINGSITEM_H*/
