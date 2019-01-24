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

#include "RemoteControlServer.hpp"

using namespace Merkaartor::RemoteControlServerNs;
using namespace Merkaartor::RemoteControlServerNs::Priv;

void RemoteControlConnection::readyRead() {
    qDebug() << "RemoteControlConnection: readyRead.";
    if ( m_socket->canReadLine() ) {
        qDebug() << "RemoteControlConnection: canReadLine.";
        QString ln = m_socket->readLine();
        QStringList tokens = ln.split( QRegExp("[ \r\n][ \r\n]*"), QString::SkipEmptyParts );
        if ( tokens[0] == "GET" ) {
            m_responseStream << "HTTP/1.1 200 OK\r\n";
            m_responseStream << "Date: " << QDateTime::currentDateTime().toString(Qt::TextDate);
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
     * Qt::QueueConnection is a workaround for a problem hit in issue #147.
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
