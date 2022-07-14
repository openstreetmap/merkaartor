/******************************************************************************
 * Filename: RemoteControlServer.cpp
 *
 * Created: 2018/07/29 22:20
 * Author: Ladislav Láska
 * e-mail: krakonos@krakonos.org
 *
 ******************************************************************************/

#include <QTcpSocket>
#include <QDateTime>
#include <QRegularExpression>

#include "RemoteControlServer.hpp"

using namespace Merkaartor;
using namespace Merkaartor::RemoteControlServerPriv;

static QString date_time_rfc7231() {
    const auto now = QDateTime::currentDateTimeUtc();

    //generate format string to format UTC timestamp similar to
    //"Sun, 06 Nov 1994 08:49:37 GMT"
    //use *non-localized* names for day & month.

    QString fmt_IMF_fixdate;
    fmt_IMF_fixdate.reserve(35);

    switch(now.date().dayOfWeek()) {
        case Qt::Monday: fmt_IMF_fixdate.append("'Mon'"); break;
        case Qt::Tuesday: fmt_IMF_fixdate.append("'Tue'"); break;
        case Qt::Wednesday: fmt_IMF_fixdate.append("'Wed'"); break;
        case Qt::Thursday: fmt_IMF_fixdate.append("'Thu'"); break;
        case Qt::Friday: fmt_IMF_fixdate.append("'Fri'"); break;
        case Qt::Saturday: fmt_IMF_fixdate.append("'Sat'"); break;
        case Qt::Sunday: fmt_IMF_fixdate.append("'Sun'"); break;
        //other values are possible, so use localized name as fallback
        default: fmt_IMF_fixdate.append("ddd"); break;
    }
    fmt_IMF_fixdate.append(", dd ");
    switch(now.date().month()) {
        case  1: fmt_IMF_fixdate.append("'Jan'"); break;
        case  2: fmt_IMF_fixdate.append("'Feb'"); break;
        case  3: fmt_IMF_fixdate.append("'Mar'"); break;
        case  4: fmt_IMF_fixdate.append("'Apr'"); break;
        case  5: fmt_IMF_fixdate.append("'May'"); break;
        case  6: fmt_IMF_fixdate.append("'Jun'"); break;
        case  7: fmt_IMF_fixdate.append("'Jul'"); break;
        case  8: fmt_IMF_fixdate.append("'Aug'"); break;
        case  9: fmt_IMF_fixdate.append("'Sep'"); break;
        case 10: fmt_IMF_fixdate.append("'Oct'"); break;
        case 11: fmt_IMF_fixdate.append("'Nov'"); break;
        case 12: fmt_IMF_fixdate.append("'Dec'"); break;
        //other values are possible, so use localized name as fallback
        default: fmt_IMF_fixdate.append("MMM"); break;
    }
    fmt_IMF_fixdate.append(" yyyy HH:mm:ss 'GMT'");

    return now.toString(fmt_IMF_fixdate);
};

void RemoteControlConnection::readyRead() {
    qDebug() << "RemoteControlConnection: readyRead.";
    if ( m_socket->canReadLine() ) {
        qDebug() << "RemoteControlConnection: canReadLine.";
        QString ln = m_socket->readLine();
        QStringList tokens = ln.split( QRegularExpression("[ \r\n][ \r\n]*"), Qt::SkipEmptyParts );
        if ( tokens[0] == "GET" ) {
            m_responseStream << "HTTP/1.1 200 OK\r\n";
            m_responseStream << "Date: " << date_time_rfc7231() << "\r\n";
            m_responseStream << "Server: Merkaartor RemoteControl\r\n";
            m_responseStream << "Content-type: text/plain\r\n";
            m_responseStream << "Access-Control-Allow-Origin: *\r\n";
            m_responseStream << "Content-length: 4\r\n\r\n";
            m_responseStream << "OK\r\n";
            m_responseStream << flush;
            m_socket->disconnectFromHost();

            qDebug() << "RemoteControlConnection: url read, response sent.";
            emit requestReceived(tokens[1]);
        }
    }
}

RemoteControlConnection::RemoteControlConnection( QTcpSocket *socket )
    : m_socket(socket), m_responseStream(socket)
{
    connect( m_socket, &QTcpSocket::readyRead, this, &RemoteControlConnection::readyRead);
    connect( m_socket, &QTcpSocket::disconnected, m_socket, &QTcpSocket::deleteLater );
    connect( m_socket, &QTcpSocket::destroyed, this, &QObject::deleteLater );
}


RemoteControlServer::RemoteControlServer( QObject* parent )
    :QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &RemoteControlServer::newConnection );
}

void RemoteControlServer::newConnection() {
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    if (socket == nullptr) {
        qDebug() << "RemoteControlServer::newConnection invoked, but no connection is pending.";
        return;
    }
    /* The RemoteControlconnection will handle it's own destruction when the connection is broken. */
    auto connHandler = new RemoteControlConnection(socket);
    /* Note:
     * Qt::QueuedConnection is a workaround for a problem hit in issue #147.
     * When this signal is called, it triggers a long sequence of events, some
     * of them opening it's own QEventLoop (dialogs). This seems to cause
     * issues in deleting the RemoteControlConnection objects, specifically the
     * QTcpSocket inside is destroyed, and after that a signal is called on it.
     * This seems to be a bug in Qt, as we correctly use deleteLater().
     * However, there might be more to it, as I have been unable to put
     * together a minimal testcase.
     *
     * For now, we make sure the main EventLoop is executing the download
     * dialogs and hopefully avoid this problem.
     */
    connect( connHandler, &RemoteControlConnection::requestReceived,
            this, [this](QString requestUrl) { emit requestReceived(requestUrl); }, Qt::QueuedConnection);

}

void RemoteControlServer::listen() {
    if (!m_tcpServer->listen( QHostAddress::LocalHost, 8111 )) {
        qWarning() << "Unable to open port localhost:8111: " << m_tcpServer->errorString();
    }
}

void RemoteControlServer::close() {
    m_tcpServer->close();
}
