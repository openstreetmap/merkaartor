#ifndef XQUTILS_P_H
#define XQUTILS_P_H

// FORWARD DECLARATIONS
class XQUtils;
class CDocumentHandler;

// CLASS DECLARATION
class XQUtilsPrivate: public CBase
{
public:
    XQUtilsPrivate(XQUtils *utils);
    ~XQUtilsPrivate();
    
    bool launchFile(const QString& filename);
	void resetInactivityTime();
    XQUtils::Error error() const;

private:
    CDocumentHandler* iDocHandler;
    XQUtils *q;
    int iError;
};

#endif /*XQUTILS_P_H*/

// End of file
