#ifndef PIDISCOVERYBEACON_H
#define PIDISCOVERYBEACON_H

#include <QObject>
#include <QTimer>
#include <QtNetwork/QUdpSocket>

class PiDiscoveryBeacon : public QObject
{
    Q_OBJECT
public:
    explicit PiDiscoveryBeacon(const QString &discoveryMessage, int discoveryInterval, QObject *parent = 0);
    static QString deviceAddress();

private slots:
    void sendHelloDataGram();

private:
    QString     broadcastSubnet();
    QString     m_discoveryMessage;
    QTimer      m_discoveryIntervalTimer;
    QUdpSocket  m_socket;
};

#endif // PIDISCOVERYBEACON_H
