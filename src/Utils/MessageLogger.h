/******************************************************************************
 * Filename: MesageLogger.h
 *
 * Created: 2017/05/26 17:48
 * Author: Ladislav Láska
 * e-mail: krakonos@krakonos.org
 *
 ******************************************************************************/

#ifndef __MESSAGELOGGER_H__
#define __MESSAGELOGGER_H__

#include <QTime>
#include <QDebug>
#include <QLoggingCategory>

#define mError(...) qCritical(__VA_ARGS__)

using Timestamp = qint64;

class LoggedMessage {
  public:
    LoggedMessage( QtMsgType type, const QMessageLogContext &ctx, QString message, Timestamp timestamp) :
      type(type), 
      category(ctx.category),
      message(message),
      timestamp(timestamp)
    {
    }
    QtMsgType type;
    QString category;
    QString message;
    Timestamp timestamp;
};

class MessageLogger : public QObject {
  Q_OBJECT
  public:
    MessageLogger() {
      timeZero_ = QDateTime::currentMSecsSinceEpoch(); 
    }

    ~MessageLogger() {
      qInstallMessageHandler(0);
    }

    static MessageLogger& GetInstance() {
      static MessageLogger instance_;
      return instance_;
    }

    void installHandler();

    void newMessage(QtMsgType type, const QMessageLogContext &ctx, const QString &message);

  signals:
    void ErrorReported( QSharedPointer< LoggedMessage > m );

  private:
    QList< QSharedPointer<const LoggedMessage> > messageBuffer_;

    Timestamp timeZero_;
    Timestamp getTimestamp() { 
      return QDateTime::currentMSecsSinceEpoch()-timeZero_; 
    }



    bool shouldLog(QtMsgType/* type */) {
      return true;
    }

    bool shouldPrint(QtMsgType/* type */) {
      return true;
    }

};

#endif
