#ifndef PIDISCOVERYBEACON_H
#define PIDISCOVERYBEACON_H

enum NodeCaps {
    NONE        = 0x00,
    PICAM       = 0x01,
    UVC         = 0x03,
    LEPTON      = 0x07,
    SEEK        = 0x0F,
    MAVTCP      = 0x1f,
    MAVUDP      = 0x3F,
    AP          = 0x7F,
    SIKRADIO    = 0xFF,
};

#include <QObject>
#include <QTimer>
#include <QtNetwork/QUdpSocket>

class PiDiscoveryBeacon : public QObject
{
    Q_OBJECT
public:
    explicit PiDiscoveryBeacon(const QString &discoveryMessage, const QStringList interfaces, int discoveryInterval, QObject *parent = 0);
    QStringList deviceAddress();

private slots:
    void sendHelloDataGram();

private:
    QStringList broadcastSubnet();
    QString     m_discoveryMessage;
    QStringList m_activeInterfaces;
    QTimer      m_discoveryIntervalTimer;
    QUdpSocket  m_socket;
    quint32     m_seqNumber;
};

#endif // PIDISCOVERYBEACON_H
