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
namespace RemoteControlServerNs {
namespace Priv {

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

        /** Start listening for remote requests. */
        void listen();

        /** 
         * Close the port and stop listening. No further requests will be
         * received, but pending connections might still trigger events. */
        void close();

    signals:
        void requestReceived(QString requestUrl);

    private slots:
        /** Internal slot to handle incoming TCP connections. */
        void newConnection();

    private:
        QTcpServer* m_tcpServer;
};

}} /* namespace Merkaartor::RemoteControlServer */

#endif
