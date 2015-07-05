#include "pidiscoverybeacon.h"

PiDiscoveryBeacon::PiDiscoveryBeacon(const QString &discoveryMessage, int discoveryInterval, QObject *parent) :
    m_discoveryMessage(discoveryMessage),
    QObject(parent)
{
    m_socket.bind(31311);
    sendHelloDataGram();

    m_discoveryIntervalTimer.setInterval(discoveryInterval);
    m_discoveryIntervalTimer.setSingleShot(true);
    connect(&m_discoveryIntervalTimer, SIGNAL(timeout()), SLOT(sendHelloDataGram()));
    m_discoveryIntervalTimer.start();
}

void PiDiscoveryBeacon::sendHelloDataGram()
{
    QByteArray helloDatagram(m_discoveryMessage.toUtf8());
    m_socket.writeDatagram(helloDatagram, QHostAddress::Broadcast, 31311);
}

