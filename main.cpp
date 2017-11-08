#include <QCoreApplication>
#include <Tufao/HttpServer>
#include <QtCore/QUrl>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerRequestRouter>
#include <Tufao/Headers>
#include "mainhandler.h"
#include "pidiscoverybeacon.h"

#define BEACON_INTERVAL 3*1000 // 3sec

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Tufao::HttpServer server;
    QString discoveryMessage("raspberry");
    QString uniqueID = "pi";

    MainHandler h;
    int beaconInterval = BEACON_INTERVAL;
    QString activeInterfaces = "wlan0";

    if (qEnvironmentVariableIsSet("picam")) {
        h.setPiCamEnabled();
    }

    if (qEnvironmentVariableIsSet("uvccam")) {
        h.setUvcEnabled();
    }

    if (qEnvironmentVariableIsSet("mavudp")) {
        h.setMAVUdpEnabled();
    }

    if (qEnvironmentVariableIsSet("hostapd")) {
        h.setHostAPDEnabled();
    }

    if (qEnvironmentVariableIsSet("mavtcp")) {
        h.setMAVTcpEnabled();
    }

    if (qEnvironmentVariableIsSet("sik")) {
        h.setSikEnabled();
    }

    if (qEnvironmentVariableIsSet("lepton")) {
        h.setLeptonEnabled();
    }

    if (qEnvironmentVariableIsSet("seek")) {
        h.setSeekEnabled();
    }

    if (qEnvironmentVariableIsSet("unique_id")) {
        uniqueID = QString::fromLocal8Bit(qgetenv("unique_id"));
    }

    discoveryMessage += QString(" %1").arg(uniqueID);

    QString caps = QString::number(h.getCaps());
    discoveryMessage += QString(" %1").arg(caps);

    if (qEnvironmentVariableIsSet("beacon_interval")) {
        beaconInterval = qEnvironmentVariableIntValue("beacon_interval");
    }

    if (qEnvironmentVariableIsSet("interface")) {
        activeInterfaces = qEnvironmentVariableIntValue("interfaces");
    }

    discoveryMessage += QString(" %1").arg(beaconInterval);


    Tufao::HttpServerRequestRouter router{
        {QRegularExpression{"^/([^/]+)$"}, h},
        {QRegularExpression{".*"}, h}
    };

    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     &router, &Tufao::HttpServerRequestRouter::handleRequest);

    server.listen(QHostAddress::Any, 8080);

    QStringList activeInterfacesList = activeInterfaces.split(",");

    PiDiscoveryBeacon beacon(discoveryMessage, activeInterfacesList, beaconInterval);
    Q_UNUSED(beacon)

    return a.exec();
}
