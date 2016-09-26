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
#include "pidiscoverybeacon.h"

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
#include <QString>

const char * UVC_URL_FRAGMENT           = "uvccam";
const char * PICAM_URL_FRAGMENT         = "picam";
const char * OSCONTROL_URL_FRAGMENT     = "os_command";
const char * HOSTAPD_GET_FRAGMENT       = "hostapdget";
const char * HOSTAPD_SET_FRAGMENT       = "hostapdset";
const char * HOSTAPD_CONF_FILE          = "/etc/hostapd/hostapd.conf";
const char * COMMAND_QUERY              = "command";
const char * USAGE_JSON_PATH            = "usage.json";
const char * TERMINATE_COMMAND          = "terminate";
const char * MAVPROXY_FRAGMENT          = "mavproxy";

MainHandler::MainHandler(QObject *parent) : QObject(parent),
   m_hasUvc(false), m_uvcProcess(0), m_picamProcess(0), m_mavproxyProcess(0), m_gstProcess(0)
{
    m_hostApdConfig.insert("ssid", "");
    m_hostApdConfig.insert("wpa_passphrase", "");
    m_hostApdConfig.insert("channel", "");
    parseHostAPDConf();
}

bool MainHandler::handleRequest(Tufao::HttpServerRequest &request,
                                Tufao::HttpServerResponse &response)
{
    response.writeHead(Tufao::HttpResponseStatus::OK);
    response.headers().replace("Content-Type", "text/html; charset=utf-8");

    qDebug() << "request debug " << request.url().toString();
    const QString urlString = request.url().toString();

    if (urlString.contains(OSCONTROL_URL_FRAGMENT)) {
        oscontrolHandler(request,response);
    }

    if (urlString.contains(UVC_URL_FRAGMENT)) {
        uvcHandler(request,response);
    }

    if (urlString.contains(PICAM_URL_FRAGMENT)) {
        picameraHandler(request,response);
    }

    if (urlString.contains(MAVPROXY_FRAGMENT)) {
        mavproxyHandler(request,response);
    }

    if (urlString.contains(HOSTAPD_GET_FRAGMENT)) {
        hostAPDGetHandler(request,response);
    }

    if (urlString.contains(HOSTAPD_SET_FRAGMENT)) {
        hostAPDSetHandler(request,response);
    }

    printUsage(request,response);

    return true;
}

void MainHandler::hostAPDGetHandler(Tufao::HttpServerRequest &request,
                                    Tufao::HttpServerResponse &response)
{
    QString responseText;
    foreach(QString key, m_hostApdConfig.keys()) {
        responseText += key + "=" + m_hostApdConfig.value(key) + "\n";
    }

    qDebug() << "hostapd config response text: " << responseText;
    response << responseText.toStdString().c_str();
    response.end();
    return;
}

void MainHandler::hostAPDSetHandler(Tufao::HttpServerRequest &request,
                                    Tufao::HttpServerResponse &response)
{
    QUrlQuery queries(request.url());

    foreach(QString key, m_hostApdConfig.keys()) {
        if (queries.hasQueryItem(key)) {
            m_hostApdConfig.insert(key, queries.queryItemValue(key));
        }
    }

    qDebug() << "after update config is " << m_hostApdConfig;

    writeHostAPDConf();
    response << "hostapd configuration file update, the new configuration requires reboot of drone os";
    response.end();
}

void MainHandler::parseHostAPDConf()
{
    QFile confFile(HOSTAPD_CONF_FILE);

    if (!confFile.exists()) {
        return;
    }

    if (!confFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream in(&confFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList pair;
        if(!line.startsWith('#')) {
            foreach(QString key, m_hostApdConfig.keys()) {
                if (line.contains(key)) {
                    QStringList pair = line.split("=");
                    if (pair.size() !=2 ) {
                        qWarning("something is critically wrong in conf file, we should think about aborting here");
                    }
                    qDebug() << "line of interest" << line;
                    m_hostApdConfig.insert(key, pair.last());
                }
            }
        }
    }
    qDebug() << m_hostApdConfig;
}

void MainHandler::writeHostAPDConf()
{
    QFile confFile(HOSTAPD_CONF_FILE);

    if (!confFile.exists()) {
        return;
    }

    if (!confFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QString outBuffer;
    {
        QTextStream in(&confFile);
        QTextStream outText(&outBuffer);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList pair;
            bool lineProcessed = false;
            if(!line.startsWith('#')) {
                foreach(QString key, m_hostApdConfig.keys()) {
                    if (line.contains(key)) {
                        outText << key << "=" << m_hostApdConfig.value(key) << "\n";
                        lineProcessed = true;
                    }
                }
            }

            if (!lineProcessed) {
                outText << line << "\n";
            }
        }
    }

    confFile.close();

    if (!confFile.open(QIODevice::WriteOnly)) {
        qWarning("couldn't open hostapd config file for writing");
        return;
    }

    QTextStream out(&confFile);
    out << outBuffer.toStdString().c_str();
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
    if (!queries.hasQueryItem(COMMAND_QUERY)) {
        response << "command for mavproxy missing";
        response.end();
        return;
    }

    auto mavProxyCommand = queries.queryItemValue(COMMAND_QUERY);

    if (mavProxyCommand.contains(TERMINATE_COMMAND) && m_mavproxyProcess) {
        terminateProcess(m_mavproxyProcess);
        mavPoxyProcessFinished();
        response << "mavproxy process terminated";
        response.end();
        return;
    }

    if (m_mavproxyProcess) {
        qDebug() << "calling terminate on the current mavproxy process";
        terminateProcess(m_mavproxyProcess);
        mavPoxyProcessFinished();
        response << "old mavproxy process finsihsed";
    }

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

void MainHandler::uvcHandler(Tufao::HttpServerRequest &request,
                    Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    if (!m_hasUvc) {
        response << "uvc not enabled";
        response.end();
        return;
    }

    QUrlQuery queries(request.url());
    if (!queries.hasQueryItem(COMMAND_QUERY)) {
        response << "command for uvc missing";
        response.end();
        return;
    }

    auto uvcCommand = queries.queryItemValue(COMMAND_QUERY);


    if (uvcCommand.contains(TERMINATE_COMMAND) && m_uvcProcess) {
        terminateProcess(m_uvcProcess);
        uvcProcessFinished();
        response << "uvc process terminated";
        response.end();
        return;
    }

    if (m_uvcProcess) {
        qDebug() << "calling terminate on the current uvc process";
        terminateProcess(m_uvcProcess);
        uvcProcessFinished();
        response << "old uvc process finsihsed";
    }

    if (!m_uvcProcess) {
        qDebug() << "starting a new uvc process";
        m_uvcProcess = new QProcess(this);
        connect(m_uvcProcess, SIGNAL(finished(int)), SLOT(uvcProcessFinished()));
        connect(m_uvcProcess, SIGNAL(destroyed()), SLOT(uvcProcessFinished()));
        m_uvcProcess->start(uvcCommand);
    }

    response << "started uvc server with command : " << uvcCommand.toUtf8();
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

void MainHandler::uvcProcessFinished()
{
    qDebug() << "uvc process finished";
    m_uvcProcess->deleteLater();
    m_uvcProcess = 0;
}

void MainHandler::mavPoxyProcessFinished()
{
    qDebug() << "mav proxy process finished" << m_mavproxyProcess->readAll();
    m_mavproxyProcess->deleteLater();
    m_mavproxyProcess = 0;
}

void MainHandler::picameraHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    QUrl url = request.url();
    QUrlQuery queries(url);
    if (!queries.hasQueryItem(COMMAND_QUERY)) {
        response << "command for picam missing";
        response.end();
        return;
    }

    auto piCamCommand = queries.queryItemValue(COMMAND_QUERY);

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
        m_gstProcess->setStandardErrorFile("./picam_gst.out");
        m_gstProcess->setStandardErrorFile("./picam_gst.err");
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


void MainHandler::oscontrolHandler(Tufao::HttpServerRequest &request,
                     Tufao::HttpServerResponse &response)
{
    QUrl url = request.url();
    QUrlQuery queries(url);
    if (!queries.hasQueryItem(COMMAND_QUERY)) {
        response << "command query missing";
        response.end();
        return;
    }

    QString command = queries.queryItemValue(COMMAND_QUERY);
    response << "executing command : " << command.toUtf8();
    response.end();

    QProcess *osProcess = new QProcess(this);
    connect(osProcess, SIGNAL(finished(int)), osProcess, SLOT(deleteLater()));
    connect(osProcess, SIGNAL(destroyed()), osProcess, SLOT(deleteLater()));
    osProcess->start(command);
}

void MainHandler::printUsage(Tufao::HttpServerRequest &request,
                Tufao::HttpServerResponse &response)
{
    Q_UNUSED(request)

    QFile file(USAGE_JSON_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        response << "couldn't open usage json file : " << USAGE_JSON_PATH;
        response.end();
        return;
    }
    QString jsonData = file.readAll();
    response.headers().insert("Content-Type", "application/json");
    jsonData.replace("$PI_ADDRESS", PiDiscoveryBeacon::deviceAddress());
    response << jsonData.toUtf8();
    response.end();
}
