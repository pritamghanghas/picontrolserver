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
    QString subnet = broadcastSubnet();
    qDebug() << "sending hello datagram: " << m_discoveryMessage << "to subnet: " << subnet;
    QByteArray helloDatagram(m_discoveryMessage.toUtf8());


    qint64 rc = m_socket.writeDatagram(helloDatagram, QHostAddress(subnet), 31311);
    if (rc == -1) {
        qDebug() << "failed to send hello datagram";
    }
}

QString PiDiscoveryBeacon::broadcastSubnet()
{
    QString address = deviceAddress();
    QStringList segments = address.split('.');
    if (segments.count() < 4) {
        return "0.0.0.0";
    }
    return segments.at(0) + '.' + segments.at(1) + '.' + segments.at(2) + '.' + "255";
}

QString PiDiscoveryBeacon::deviceAddress()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QHostAddress addr;
    Q_FOREACH(QHostAddress address, addresses) {
        if (address.isLoopback()) {
            continue;
        }

        if (address.protocol() == QAbstractSocket::IPv4Protocol) {
            addr = address;
            break;
	}
    }

    QString address = addr.toString();
    qDebug() << "got the following address for the device" << address;
    return address;
}
