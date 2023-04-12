#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>

#include <QFuture>
#include <QtConcurrent>

#include "udpclient.h"
#include "commandline.h"
#include "chat.h"
#include "tcpclient.h"
#include "networkaccessmanager.h"
#include "tcpserver.h"
#include "multithreadtcpserver.h"

namespace localNetworkEnumuration
{
    void main()
    {
        QList<QHostAddress> list = QNetworkInterface::allAddresses();

        for (QList<QHostAddress>::Iterator iter = list.begin(); iter < list.end(); ++iter)
        {
            QHostAddress address = *iter;
            qInfo() << address.toString();

            qInfo() << "\t Loopback: " << address.isLoopback();
            qInfo() << "\t Broadcast: " << address.isBroadcast();
            qInfo() << "\t Multicast: " << address.isMulticast();
            switch (address.protocol())
            {
            case QAbstractSocket::UnknownNetworkLayerProtocol:
                qInfo() << "\tProtocol: Unknown";
                break;
            case QAbstractSocket::AnyIPProtocol:
                qInfo() << "\tProtocol: Any";
                break;
            case QAbstractSocket::IPv4Protocol:
                qInfo() << "\tProtocol: IP4";
                break;
            case QAbstractSocket::IPv6Protocol:
                qInfo() << "\tProtocol: IP6";
                break;
            default:
                break;
            }
        }
    }
}

// asynronized connection
namespace udpClient
{
    void main()
    {
        QEventLoop loop;
        // code
        UdpClient client;
        client.start();

        loop.exec();
    }
}

namespace udpChat
{
    void main()
    {
        QEventLoop loop;

        // code
        CommandLine cli(&loop, stdin); // loop entered to establish parent-child relatianship
        QFuture<void> future = QtConcurrent::run(&CommandLine::monitor, &cli);

        Chat chat;
        QObject::connect(&cli, &CommandLine::command, &chat, &Chat::command, Qt::QueuedConnection); // version <6 => Qt::QueuedConnection
        qInfo() << "Enter your name to begin:";

        loop.exec();
    }
}

namespace tcpClient
{
    void main()
    {
        QEventLoop loop;

        TcpClient client;
        QString hostname = "google.com";
        quint16 port = 443;
        // github.com
        // www.httpbin.org
        // www.google.com
        // client.connectToHost("www.httpbin.org", 80); // 80 is the common unencrypted
        // qt have openssl dependency for ssl
        client.connectToHost(hostname, port); // SSL: 443 is the common encrypted

        loop.exec();
    }
}

namespace synronizedSockets
{
    // waste of time and computer resources!!!.
    // use signal-slot
    // But it will come in handy understanding the logic in multithreaded web servers
    bool bind(QUdpSocket &socket, quint16 port)
    {
        // "ReuseAddressHint" works for windows 11
        if (socket.bind(port, QAbstractSocket::ReuseAddressHint))
            return true;

        qInfo() << socket.errorString();
        return false;
    }
    void send(QUdpSocket &socket, quint16 port)
    {
        qInfo() << "Sending...";
        QString date = QDateTime::currentDateTime().toString();
        QByteArray data = date.toLatin1(); // ascii

        QNetworkDatagram datagram(data, QHostAddress::Broadcast, port);
        if (!socket.writeDatagram(datagram))
        {
            qInfo() << socket.errorString();
        }
    }
    void read(QUdpSocket &socket)
    {
        qInfo() << "Reading...";
        if (socket.hasPendingDatagrams())
        {
            QNetworkDatagram datagram = socket.receiveDatagram();
            qInfo() << datagram.data();
        }
    }
    void udp()
    {
        QUdpSocket source, destination;
        quint16 port = 2023;
        qInfo() << "Binding...";
        if (bind(source, port) && bind(destination, port))
        {
            send(source, port);
            read(destination);
        }
        else
        {
            qInfo() << "Could not bind";
        }
    }

    void tcp()
    {
        QString hostname = "www.google.com";
        quint16 port = 80;
        QTcpSocket socket;
        socket.connectToHost(hostname, port);
        qInfo() << "Connecting...";

        if (socket.waitForConnected(5000))
        { // give some time.
            qInfo() << "Sending request";

            QByteArray data;
            data.append("GET / HTTP/1.1\r\n");
            data.append("Host: local\r\n");
            data.append("Connection: Close\r\n");
            data.append("\r\n");

            socket.write(data);
            socket.waitForBytesWritten(5000);

            if (socket.waitForReadyRead(5000))
            {
                QByteArray reponse = socket.readAll();
                qInfo() << "Response: " << reponse.length() << " bytes";
                qInfo() << "Response: " << reponse;
            }

            socket.close();
            socket.waitForDisconnected();
            qInfo() << "Disconnecting...";
        }
        else
        {
            qInfo() << "Error: " << socket.errorString();
        }
    }
    void main()
    {
        udp();
        // tcp();
    }
}

namespace threadedSocket
{
    void udp()
    {
        QEventLoop loop;

        // code
        //  thread should not have parent from another thread!!!
        UdpClient *client = new UdpClient();
        QThread thread;
        thread.setObjectName("Thread 1");
        client->moveToThread(&thread);
        QObject::connect(&thread, &QThread::started, client, &UdpClient::run);
        thread.start();

        loop.exec();
    }
    void tcp()
    {
        QString hostname = "www.google.com";
        quint16 port = 443;

        QEventLoop loop;

        TcpClient *client = new TcpClient();
        QThread thread;
        thread.setObjectName("Thread 1");
        client->moveToThread(&thread);
        QObject::connect(&thread, &QThread::started, client, &TcpClient::run);
        client->setHostname(hostname);
        client->setPort(port);

        thread.start();

        loop.exec();
    }
    void main()
    {
        // udp();
        tcp();
    }
}

namespace NetworkAccessManagerSpace
{
    void main()
    {

        QNetworkAccessManager nm;
        qInfo() << "Supported protocols:" << nm.supportedSchemes();

        QEventLoop loop;

        NetworkAccessManager manager;
        manager.get("https://postman-echo.com/get?foo1=bar1&foo2=bar2");

        QByteArray data;
        data.append("param1=hello");
        data.append("&");
        data.append("param2=foo");

        manager.post("https://postman-echo.com/post", data);

        loop.exec();
    }
}

namespace Server
{
    void tcp()
    {
        QEventLoop loop;

        TcpServer *server = new TcpServer();
        server->setPort(23);
        server->start();

        loop.exec();
    }
    void tcp_multithread()
    {
        QEventLoop loop;

        MultiThreadTcpServer server;
        server.start(2023);

        loop.exec();
    }
    void main()
    {
        // tcp();
        tcp_multithread();
    }
}

int main(int argc, char *argv[])
{
    QThread::currentThread()->setObjectName("Main Thread");
    QCoreApplication a(argc, argv);
    // localNetworkEnumuration::main();
    // udpClient::main();
    // udpChat::main();
    // tcpClient::main();
    // synronizedSockets::main();
    // threadedSocket::main();
    // NetworkAccessManagerSpace::main();
    Server::main();

    qInfo() << "Program stopped Execution";

    return a.exec();
}
