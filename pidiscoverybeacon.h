#ifndef PIDISCOVERYBEACON_H
#define PIDISCOVERYBEACON_H

enum NodeCaps {
    NONE        = 0,
    PICAM       = 1,
    UVC         = 1 << 1,
    LEPTON      = 1 << 2,
    SEEK        = 1 << 3,
    MAVTCP      = 1 << 4,
    MAVUDP      = 1 << 5,
    AP          = 1 << 6,
    SIKRADIO    = 1 << 7,
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
