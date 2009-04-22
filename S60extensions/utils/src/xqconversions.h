#ifndef XQCONVERSIONS
#define XQCONVERSIONS

// INCLUDES
#include <QObject>

// CLASS DECLARATION
class XQConversions
{
public:
    static QString s60DescToQString(const TDesC& desc);
    static HBufC* qStringToS60Desc(const QString& string);
    static QString s60Desc8ToQString(const TDesC8& desc);
    static HBufC8* qStringToS60Desc8(const QString& string);
    static QByteArray s60Desc8ToQByteArray(const TDesC8& desc);
    static HBufC8* qByteArrayToS60Desc8(const QByteArray& string);
};

#endif // XQCONVERSIONS

// End of file
