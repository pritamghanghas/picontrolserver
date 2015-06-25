/*
  Copyright (c) 2012, 2013 Vinícius dos Santos Oliveira

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  */

#include "mainhandler.h"
#include <Tufao/HttpServerRequest>
#include <Tufao/Headers>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerRequestRouter>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QProcess>
#include <QFile>

const char * THERMAL_URL_FRAGMENT       = "thermalcam";
const char * PICAM_URL_FRAGMENT         = "picam";
const char * SHUTDOWN_URL_FRAGMENT      = "shutdown";
const char * PICAM_COMMAND_QUERY        = "command";
const char * LEPTON_MJPEG_SERVER_COMMAND= "/home/pi/hermal_mjpeg_streamer/hermal_mjpeg_streamer";
const char * SHUTDOWN_COMMAND           = "sudo halt";
const char * USAGE_JSON_PATH            = "usage.json";

MainHandler::MainHandler(QObject *parent) : QObject(parent),
   m_hasThermal(false), m_thermalProcess(0), m_picamProcess(0)
{

}

bool MainHandler::handleRequest(Tufao::HttpServerRequest &request,
                                Tufao::HttpServerResponse &response)
{
    response.writeHead(Tufao::HttpResponseStatus::OK);
    response.headers().replace("Content-Type", "text/html; charset=utf-8");

    qDebug() << "request debug " << request.url().toString();
    const QString urlString = request.url().toString();

    if (urlString.contains(SHUTDOWN_URL_FRAGMENT)) {
        shutdownHandler(request,response);
    }

    if (urlString.contains(THERMAL_URL_FRAGMENT)) {
        thermalHandler(request,response);
    }

    if (urlString.contains(PICAM_URL_FRAGMENT)) {
        picameraHandler(request,response);
    }

    printUsage(request,response);

    return true;
}

void MainHandler::thermalHandler(Tufao::HttpServerRequest &request,
                    Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    if (!m_hasThermal) {
        response << "thermal not enabled";
        response.end();
        return;
    }

    QString command = LEPTON_MJPEG_SERVER_COMMAND;
//    QString command = "vi /tmp/faltu";

    if (m_thermalProcess) {
        qDebug() << "calling terminate on the current thermal process";
        m_thermalProcess->disconnect();
        m_thermalProcess->terminate();
        m_thermalProcess->waitForFinished();
        thermalProcessFinished();
    }

    if (!m_thermalProcess) {
        qDebug() << "starting a new thermal process";
        m_thermalProcess = new QProcess(this);
        connect(m_thermalProcess, SIGNAL(finished(int)), SLOT(thermalProcessFinished()));
        connect(m_thermalProcess, SIGNAL(destroyed()), SLOT(thermalProcessFinished()));
        m_thermalProcess->start(command);
    }

    response << "started thermal server with command : " << command.toUtf8();
    response.end();
}

void MainHandler::thermalProcessFinished()
{
    qDebug() << "thermal process finished";
    m_thermalProcess->deleteLater();
    m_thermalProcess = 0;
}

void MainHandler::picameraHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    auto url = request.url();
    QUrlQuery queries(url);
    if (!queries.hasQueryItem(PICAM_COMMAND_QUERY)) {
        response << "command for picam missing";
        response.end();
        return;
    }

    auto piCamCommand = queries.queryItemValue(PICAM_COMMAND_QUERY);

    if (m_picamProcess) {
        qDebug() << "calling terminate on current picam pipeline";
        m_picamProcess->disconnect();
        m_picamProcess->terminate();
        m_picamProcess->waitForFinished();
        piCamProcessFinished();
    }

    if (!m_picamProcess) {
        qDebug() << "starting a new picam pipeline" << piCamCommand;
        m_picamProcess = new QProcess(this);
        m_picamProcess->setStandardOutputFile("./picam.out");
        m_picamProcess->setStandardErrorFile("./picam.err");
        connect(m_picamProcess, SIGNAL(finished(int)), SLOT(piCamProcessFinished()));
        connect(m_picamProcess, SIGNAL(destroyed()), SLOT(piCamProcessFinished()));
        m_picamProcess->start(piCamCommand);
    }
    response << "starting the server with command : " << piCamCommand.toUtf8();

    response.end();
}

void MainHandler::piCamProcessFinished()
{
    qDebug() << "recieved picam process finsihed or termintaed";
    m_picamProcess->deleteLater();
    m_picamProcess = 0;
}


void MainHandler::shutdownHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)
    response << "shutting down";
    response.end();
    QProcess *shutdownProcess = new QProcess(this);
    connect(shutdownProcess, SIGNAL(finished(int)), shutdownProcess, SLOT(deleteLater()));
    connect(shutdownProcess, SIGNAL(destroyed()), shutdownProcess, SLOT(deleteLater()));
    shutdownProcess->start(SHUTDOWN_COMMAND);
}

void MainHandler::printUsage(Tufao::HttpServerRequest &request,
                Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    QFile file(USAGE_JSON_PATH);
    file.open(QIODevice::ReadOnly);
    response << file.readAll();
    response.end();
}
