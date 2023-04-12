#include "udpclient.h"

UdpClient::UdpClient(QObject *parent)
    : QObject{parent}
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    timer = new QTimer(this);
    socket = new QUdpSocket(this);
    loop = new QEventLoop(this);

    connect(timer, &QTimer::timeout, this, &UdpClient::timeout, Qt::ConnectionType::UniqueConnection);
    connect(socket, &QUdpSocket::readyRead, this, &UdpClient::readyRead, Qt::ConnectionType::UniqueConnection);
    timer->setInterval(1000);
    counter = 0;
}

UdpClient::~UdpClient()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();
    qInfo() << "Deconstructed";
}

// thread start
void UdpClient::run()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    start();
    loop->exec();
}

void UdpClient::start()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    if (!socket->bind(port)) // QAbstractSocket::ShareAddress
    {
        qInfo() << socket->errorString();
        return;
    }

    qInfo() << "Started UDP on " << socket->localAddress() << ":" << socket->localPort();

    broadcast();
}

void UdpClient::stop()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    timer->stop();
    socket->close();

    if (loop)
    {
        qInfo() << "Stopping event loop";
        loop->quit();
        delete loop;
    }

    qInfo() << "Stopped";
    deleteLater();
}

void UdpClient::timeout()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    QString date = QDateTime::currentDateTime().toString();
    QByteArray data = date.toLatin1();

    QNetworkDatagram datagram(data, QHostAddress::Broadcast, port);
    qInfo() << "Send: " << data;
    socket->writeDatagram(datagram);

    ++counter;
    if (counter > 5)
        stop();
}

void UdpClient::readyRead()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    while (socket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = socket->receiveDatagram();
        qInfo() << "Read: " << datagram.data() << " from " << datagram.senderAddress() << ":" << datagram.senderPort() << " to " << datagram.destinationAddress() << ":" << datagram.destinationPort();
    }
}

void UdpClient::broadcast()
{
    qInfo() << Q_FUNC_INFO << QThread::currentThread();

    qInfo() << "Broadcasting...";
    timer->start();
}

// Setter/Getter
qint16 UdpClient::getPort() const
{
    return port;
}

void UdpClient::setPort(qint16 newPort)
{
    port = newPort;
}
