#include "pidiscoverybeacon.h"

PiDiscoveryBeacon::PiDiscoveryBeacon(const QString &discoveryMessage, int discoveryInterval, QObject *parent) :
    m_discoveryMessage(discoveryMessage),
    QObject(parent)
{
//    m_socket.bind(31311);
    sendHelloDataGram();

    m_discoveryIntervalTimer.setInterval(discoveryInterval);
    //m_discoveryIntervalTimer.setSingleShot(true);
    connect(&m_discoveryIntervalTimer, SIGNAL(timeout()), SLOT(sendHelloDataGram()));
    m_discoveryIntervalTimer.start();
}

void PiDiscoveryBeacon::sendHelloDataGram()
{
    qDebug() << "sending hello datagram: " << m_discoveryMessage;
    QByteArray helloDatagram(m_discoveryMessage.toUtf8());
    qint64 rc = m_socket.writeDatagram(helloDatagram, QHostAddress("192.168.42.255"), 31311);
    if (rc == -1) {
        qDebug() << "failed to send hello datagram";
    }
}

