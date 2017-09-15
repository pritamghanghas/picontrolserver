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

    MainHandler h;
    int beaconInterval = BEACON_INTERVAL;

    if (qEnvironmentVariableIsSet("picam")) {
        discoveryMessage += " picam";
        h.setPiCamEnabled(true);
    }
    if (qEnvironmentVariableIsSet("uvc_cam")) {
        discoveryMessage += " uvc";
        h.setUvcEnabled(true);
    }

    if (qEnvironmentVariableIsSet("mavproxy")) {
        discoveryMessage += " mavproxy";
        h.setMavProxyEnabled(true);
    }

    if (qEnvironmentVariableIsSet("hostapd")) {
        discoveryMessage += " hostapd";
        h.setMavProxyEnabled(true);
    }

    if (qEnvironmentVariableIsSet("beacon_interval")) {
//        beaconInterval = qEnvironmentVariableIntValue("beacon_interval");
    }
    discoveryMessage += QString(" b%1").arg(beaconInterval);

    qDebug() << "beacon message is " << beaconInterval;

    Tufao::HttpServerRequestRouter router{
        {QRegularExpression{"^/([^/]+)$"}, h},
        {QRegularExpression{".*"}, h}
    };

    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     &router, &Tufao::HttpServerRequestRouter::handleRequest);

    server.listen(QHostAddress::Any, 8080);

    PiDiscoveryBeacon beacon(discoveryMessage, beaconInterval);
    Q_UNUSED(beacon)

    return a.exec();
}
