#ifndef XQMESSAGING_S60_P_H_
#define XQMESSAGING_S60_P_H_

// INCLUDES
#include <private/qobject_p.h>
#include <QList>
#include <QString>
#include <msvapi.h>
#include <msvstd.h>
#include "xqmessaging.h"

// FORWARD DECLARATIONS
class XQSMSMessaging;
class CMsvSession;
class CMsvEntry;
class CSmsClientMtm;
class CMmsClientMtm;
class CClientMtmRegistry;
class CCharFormatLayer;
class CParaFormatLayer;
class CMsvSessionObserver;

// CLASS DECLARATION
class XQMessagingPrivate : public CBase
{
public:
    XQMessagingPrivate(XQMessaging* apParent);
    ~XQMessagingPrivate();

    XQMessaging::Error send(const XQMessage& message);
    XQMessaging::Error startReceiving(XQMessaging::MsgType msgType);
    void stopReceiving();

    friend class CMsvSessionObserver;    

private:
    XQMessaging::Error sendSMSL(const XQMessage& message);
    XQMessaging::Error sendMMSL(const XQMessage& message);
    void DeliverMessage(XQMessage& message);
    void DeliverError(XQMessaging::Error error);
    XQMessaging::Error initializeObserver(XQMessaging::MsgType msgType);

private: // Data
    CMsvSessionObserver* ipObserver;
    XQMessaging*         ipParent;
};

class CMsvSessionObserver : public CActive, public MMsvSessionObserver
{
public: // Constructors and destructor
    static CMsvSessionObserver* NewL(XQMessagingPrivate* apParent, XQMessaging::MsgType msgType);
    ~CMsvSessionObserver();

private: // from MMsvSessionObserver
    void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1,
                             TAny* aArg2, TAny* aArg3);
    void DeliverMessagesL();

private: // from CActive
    void RunL();
    void DoCancel();

private: // Constructors
    CMsvSessionObserver(XQMessagingPrivate* apParent);
    void ConstructL(XQMessaging::MsgType msgType);

public: // Data
    bool                     iListenForIncomingMessages;
    XQMessaging::MsgType     iMsgType;

private: // Data
    TInt                     counter;                  
    XQMessagingPrivate*      ipParent;

    CCharFormatLayer*        ipCharFormatLayer;
    CParaFormatLayer*        ipParaFormatLayer;
    CRichText*               ipRichText;

    TBool                    iMsvSessionReady;
    CMsvSession*             ipMsvSession;      // Session with the messaging server
    CClientMtmRegistry*      ipClientMtmReg;
    CSmsClientMtm*           ipSmsMtm;
    CMmsClientMtm*           ipMmsMtm;
    
    CMsvEntrySelection*      ipReceivedMessages;
    RTimer                   iTimer;            // Timer used for delaying delivering of received messages
    
    TBuf<KMaxPath>           iPath;
};

#endif /* XQMESSAGING_S60_P_H_ */

// End of file

