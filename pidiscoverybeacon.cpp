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
    QStringList subnets = broadcastSubnet();

    QString message = m_discoveryMessage;
    message += QString(" ser%1").arg(m_seqNumber);
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
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    QStringList ads;
    Q_FOREACH(QNetworkInterface interface, interfaces) {
        if (!m_activeInterfaces.contains(interface.name())) {
            continue;
        }

        QList<QHostAddress> interfaceAddress = interface.allAddresses();

         Q_FOREACH(QHostAddress address, interfaceAddress) {

            if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
                ads.append(address.toString());
            }
        }
    }
    qDebug() << "got the following addresses for the device" << ads;
    return ads;
}
