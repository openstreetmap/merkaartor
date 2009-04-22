// INCLUDE FILES
#include "xqmessaging.h"
#include "xqmessaging_s60_p.h"
#include <e32des16.h>
#include <rsendasmessage.h>
#include <rsendas.h>
#include <mtclreg.h>
#include <smut.h>
#include <mmsconst.h>
#include <smsclnt.h>        // CSmsClientMtm
#include <mmsclient.h>      // CMmsClientMtm 
#include <msvstore.h>       // CMsvStore
#include <txtrich.h>        // CRichText
#include <txtfmlyr.h>
#include <smuthdr.h>        // CSmsHeader
#include <utf.h>            // CnvUtfConverter
#include <MMsvAttachmentManager.h>
#include <CMsvAttachment.h>
#include <SendUiConsts.h>

const TInt KWaitAfterReceivedMessage = 100000; // = 0.1 seconds

#ifdef __WINS__
const TMsvId KObservedFolderId = KMsvDraftEntryId;
#else
const TMsvId KObservedFolderId = KMsvGlobalInBoxIndexEntryId;
#endif

XQMessagingPrivate::XQMessagingPrivate(XQMessaging* apParent)
    : ipParent(apParent)
{
}

XQMessagingPrivate::~XQMessagingPrivate()
{
    delete ipObserver;
}

XQMessaging::Error XQMessagingPrivate::send(const XQMessage & message)
{
   XQMessaging::Error error = XQMessaging::NoError;
    
    if (message.type() == XQMessaging::MsgTypeSMS) {
        TRAPD(err, error = sendSMSL(message));
        if (err) {
            error = XQMessaging::InternalError;
        }
    } else if (message.type() == XQMessaging::MsgTypeMMS) {
        TRAPD(err, error = sendMMSL(message));
        if (err) {
            error = XQMessaging::InternalError;
        }
    } else if (message.attachments().count() > 0 || !message.subject().isNull()) {
        TRAPD(err, error = sendMMSL(message));
        if (err) {
            error = XQMessaging::InternalError;
        }
    } else {
        TRAPD(err, error = sendSMSL(message));
        if (err) {
            error = XQMessaging::InternalError;
        }
    }

    return error;
}

XQMessaging::Error XQMessagingPrivate::sendSMSL(const XQMessage & message)
{
    if (message.body().isNull()) {
        return XQMessaging::NullMessageBodyError;
    }
    if (message.receivers().count() == 0) {
        return XQMessaging::ReceiverNotDefinedError;
    }

    RSendAs sendAs;
    TInt err = sendAs.Connect();
    if (err) {
        return XQMessaging::InternalError;
    }
    CleanupClosePushL(sendAs);

    RSendAsMessage sendAsMessage;
    sendAsMessage.CreateL(sendAs, KUidMsgTypeSMS);
    CleanupClosePushL(sendAsMessage);

    // Prepare the message
    
    // Add receivers
    QStringListIterator i(message.receivers());
    TPtrC16 receiver(KNullDesC);
    QString qreceiver;
    while (i.hasNext()) {
        qreceiver = i.next();
        receiver.Set(reinterpret_cast<const TUint16*>(qreceiver.utf16()));
        sendAsMessage.AddRecipientL(receiver, RSendAsMessage::ESendAsRecipientTo);
    }

    // Set Body text
    QString body = message.body();
    TPtrC16 msg(reinterpret_cast<const TUint16*>(body.utf16()));
    HBufC* bd = msg.AllocL();
    sendAsMessage.SetBodyTextL(*bd);
    
    // Send the message
    sendAsMessage.SendMessageAndCloseL();

    CleanupStack::Pop(); // sendAsMessage (already closed)
    CleanupStack::PopAndDestroy(); // sendAs

    return XQMessaging::NoError;
}

XQMessaging::Error XQMessagingPrivate::sendMMSL(const XQMessage & message)
{
    if (message.receivers().count() == 0) {
        return XQMessaging::ReceiverNotDefinedError;
    }

    RSendAs sendAs;
    TInt err = sendAs.Connect();
    if (err) {
        return XQMessaging::InternalError;
    }
    CleanupClosePushL(sendAs);

    RSendAsMessage sendAsMessage;
    sendAsMessage.CreateL(sendAs, KUidMsgTypeMultimedia);
    CleanupClosePushL(sendAsMessage);

    // Prepare the message
    
    // Add receivers
    QStringListIterator i(message.receivers());
    TPtrC16 receiver(KNullDesC);
    QString qreceiver;
    while (i.hasNext()) {
        qreceiver = i.next();
        receiver.Set(reinterpret_cast<const TUint16*>(qreceiver.utf16()));
        sendAsMessage.AddRecipientL(receiver, RSendAsMessage::ESendAsRecipientTo);
    }
    
    // Set Subject
    QString subject = message.subject();
    TPtrC16 sbj(reinterpret_cast<const TUint16*>(subject.utf16()));
    sendAsMessage.SetSubjectL(sbj);

    // Set Body text
    QString body = message.body();
    TPtrC16 msg(reinterpret_cast<const TUint16*>(body.utf16()));
    HBufC8* pMsg = CnvUtfConverter::ConvertFromUnicodeToUtf8L(msg);
    
    RFs fileServer;
    User::LeaveIfError(fileServer.Connect());
    RFile file;
    TFileName tempFileName;
    // Temporary file will be written to private folder (because Path == KNullDesC)
    err = file.Temp(fileServer,KNullDesC,tempFileName,EFileWrite);
    if (err) {
        return XQMessaging::InternalError;
    }
    file.Write(*pMsg);
    file.Close();
    delete pMsg;
    
    TRequestStatus status;
    sendAsMessage.AddAttachment(tempFileName,_L8("text/plain"),status);
    User::WaitForRequest(status);
    
    if (message.attachments().count() > 0) {
        // Add attachments
        TPtrC16 attachmentPath(KNullDesC);
        QString qattachmentPath;
        QStringListIterator j(message.attachments());
        while (j.hasNext()) {
            qattachmentPath = j.next();
            attachmentPath.Set(reinterpret_cast<const TUint16*>(qattachmentPath.utf16()));
            sendAsMessage.AddAttachment(attachmentPath,status);
            User::WaitForRequest(status);
        }
    }
    fileServer.Delete(tempFileName);
    fileServer.Close();
    
    // Send the message
    sendAsMessage.SendMessageAndCloseL();

    CleanupStack::Pop(); // sendAsMessage (already closed)
    CleanupStack::PopAndDestroy(); // sendAs

    return XQMessaging::NoError;
}

XQMessaging::Error XQMessagingPrivate::startReceiving(XQMessaging::MsgType msgType)
{
    if (!ipObserver) {
        XQMessaging::Error error = initializeObserver(msgType);
        if (error != XQMessaging::NoError) {
            return error;
        }
    }
    ipObserver->iMsgType = msgType;
    ipObserver->iListenForIncomingMessages = true;

    return XQMessaging::NoError;
}

void XQMessagingPrivate::stopReceiving()
{
    delete ipObserver;
    ipObserver = NULL;
}

XQMessaging::Error XQMessagingPrivate::initializeObserver(XQMessaging::MsgType msgType)
{
    TRAPD(err, ipObserver = CMsvSessionObserver::NewL(this, msgType));
    if (err) {
        return XQMessaging::InternalError;
    }

    return XQMessaging::NoError;
}

void XQMessagingPrivate::DeliverMessage(XQMessage& message)
{
    emit ipParent->messageReceived(message);
}

void XQMessagingPrivate::DeliverError(XQMessaging::Error error)
{
    emit ipParent->error(error);
}

CMsvSessionObserver* CMsvSessionObserver::NewL(XQMessagingPrivate* apParent, XQMessaging::MsgType msgType)
{
    CMsvSessionObserver* self = new (ELeave) CMsvSessionObserver(apParent);
    self->ConstructL(msgType);
    return self;
}

CMsvSessionObserver::CMsvSessionObserver(XQMessagingPrivate* apParent)
    : CActive(EPriorityStandard), ipParent(apParent)
{
}

void CMsvSessionObserver::ConstructL(XQMessaging::MsgType msgType)
{
    iMsgType = msgType;
    ipReceivedMessages = new (ELeave) CMsvEntrySelection;
    // Initialize a channel of communication between a client thread (Client-side MTM,
    // User Interface MTM,or message client application) and the Message Server thread.
    ipMsvSession = CMsvSession::OpenAsyncL(*this);
    User::LeaveIfError(iTimer.CreateLocal());
    CActiveScheduler::Add(this);
    
    RFs fileServer;
    User::LeaveIfError(fileServer.Connect());
    CleanupClosePushL(fileServer);
    TBuf<KMaxPath> privatePath;
    fileServer.CreatePrivatePath(EDriveC);
    fileServer.PrivatePath(privatePath);
    iPath.Append(_L("c:"));
    iPath.Append(privatePath);
    iPath.Append(_L("tempattachments\\"));                         
    CFileMan* pFileMan=CFileMan::NewL(fileServer);
    CleanupStack::PushL(pFileMan);
    pFileMan->RmDir(iPath);
    fileServer.MkDirAll(iPath);
    CleanupStack::PopAndDestroy(pFileMan);
    CleanupStack::PopAndDestroy(&fileServer);
    
    ipClientMtmReg = CClientMtmRegistry::NewL(*ipMsvSession);
    //Note: If capabilities are missing, then iSmsMtm stays null
    // Get the SMS Mtm client from the registry
    ipSmsMtm = static_cast<CSmsClientMtm*>(ipClientMtmReg->NewMtmL(KUidMsgTypeSMS));
    ipMmsMtm = static_cast<CMmsClientMtm*>(ipClientMtmReg->NewMtmL(KUidMsgTypeMultimedia));
}

CMsvSessionObserver::~CMsvSessionObserver()
{
    TRAPD(error,
        RFs fileServer;
        if (fileServer.Connect() == KErrNone) {
            CleanupClosePushL(fileServer);
            TBuf<KMaxPath> privatePath;
            fileServer.CreatePrivatePath(EDriveC);
            fileServer.PrivatePath(privatePath);
            TBuf<KMaxPath> path;
            path.Append(_L("c:"));
            path.Append(privatePath);
            path.Append(_L("tempattachments\\"));                         
            CFileMan* pFileMan=CFileMan::NewL(fileServer);
            CleanupStack::PushL(pFileMan);
            pFileMan->RmDir(path);
            CleanupStack::PopAndDestroy(pFileMan);
            CleanupStack::PopAndDestroy(&fileServer);
        }
    );

    Cancel();
    iTimer.Close();

    delete ipReceivedMessages;

    delete ipRichText;
    delete ipParaFormatLayer;
    delete ipCharFormatLayer;

    delete ipSmsMtm;
    delete ipMmsMtm;
    delete ipClientMtmReg;

    //TODO: Check: ipMsvSession->RemoveObserver(*this);
    delete ipMsvSession;
}

void CMsvSessionObserver::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1,
                                              TAny* aArg2, TAny* /*aArg3*/)
{
    switch (aEvent) {
        case EMsvServerReady:
            iMsvSessionReady = ETrue;
            break;
        case EMsvEntriesCreated:
            if (aArg2 && *(static_cast<TMsvId*>(aArg2)) == KObservedFolderId
                      && iListenForIncomingMessages) {
                CMsvEntrySelection* entries = static_cast<CMsvEntrySelection*>(aArg1);

                if (entries != NULL) {
                    TInt count = entries->Count();
                    while (count--) {
                        TRAPD(err,
                            const TMsvId id = (*entries)[count];
                            CMsvEntry* pReceivedEntry = ipMsvSession->GetEntryL(id);
                            CleanupStack::PushL(pReceivedEntry);
                            const TMsvEntry& entry = pReceivedEntry->Entry();
                            if ((iMsgType & XQMessaging::MsgTypeSMS) && 
                                (entry.iMtm == KUidMsgTypeSMS) &&
                                (entry.iType == KUidMsvMessageEntry)) {
                                ipReceivedMessages->AppendL(id);
                            } else if ((iMsgType & XQMessaging::MsgTypeMMS) &&
                                       (entry.iMtm == KUidMsgTypeMultimedia) &&
                                       (entry.iType == KUidMsvMessageEntry)) {
                                  ipReceivedMessages->AppendL(id);
                            }
                            CleanupStack::PopAndDestroy(pReceivedEntry);
                        );
                        if (err) {
                            ipParent->DeliverError(XQMessaging::InternalError);
                        }
                    }

                    if (ipReceivedMessages->Count() != 0) {
                        counter = 0;
                        Cancel(); // Cancel possible previous iTimer
                        // Wait 0.1 seconds to make sure that message is ready to be read
                        iTimer.After(iStatus, KWaitAfterReceivedMessage);
                        SetActive();
                    }
                }
            }
            break;

        default:
            break;
    }
}

void CMsvSessionObserver::RunL()
{
    TRAPD(err, DeliverMessagesL());
    if (err) {
        counter++;
        // Wait another 0.1 seconds to give time to message to be ready
        iTimer.After(iStatus, KWaitAfterReceivedMessage);
        SetActive();
        if (counter > 50) { // One message is waited only for 5 seconds in maximum
            ipParent->DeliverError(XQMessaging::InternalError);
            // Remove problematic message from queue
            counter = 0;
            ipReceivedMessages->Delete(0,1);
            iTimer.After(iStatus, KWaitAfterReceivedMessage);
            SetActive();
        }
    }
}

void CMsvSessionObserver::DeliverMessagesL()
{
    TInt count = ipReceivedMessages->Count();
    while (count--) {
        QString messageSubject;
        QString messageBody;
        QString messageSender;
        QStringList attachmentPaths;
        XQMessaging::MsgType type = XQMessaging::MsgTypeNotDefined;

        const TMsvId id = (*ipReceivedMessages)[0];
        CMsvEntry* pReceivedEntry = ipMsvSession->GetEntryL(id);
        CleanupStack::PushL(pReceivedEntry);

        // Read message sender
        if (ipSmsMtm || ipMmsMtm) {
            if (ipSmsMtm && pReceivedEntry->Entry().iMtm == KUidMsgTypeSMS) {
                ipSmsMtm->SwitchCurrentEntryL(id);
                ipSmsMtm->LoadMessageL();
                CSmsHeader& header = ipSmsMtm->SmsHeader();
                messageSender = QString::fromUtf16(header.FromAddress().Ptr(),
                                                   header.FromAddress().Length());
            } else if (ipMmsMtm && pReceivedEntry->Entry().iMtm == KUidMsgTypeMultimedia) {
                ipMmsMtm->SwitchCurrentEntryL(id);
                ipMmsMtm->LoadMessageL();
                messageSender = QString::fromUtf16(ipMmsMtm->Sender().Ptr(),
                                                   ipMmsMtm->Sender().Length());
            }
        } else if (pReceivedEntry->Entry().iDetails.Length() > 0) {
            messageSender = QString::fromUtf16(pReceivedEntry->Entry().iDetails.Ptr(),
                                               pReceivedEntry->Entry().iDetails.Length());
        }
        
        CMsvStore* pStore = pReceivedEntry->ReadStoreL();
        CleanupStack::PushL(pStore);
        if (pReceivedEntry->Entry().iMtm == KUidMsgTypeSMS) {
            type = XQMessaging::MsgTypeSMS;
            // Read message body for SMS
            if (pStore->HasBodyTextL()) {
                if (!ipRichText) {
                    ipCharFormatLayer = CCharFormatLayer::NewL();
                    ipParaFormatLayer = CParaFormatLayer::NewL();
                    ipRichText=CRichText::NewL(ipParaFormatLayer,ipCharFormatLayer);
                }
                ipRichText->Reset();
                pStore->RestoreBodyTextL(*ipRichText);
                HBufC* pMessage = HBufC::NewLC(ipRichText->DocumentLength());
                TPtr ptr2(pMessage->Des());
                ipRichText->Extract(ptr2);
                messageBody = QString::fromUtf16(pMessage->Ptr(),pMessage->Length());
                CleanupStack::PopAndDestroy(pMessage);
            }
        } else if (pReceivedEntry->Entry().iMtm == KUidMsgTypeMultimedia) {
            type = XQMessaging::MsgTypeMMS;
            // Read message subject for MMS
            if (pReceivedEntry->Entry().iDescription.Length() > 0) {
                messageSubject = QString::fromUtf16(pReceivedEntry->Entry().iDescription.Ptr(),
                                                    pReceivedEntry->Entry().iDescription.Length());
            }
            TInt count = pStore->AttachmentManagerL().AttachmentCount();
            TBool textAdded = EFalse;
            for (TInt i = 0; i < count; i++) {
                CMsvAttachment* pAttachment = pStore->AttachmentManagerL().GetAttachmentInfoL(i);
                CleanupStack::PushL(pAttachment);
                if (pAttachment->MimeType() == _L8("text/plain") && !textAdded) {
                    // Read message body for MMS
                    textAdded = ETrue;
                    RFile file = pStore->AttachmentManagerL().GetAttachmentFileL(i);
                    CleanupClosePushL(file);
                    TInt fileSize;
                    file.Size(fileSize);
                    HBufC8* pFileContent = HBufC8::NewLC(fileSize);
                    TPtr8 fileContent(pFileContent->Des());
                    file.Read(fileContent);
                    HBufC* pMsg = CnvUtfConverter::ConvertToUnicodeFromUtf8L(*pFileContent);
                    CleanupStack::PopAndDestroy(pFileContent);
                    CleanupStack::PopAndDestroy(&file);
                    messageBody = QString::fromUtf16(pMsg->Ptr(),pMsg->Length());
                    delete pMsg;
                } else {
                    // Read attachment for MMS
                    //const TDesC& filePath = pAttachment->FilePath();
                    //attachmentPaths.append(QString::fromUtf16(filePath.Ptr(),filePath.Length()));
                    // Read attachment for MMS
                    RFile file = pStore->AttachmentManagerL().GetAttachmentFileL(i);
                    CleanupClosePushL(file);
                    TInt fileSize;
                    file.Size(fileSize);
                    HBufC8* pFileContent = HBufC8::NewL(fileSize);
                    TPtr8 fileContent(pFileContent->Des());
                    file.Read(fileContent);
                    CleanupStack::PopAndDestroy(&file);
                    CleanupStack::PushL(pFileContent);
                    // write tempFile to private folder
                    RFs fileServer;
                    User::LeaveIfError(fileServer.Connect());
                    CleanupClosePushL(fileServer);
                    RFile file2;
                    TFileName tempFileName;
                    TInt err = file2.Temp(fileServer,iPath,tempFileName,EFileWrite);
                    if (err == KErrNone) {
                         CleanupClosePushL(file2);
                         file2.Write(*pFileContent);
                         CleanupStack::PopAndDestroy(&file2);
                         attachmentPaths.append(QString::fromUtf16(tempFileName.Ptr(),tempFileName.Length()));
                    }
                    CleanupStack::PopAndDestroy(&fileServer);
                    CleanupStack::PopAndDestroy(pFileContent);
                }
                CleanupStack::PopAndDestroy(pAttachment);
            }
        }
        CleanupStack::PopAndDestroy(pStore);

        CleanupStack::PopAndDestroy(pReceivedEntry);
        ipReceivedMessages->Delete(0,1);

        XQMessage message;
        message.setType(type);
        message.setSender(messageSender);
        if (!messageBody.isEmpty()) {
            message.setBody(messageBody);
        }
        if (!messageSubject.isEmpty()) {
            message.setSubject(messageSubject);
        }
        if (!attachmentPaths.isEmpty()) {
            message.setAttachments(attachmentPaths);
        }
        ipParent->DeliverMessage(message);
    }
}

void CMsvSessionObserver::DoCancel()
{
    iTimer.Cancel();
}

// End of file

