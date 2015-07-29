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
    qDebug() << "sending hello datagram: " << m_discoveryMessage;
    QByteArray helloDatagram(m_discoveryMessage.toUtf8());
    QString address = deviceAddress();
    QStringList octets = address.split('.');
    if (octets.size() < 4) {
        qDebug() << "failed to find proper address to send udp broadcast";
        return;
    }
    QString broadcastAddress = octets[0] + octets[1] + octets[2] + "255";

    qint64 rc = m_socket.writeDatagram(helloDatagram, QHostAddress(broadcastAddress), 31311);
    if (rc == -1) {
        qDebug() << "failed to send hello datagram";
    }
}

QString PiDiscoveryBeacon::deviceAddress()
{
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QHostAddress addr;
    Q_FOREACH(QHostAddress address, addresses) {
        if (address.isLoopback()) {
            continue;
        }
        addr = address;
        break;
    }

    QString address = addr.toString();
    qDebug() << "got the following address for the device" << address;
    return address;
}
