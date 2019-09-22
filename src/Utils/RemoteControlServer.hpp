/******************************************************************************
 * Filename: RemoteControlServer.hpp
 *
 * Created: 2018/07/29 22:08
 * Author: Ladislav Láska
 * e-mail: krakonos@krakonos.org
 *
 ******************************************************************************/

#ifndef __REMOTECONTROLSERVER_HPP__
#define __REMOTECONTROLSERVER_HPP__

#include <QObject>
#include <QTcpServer>


namespace Merkaartor {
namespace RemoteControlServerPriv {

/**
 * Instance is spawned for each connection to the remote control port. Once a
 * request is received, it emits requestReceived signal and closes the
 * connection gracefully.
 *
 * This object shall not be used outside of RemoteControlServer. The server
 * proxies the requestReceived() signal.
 *
 * The object destroys itself after the connection is closed.
 */
class RemoteControlConnection : public QObject {
    Q_OBJECT
    public: 
        RemoteControlConnection( QTcpSocket *socket );
    public slots:
        void readyRead();
    signals:
        void requestReceived(QString requestUrl);
    private:
        QTcpSocket* m_socket;
        QTextStream m_responseStream;
};

}

class RemoteControlServer : public QObject {
    Q_OBJECT 

    public:
        RemoteControlServer(QObject* parent = nullptr);

        /** Start listening for remote requests. Request is automatically
         * accepted and appropriate requestReceived signal is emitted. This
         * includes invalid requests as well. */
        void listen();

        /** 
         * Close the port and stop listening. No further requests will be
         * received, but pending connections might still trigger events. */
        void close();

    signals:
        /**
         * Emitted every time a new remote control request is received.
         */
        void requestReceived(QString requestUrl);

    private slots:
        /** Internal slot to handle incoming TCP connections. */
        void newConnection();

    private:
        QTcpServer* m_tcpServer;
};

} /* namespace Merkaartor */

#endif
