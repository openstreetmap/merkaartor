// INCLUDE FILES
#include "xqmessaging.h"
#ifdef Q_OS_SYMBIAN
#include "xqmessaging_s60_p.h"
#else
#include "xqmessaging_stub_p.h"
#endif

/****************************************************
 *
 * XQMessaging
 *
 ****************************************************/
/*!
    \class XQMessaging

    \brief The XQMessaging class can be used for sending and receiving SMS and MMS Messages.
    
    Example:
    \code
    XQMessaging *messaging = new XQMessaging(this);
    connect(messaging, SIGNAL(messageReceived(const XQMessage &)), this, SLOT(smsMessageReceived(const XQMessage &)));
    messaging->startReceiving();

    QLineEdit *addressEdit = new QLineEdit(this);
    QTextEdit *messageEdit = new QTextEdit(this);
    XQMessage message(QStringList(addressEdit->text()),QString(messageEdit->toPlainText()));
    messaging->send(message);
    \endcode
*/

/*!
    Constructs a XQMessaging object with the given parent.
    \sa send()
*/
XQMessaging::XQMessaging(QObject * parent)
 : QObject(parent), d(new XQMessagingPrivate(this))
{
}

/*!
    Destroys the XQMessaging object.
*/
XQMessaging::~XQMessaging()
{
    delete d;
}

/*!
    \enum XQMessaging::Error

    This enum defines the possible errors for a XQMessaging object.
*/
/*! \var XQMessaging::Error XQMessaging::NoError
    No error occured.
*/
/*! \var XQMessaging::Error XQMessaging::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQMessaging::Error XQMessaging::InternalError
    Internal error.
*/
/*! \var XQMessaging::Error XQMessaging::UnknownError
    Unknown error.
*/

/*!
    \enum XQMessaging::MsgTypeFlag

    This enum defines the possible message types for a XQMessaging object.
*/
/*! \var XQMessaging::MsgTypeNotDefined XQMessaging::MsgTypeNotDefined
    Message type is not defined.
*/
/*! \var XQMessaging::MsgTypeSMS XQMessaging::MsgTypeSMS
    Message type is SMS.
*/
/*! \var XQMessaging::MsgTypeMMS XQMessaging::MsgTypeMMS
    Message type is MMS.
*/

/*!
    Sends SMS or MMS Message. If message contains attachment(s), message will
    be sent as MMS, otherwise message will be sent as SMS.

    \param message message to be sent
    \return NoError if message was successfully sent; otherwise returns error.
*/
XQMessaging::Error XQMessaging::send(const XQMessage & message)
{
    return d->send(message);
}

/*!
    Starts receiving XQMessages.
    messageReceived signal is emitted every time SMS or MMS
    message is received.

    \param msgType the type for messages to be received (SMS or MMS or both)
    \return NoError if receing of messages was successfully started; otherwise returns error.

    \sa stopReceiving(), messageReceived()
*/
XQMessaging::Error XQMessaging::startReceiving(XQMessaging::MsgType msgType)
{
    return d->startReceiving(msgType);
}

/*!
    Stops receiving XQMessages.

    \sa startReceiving()
*/
void XQMessaging::stopReceiving()
{
    d->stopReceiving();
}

/*!
    \fn void XQMessaging::messageReceived(const XQMessage& message)

    This signal is emitted when message is received.

    \param message a received message
    \sa startReceiving(), stopReceiving()
*/

/*!
    \fn void XQMessaging::error(XQMessaging::Error errorCode)

    This signal is emitted when error has occured.

    \param errorCode a error code
    \sa startReceiving()
*/


/****************************************************
 *
 * XQMessage
 *
 ****************************************************/
/*!
    \class XQMessage

    \brief The XQMessage class can be used to construct SMS or MMS Message.
*/

/*!
    Constructs an empty XQMessage object.
*/
XQMessage::XQMessage()
    : msgtype(XQMessaging::MsgTypeNotDefined)
{
}

/*!
    Constructs a new SMS XQMessage object with specified receivers and body.
*/
XQMessage::XQMessage(const QStringList & receivers, const QString & body)
    : msgreceivers(receivers),
      msgbody(body),
      msgtype(XQMessaging::MsgTypeSMS)
{
}

/*!
    Constructs a new MMS XQMessage object with specified receivers, body and attachments.
*/
XQMessage::XQMessage(const QStringList & receivers, const QString & body, const QStringList & attachmentPaths)
    : msgreceivers(receivers),
      msgbody(body),
      msgattachmentPaths(attachmentPaths),
      msgtype(XQMessaging::MsgTypeMMS)
{
}

/*!
    Destroys the XQMessage object.
*/
XQMessage::~XQMessage()
{
}

/*!
    Sets receivers of the message.

    \param receivers list of receivers of the message
    \sa receivers()
*/
void XQMessage::setReceivers(const QStringList & receivers)
{
    msgreceivers = receivers;
}

/*!
    Sets the body of the message.

    \param body the body of the message
    \sa body()
*/
void XQMessage::setBody(const QString & body)
{
    msgbody = body;
}

/*!
    Sets the subject of the message.

    \param subject the subject of the message
    \sa subject()
*/
void XQMessage::setSubject(const QString & subject)
{
    msgsubject = subject;
}

/*!
    Sets attachments of the message.

    \param attachmentPaths list of paths to attachments of the message
    \sa attachments()
*/
void XQMessage::setAttachments(const QStringList & attachmentPaths)
{
    msgattachmentPaths = attachmentPaths;
}

/*!
    Sets the sender of the message.

    \param sender the sender of the message
    \sa sender()
*/
void XQMessage::setSender(const QString & sender)
{
    msgsender = sender;
}

/*!
    Sets the type of the message.

    \param type the type of the message
    \sa type()
*/
void XQMessage::setType(XQMessaging::MsgType type)
{
    msgtype = type;
}

/*!
    Returns list of receivers of the message.

    \return list of receivers of the message
    \sa setReceivers()
*/
QStringList XQMessage::receivers() const
{
    return msgreceivers;
}

/*!
    Returns the body of the message.

    \return the body of the message
    \sa setBody()
*/
QString XQMessage::body() const
{
    return msgbody;
}

/*!
    Returns the subject of the message.

    \return the subject of the message
    \sa setSubject()
*/
QString XQMessage::subject() const
{
    return msgsubject;
}

/*!
    Returns list of paths to attachments of the message.

    \return list of paths to attachments of the message
    \sa setAttachments()
*/
QStringList XQMessage::attachments() const
{
    return msgattachmentPaths;
}

/*!
    Returns the sender of the message.

    \return the sender of the message
    \sa setSender()
*/
QString XQMessage::sender() const
{
    return msgsender;
}

/*!
    Returns the type of the message.

    \return the type of the message
    \sa setType()
*/
XQMessaging::MsgType XQMessage::type() const
{
    return msgtype;
}

// End of file

