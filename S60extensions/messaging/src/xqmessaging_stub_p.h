#ifndef XQMESSAGING_STUB_P_H_
#define XQMESSAGING_STUB_P_H_

// INCLUDES
#include "private/qobject_p.h"
#include "xqsmsmessaging.h"

// CLASS DECLARATION
class XQMessagingPrivate : public QObjectPrivate, CBase
{
    Q_DECLARE_PUBLIC(XQSMSMessaging)

public:
    XQMessagingPrivate();
    ~XQMessagingPrivate();

    XQMessaging::Error send(const XQMessage & message);
    void startReceiving(XQMessaging::MsgType msgType);
    void stopReceiving();

public: // Data
    int error;

private: // Data
};

#endif /* XQMESSAGING_STUB_P_H_ */

// End of file
