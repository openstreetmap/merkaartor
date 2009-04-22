#ifndef XQMESSAGING_H
#define XQMESSAGING_H

// INCLUDES
#include <QObject>
#include <QString>
#include <QStringList>

// FORWARD DECLARATIONS
class XQMessage;

// CLASS DECLARATION
class XQMessaging : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        NullMessageBodyError,
        ReceiverNotDefinedError,
        InternalError,
        UnknownError = -1
    };

    enum MsgTypeFlag {
        MsgTypeNotDefined = 0x00000000,
        MsgTypeSMS        = 0x00000001,
        MsgTypeMMS        = 0x00000002
        //MsgTypeEmail      = 0x00000004
    };
    Q_DECLARE_FLAGS(MsgType, MsgTypeFlag)

    XQMessaging(QObject *parent = 0);
    ~XQMessaging();

    XQMessaging::Error send(const XQMessage & message);

Q_SIGNALS:
    void error(XQMessaging::Error errorCode);
    void messageReceived(const XQMessage& message);

public Q_SLOTS:
    XQMessaging::Error startReceiving(XQMessaging::MsgType msgType = MsgTypeSMS | MsgTypeMMS);
    void stopReceiving();

private:
    friend class XQMessagingPrivate;
    XQMessagingPrivate *d;
};

class XQMessage
{
public:
    XQMessage();
    XQMessage(const QStringList& receivers, const QString& body);
    XQMessage(const QStringList& receivers, const QString& body, const QStringList& attachmentPaths);
    virtual ~XQMessage();

    virtual void setReceivers(const QStringList& receivers);
    virtual void setBody(const QString& body);
    virtual void setSubject(const QString& subject);
    virtual void setAttachments(const QStringList& attachmentPaths);
    virtual void setSender(const QString& sender);
    virtual void setType(XQMessaging::MsgType type);
    virtual QStringList receivers() const;
    virtual QString body() const;
    virtual QString subject() const;
    virtual QStringList attachments() const;
    virtual QString sender() const;
    virtual XQMessaging::MsgType type() const;

private: // Data
    QStringList msgreceivers;
    QString     msgsubject;
    QString     msgbody;
    QStringList msgattachmentPaths;
    QString     msgsender;
    XQMessaging::MsgType msgtype;
};

#endif // XQSMSMESSAGING_H

// End of file

