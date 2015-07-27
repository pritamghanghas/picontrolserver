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
const char * CAM_COMMAND_QUERY          = "command";
const char * LEPTON_MJPEG_SERVER_COMMAND= "/home/pi/thermal_mjpeg_streamer/thermal_mjpeg_streamer";
const char * SHUTDOWN_COMMAND           = "sudo halt";
const char * USAGE_JSON_PATH            = "usage.json";
const char * TERMINATE_COMMAND          = "terminate";
const char * MAVPROXY_FRAGMENT          = "mavproxy";
const char * RASPILOR_FRAGMENT          = "raspilot";

MainHandler::MainHandler(QObject *parent) : QObject(parent),
   m_hasThermal(false), m_thermalProcess(0), m_picamProcess(0), m_mavproxyProcess(0), m_gstProcess(0)
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

    if (urlString.contains(MAVPROXY_FRAGMENT)) {
        mavproxyHandler(request,response);
    }

    printUsage(request,response);

    return true;
}

void MainHandler::mavproxyHandler(Tufao::HttpServerRequest &request,
                                  Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    if (!m_hasMavProxy) {
        response << "mavproxy not enabled";
        response.end();
        return;
    }

    QUrlQuery queries(request.url());
    if (!queries.hasQueryItem(CAM_COMMAND_QUERY)) {
        response << "command for mavproxy missing";
        response.end();
        return;
    }

    auto mavProxyCommand = queries.queryItemValue(CAM_COMMAND_QUERY);

    if (mavProxyCommand.contains(TERMINATE_COMMAND) && m_mavproxyProcess) {
        terminateProcess(m_mavproxyProcess);
        mavPoxyProcessFinished();
        response << "mavproxy process terminated";
        response.end();
        return;
    }

//    if (m_mavproxyProcess) {
//        qDebug() << "calling terminate on the current mavproxy process";
//        terminateProcess(m_mavproxyProcess);
//        mavPoxyProcessFinished();
//        response << "old mavproxy process finsihsed";
//    }

    if (!m_mavproxyProcess) {
        qDebug() << "starting a new mavproxy process" << mavProxyCommand;
        m_mavproxyProcess = new QProcess(this);
        connect(m_mavproxyProcess, SIGNAL(finished(int)), SLOT(mavPoxyProcessFinished()));
        connect(m_mavproxyProcess, SIGNAL(destroyed()), SLOT(mavPoxyProcessFinished()));
        m_mavproxyProcess->start(mavProxyCommand);
        response << "started mav server with command : " << mavProxyCommand.toUtf8();
    }

    response.end();
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

    QUrlQuery queries(request.url());
    if (!queries.hasQueryItem(CAM_COMMAND_QUERY)) {
        response << "command for thermal missing";
        response.end();
        return;
    }

    auto thermalCommand = queries.queryItemValue(CAM_COMMAND_QUERY);


    if (thermalCommand.contains(TERMINATE_COMMAND) && m_thermalProcess) {
        terminateProcess(m_thermalProcess);
        thermalProcessFinished();
        response << "thermal process terminated";
        response.end();
        return;
    }

    QString command = LEPTON_MJPEG_SERVER_COMMAND;
//    QString command = "vi /tmp/faltu";

    if (m_thermalProcess) {
        qDebug() << "calling terminate on the current thermal process";
        terminateProcess(m_thermalProcess);
        thermalProcessFinished();
        response << "old thermal process finsihsed";
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

void MainHandler::terminateProcess(QProcess *process) const
{
    if (process == 0) {
        return;
    }
    process->disconnect();
    process->terminate();
    process->waitForFinished();
}

void MainHandler::thermalProcessFinished()
{
    qDebug() << "thermal process finished";
    m_thermalProcess->deleteLater();
    m_thermalProcess = 0;
}

void MainHandler::mavPoxyProcessFinished()
{
    qDebug() << "mav proxy process finished";
    m_mavproxyProcess->deleteLater();
    m_mavproxyProcess = 0;
}

void MainHandler::picameraHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    QUrl url = request.url();
    QUrlQuery queries(url);
    if (!queries.hasQueryItem(CAM_COMMAND_QUERY)) {
        response << "command for picam missing";
        response.end();
        return;
    }

    auto piCamCommand = queries.queryItemValue(CAM_COMMAND_QUERY);

    if (piCamCommand.contains(TERMINATE_COMMAND) && m_picamProcess) {
        terminateProcess(m_picamProcess);
        piCamProcessFinished();
        response << "picam process terminated";
        response.end();
        return;
    }

    if (m_picamProcess) {
        qDebug() << "calling terminate on current picam pipeline";
        terminateProcess(m_picamProcess);
        piCamProcessFinished();
        response << "picam process terminated";
    }

    if (!m_picamProcess) {
        qDebug() << "starting a new picam pipeline" << piCamCommand;
        m_picamProcess = new QProcess(this);
        m_picamProcess->setStandardErrorFile("./picam.err");
        connect(m_picamProcess, SIGNAL(finished(int)), SLOT(piCamProcessFinished()));
        connect(m_picamProcess, SIGNAL(destroyed()), SLOT(piCamProcessFinished()));
        m_gstProcess = new QProcess(m_picamProcess);
        m_gstProcess->setStandardErrorFile("./gst.out");
        m_gstProcess->setStandardErrorFile("./gst.err");
        m_picamProcess->setStandardOutputProcess(m_gstProcess);
        QStringList commands= piCamCommand.split("|");
        if (commands.size() <2 ) {
            qDebug() << "for picam the process needs a pipe";
            terminateProcess(m_picamProcess);
            return;
        }
        m_gstProcess->start(commands.last());
        m_picamProcess->start(commands.first());
    }
    response << "starting the server with command : " << piCamCommand.toUtf8();

    response.end();
}

void MainHandler::piCamProcessFinished()
{
    qDebug() << "recieved picam process finsihed or termintaed";
    m_picamProcess->disconnect();
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
