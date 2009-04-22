#include "xqconversions.h"
#include "utf.h"

/*!
    \class XQConversions
    \brief XQConversions class offers functions for converting Symbian/Series60 data types to Qt data types and vice versa.
*/


/*!
    Converts Symbian/Series60 descriptor (string) to QString

    \param desc descriptor to be converted
    \return QString containing converted string
*/
QString XQConversions::s60DescToQString(const TDesC& desc)
{
    return QString::fromUtf16(desc.Ptr(),desc.Length());
}

/*!
    Converts QString to Symbian/Series60 descriptor (string).
    Note: Ownership of returned descriptor (string) is transferred to caller

    \param string QString to be converted
    \return pointer to Symbian/Series60 descriptor on success;
            otherwise returns NULL pointer
*/
HBufC* XQConversions::qStringToS60Desc(const QString& string)
{
    TPtrC16 str(reinterpret_cast<const TUint16*>(string.utf16()));
    return str.Alloc();
}

/*!
    Converts Symbian/Series60 8 bit descriptor (UTF8 string) to QString

    \param desc 8 bit descriptor to be converted
    \return QString on success; otherwise returns null QString
*/
QString XQConversions::s60Desc8ToQString(const TDesC8& desc)
{
    QString qtString;
    HBufC* s60str;
    TRAPD(error, s60str = CnvUtfConverter::ConvertToUnicodeFromUtf8L(desc));
    if (error == KErrNone) {
        qtString = QString::fromUtf16(s60str->Ptr(),s60str->Length());
        delete s60str;
    }
    return qtString;
}

/*!
    Converts QString to Symbian/Series60 8 bit descriptor (UTF8 string).
    Note: Ownership of returned descriptor (string) is transferred to caller

    \param string QString to be converted
    \return pointer to UTF8 string in Symbian/Series60 descriptor on success;
            otherwise returns NULL pointer
*/
HBufC8* XQConversions::qStringToS60Desc8(const QString& string)
{
    TPtrC16 str(reinterpret_cast<const TUint16*>(string.utf16()));
    HBufC8* s60str;
    TRAPD(error, s60str = CnvUtfConverter::ConvertFromUnicodeToUtf8L(str));
    if (error != KErrNone) {
        return NULL;
    }
    return s60str;
}

/*!
    Converts Symbian/Series60 8 bit descriptor to QByteArray

    \param desc 8 bit descriptor to be converted
    \return QByteArray on success; otherwise returns null QString
*/
QByteArray XQConversions::s60Desc8ToQByteArray(const TDesC8& desc)
{
    return QByteArray((const char *)desc.Ptr(),desc.Length());
}


/*!
    Converts QByteArray to Symbian/Series60 8 bit descriptor.
    Note: Ownership of returned descriptor (string) is transferred to caller

    \param byteArray QByteArray to be converted
    \return pointer to 8 bit descriptor string in Symbian/Series60 descriptor on success;
            otherwise returns NULL pointer
*/
HBufC8* XQConversions::qByteArrayToS60Desc8(const QByteArray& byteArray)
{
    TPtrC8 ptr8((TUint8 *)(byteArray.constData()));
    return ptr8.Alloc();
}

// End of file
