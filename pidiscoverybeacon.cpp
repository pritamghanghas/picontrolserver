#include "pidiscoverybeacon.h"
#include <QNetworkInterface>

PiDiscoveryBeacon::PiDiscoveryBeacon(const QString &discoveryMessage, const QStringList interfaces, int discoveryInterval,  QObject *parent) :
    QObject(parent),
    m_discoveryMessage(discoveryMessage),
    m_activeInterfaces(interfaces)
{
    m_discoveryIntervalTimer.setTimerType(Qt::PreciseTimer);
    m_discoveryIntervalTimer.setInterval(discoveryInterval);
    m_seqNumber = 0;

    sendHelloDataGram();
    connect(&m_discoveryIntervalTimer, SIGNAL(timeout()), SLOT(sendHelloDataGram()));
    m_discoveryIntervalTimer.start();

}

void PiDiscoveryBeacon::sendHelloDataGram()
{
    static QStringList subnets = broadcastSubnet();

    QString message = m_discoveryMessage;
    message += QString(" %1").arg(m_seqNumber);
    m_seqNumber++;

    qDebug() << "sending hello datagram: " << message << "to subnets: " << subnets;

    QByteArray helloDatagram(message.toUtf8());

    Q_FOREACH(const QString subnet, subnets)
    {
        qint64 rc = m_socket.writeDatagram(helloDatagram, QHostAddress(subnet), 31311);
        if (rc == -1) {
            qDebug() << "failed to send hello datagram to broadcast address" << subnet;
        }
    }
}

QStringList PiDiscoveryBeacon::broadcastSubnet()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    QStringList ads;
    Q_FOREACH(QNetworkInterface interface, interfaces) {
        if (!m_activeInterfaces.contains(interface.name())) {
            qDebug() << "not one of the active interfaces";
            continue;
        }

        QList<QNetworkAddressEntry> interfaceAddress = interface.addressEntries();

         Q_FOREACH(QNetworkAddressEntry address, interfaceAddress) {
             QHostAddress ip = address.ip();

            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                ads.append(address.broadcast().toString());
            }
        }
    }
    qDebug() << "got the following broadcast addresses for the device" << ads;
    return ads;
}

QStringList PiDiscoveryBeacon::deviceAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    QStringList ads;
    Q_FOREACH(QNetworkInterface interface, interfaces) {
        if (!m_activeInterfaces.contains(interface.name())) {
            qDebug() << "not one of the active interfaces";
            continue;
        }

        QList<QNetworkAddressEntry> interfaceAddress = interface.addressEntries();

         Q_FOREACH(QNetworkAddressEntry address, interfaceAddress) {
             QHostAddress ip = address.ip();

            if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                ads.append(ip.toString());
            }
        }
    }
    qDebug() << "got the following addresses for the device" << ads;
    return ads;
}
