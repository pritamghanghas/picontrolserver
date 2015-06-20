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

MainHandler::MainHandler(QObject *parent) :
    defaultPiCamCommand("raspivid -t 0 -h 720 -w 1280 -fps 25 -hf -b 2000000 -o - | gst-launch-1.0 -v fdsrc ! h264parse ! rtph264pay config-interval=1 pt=96 ! gdppay ! udpsink host=192.168.1.1 port=5001")
  ,m_thermalProcess(0)
  ,m_picamProcess(0)
  ,m_hasThermal(false)
  ,QObject(parent)
{

}

bool MainHandler::handleRequest(Tufao::HttpServerRequest &request,
                                Tufao::HttpServerResponse &response)
{
    response.writeHead(Tufao::HttpResponseStatus::OK);
    response.headers().replace("Content-Type", "text/html; charset=utf-8");

    auto serverIp = request.headers().value("Host").split(':').first();
    qDebug() << "server ip " << serverIp;


//    qDebug() << "request debug " << request.url().toString();
    const QString urlString = request.url().toString();

    if (urlString.contains("shutdown")) {
        shutdownHandler(request,response);
    }

    if (urlString.contains("thermalcam")) {
        thermalHandler(request,response);
    }

    if (urlString.contains("picam")) {
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

    QString command = "/home/pi/LeptonModule/4624820/simple_mjpeg_streamer_http_server";

    if (!m_thermalProcess) {
        m_thermalProcess = new QProcess(this);
        connect(m_thermalProcess, SIGNAL(finished(int)), SLOT(thermalProcessFinished()));
        connect(m_thermalProcess, SIGNAL(destroyed()), SLOT(thermalProcsessFinished()));
        m_thermalProcess->start(command);
    }

    response << "started thermal server with command : " << command.toUtf8();
    response.end();
}

void MainHandler::thermalProcessFinished()
{
    m_thermalProcess->deleteLater();
    m_thermalProcess = 0;
}

void MainHandler::picameraHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    auto url = request.url();
    auto piCamCommand = defaultPiCamCommand;
    QUrlQuery queries(url);
    if (queries.hasQueryItem("command")) {
        piCamCommand = queries.queryItemValue("command");
    }

    if (!m_picamProcess) {
        m_picamProcess = new QProcess(this);
        connect(m_picamProcess, SIGNAL(finished(int)), SLOT(piCamProcessFinished()));
        connect(m_picamProcess, SIGNAL(destroyed()), SLOT(piCamProcessFinished()));
        m_picamProcess->start(piCamCommand);
    }
    response << "starting the server with command : " << piCamCommand.toUtf8();

    response.end();
}

void MainHandler::piCamProcessFinished()
{
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
    shutdownProcess->start("sudo halt");
}

void MainHandler::printUsage(Tufao::HttpServerRequest &request,
                Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    QFile file("usage.json");
    file.open(QIODevice::ReadOnly);
    response << file.readAll();
    response.end();
}
