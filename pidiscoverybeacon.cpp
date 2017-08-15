#include "pidiscoverybeacon.h"
#include <QNetworkInterface>

PiDiscoveryBeacon::PiDiscoveryBeacon(const QString &discoveryMessage, int discoveryInterval, QObject *parent) :
    m_discoveryMessage(discoveryMessage),
    QObject(parent)
{
    sendHelloDataGram();

    m_discoveryIntervalTimer.setInterval(discoveryInterval);
    connect(&m_discoveryIntervalTimer, SIGNAL(timeout()), SLOT(sendHelloDataGram()));
    m_discoveryIntervalTimer.start();
}

void PiDiscoveryBeacon::sendHelloDataGram()
{
    QStringList subnets = broadcastSubnet();
    qDebug() << "sending hello datagram: " << m_discoveryMessage << "to subnets: " << subnets;
    QByteArray helloDatagram(m_discoveryMessage.toUtf8());

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
    QStringList subnets;
    QStringList ads = deviceAddress();

    Q_FOREACH(const QString address, ads)
    {
        QStringList segments = address.split('.');
        if (segments.count() < 4) {
            subnets << "0.0.0.0";
            continue;
        }
        subnets << segments.at(0) + '.' + segments.at(1) + '.' + segments.at(2) + '.' + "255";
    }
    return subnets;
}

QStringList PiDiscoveryBeacon::deviceAddress()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QStringList ads;
    Q_FOREACH(QHostAddress address, addresses) {
        if (address.isLoopback()) {
            continue;
        }

        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            ads.append(address.toString());
	}
    }
    qDebug() << "got the following address for the device" << ads;
    return ads;
}
