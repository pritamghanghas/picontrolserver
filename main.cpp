#include <QCoreApplication>
#include <Tufao/HttpServer>
#include <QtCore/QUrl>
#include <Tufao/HttpServerRequest>
#include <Tufao/HttpServerRequestRouter>
#include <Tufao/Headers>
#include "mainhandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Tufao::HttpServer server;

    MainHandler h;
    if (qEnvironmentVariableIsSet("thermal_cam")) {
        h.setThermalEnabled(true);
    }

    Tufao::HttpServerRequestRouter router{
        {QRegularExpression{"^/([^/]+)$"}, h},
        {QRegularExpression{".*"}, h}
    };

    QObject::connect(&server, &Tufao::HttpServer::requestReady,
                     &router, &Tufao::HttpServerRequestRouter::handleRequest);

    server.listen(QHostAddress::Any, 8080);

    return a.exec();
}
