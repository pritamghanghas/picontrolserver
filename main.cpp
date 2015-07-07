#include <QCoreApplication>
#include <Tufao/HttpServer>
#include <QtCore/QUrl>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerRequestRouter>
#include <Tufao/Headers>
#include "mainhandler.h"
#include "pidiscoverybeacon.h"

#define BEACON_INTERVAL 30*1000 // 30sec
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Tufao::HttpServer server;
    QString discoveryMessage("raspberry picam");

    MainHandler h;
    if (qEnvironmentVariableIsSet("thermal_cam")) {
        discoveryMessage += " thermal";
        h.setThermalEnabled(true);
    }

    if (qEnvironmentVariableIsEmpty("mavproxy")) {
        discoveryMessage += " mavproxy";
        h.setMavProxyEnabled(true);
    }

    Tufao::HttpServerRequestRouter router{
        {QRegularExpression{"^/([^/]+)$"}, h},
        {QRegularExpression{".*"}, h}
    };

    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     &router, &Tufao::HttpServerRequestRouter::handleRequest);

    server.listen(QHostAddress::Any, 8080);

    PiDiscoveryBeacon beacon(discoveryMessage, BEACON_INTERVAL);
    Q_UNUSED(beacon)

    return a.exec();
}
