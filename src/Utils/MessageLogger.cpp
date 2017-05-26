/******************************************************************************
 * Filename: MesageLogger.cpp
 *
 * Created: 2017/05/26 17:48
 * Author: Ladislav Láska
 * e-mail: krakonos@krakonos.org
 *
 ******************************************************************************/


#include <QDebug>
#include <QTextStream>

#include "MessageLogger.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &message) {
  MessageLogger::GetInstance().newMessage(type, ctx, message);

}

void MessageLogger::installHandler() {
  qInstallMessageHandler(myMessageHandler);
}

QTextStream& operator<< (QTextStream& stream, LoggedMessage& msg) {
  auto typeToString = [](QtMsgType t) {
    switch (t) {
      case QtMsgType::QtInfoMsg:     return QString("(II)");
      case QtMsgType::QtDebugMsg:    return QString("(DD)");
      case QtMsgType::QtWarningMsg:  return QString("(WW)");
      case QtMsgType::QtFatalMsg:    return QString("(FF)");
      case QtMsgType::QtCriticalMsg: return QString("(CC)");
      default:                       return QString("(\?\?)"); /* NOTE: ??) is a trigraph. Escape it. */
                                
    }
  };
  stream.setFieldAlignment( QTextStream::FieldAlignment::AlignLeft );
  stream
    << QTime::fromMSecsSinceStartOfDay(msg.timestamp % (1000*60*60*24) ).toString("hh:mm:ss.zzz") << " "
    << typeToString(msg.type)
    << " [ "
      << qSetFieldWidth(30) << msg.category << qSetFieldWidth(0)
    << " ] "
    << msg.message
    << "\n";
  return stream;
}

void MessageLogger::newMessage(QtMsgType type, const QMessageLogContext &ctx, const QString &message) {
  auto ptr = QSharedPointer<LoggedMessage>( new LoggedMessage(type, ctx, message, getTimestamp()) );

  if (shouldLog(type)) {
    messageBuffer_.append( ptr );
  }

  if (shouldPrint(type)) {
    QTextStream stream(stderr);
    stream << *ptr;
  }

  if (type == QtMsgType::QtCriticalMsg) {
    emit ErrorReported(ptr);
  }
}
