/*
  Copyright (c) 2012 Vinícius dos Santos Oliveira

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

#ifndef MAINHANDLER_H
#define MAINHANDLER_H

#include <QtCore/QObject>
#include <QProcess>
#include <QMap>
#include <Tufao/AbstractHttpServerRequestHandler>
#include "pidiscoverybeacon.h"


class MainHandler : public QObject,
                    public Tufao::AbstractHttpServerRequestHandler
{
    Q_OBJECT
public:
    explicit MainHandler(QObject *parent = 0);

    void setUvcEnabled()         { m_caps |= NodeCaps::UVC; }
    void setMAVUdpEnabled()      { m_caps |= NodeCaps::MAVUDP; }
    void setPiCamEnabled()       { m_caps |= NodeCaps::PICAM; }
    void setHostAPDEnabled()     { m_caps |= NodeCaps::AP; }
    void setLeptonEnabled()      { m_caps |= NodeCaps::LEPTON; }
    void setSikEnabled()         { m_caps |= NodeCaps::SIKRADIO; }
    void setSeekEnabled()        { m_caps |= NodeCaps::SEEK; }
    void setMAVTcpEnabled()      { m_caps |= NodeCaps::MAVTCP; }

    bool getMAVUdpEnabled() const       { return m_caps & NodeCaps::MAVUDP; }
    bool getUvcEnabled() const          { return m_caps & NodeCaps::UVC; }
    bool getPiCamEnabled() const        { return m_caps & NodeCaps::PICAM; }
    bool getHostAPDEnabled() const      { return m_caps & NodeCaps::AP; }
    bool getLeptonEnabled() const       { return m_caps & NodeCaps::LEPTON; }
    bool getSeekEnabled() const         { return m_caps & NodeCaps::SEEK; }
    bool getSikEnabled() const          { return m_caps & NodeCaps::SIKRADIO; }
    bool getMAVTcpEnabled() const       { return m_caps & NodeCaps::MAVTCP; }
    int getCaps() const                 { return m_caps; }


public slots:
    bool handleRequest(Tufao::HttpServerRequest &request,
                       Tufao::HttpServerResponse &response) override;

private slots:
    void uvcProcessFinished();
    void piCamProcessFinished();
    void mavPoxyProcessFinished();

private:
    void printUsage(Tufao::HttpServerRequest &request,
                    Tufao::HttpServerResponse &response);

    void picameraHandler(Tufao::HttpServerRequest &request,
                         Tufao::HttpServerResponse &response);

    void uvcHandler(Tufao::HttpServerRequest &request,
                        Tufao::HttpServerResponse &response);

    void oscontrolHandler(Tufao::HttpServerRequest &request,
                         Tufao::HttpServerResponse &response);

    void mavproxyHandler(Tufao::HttpServerRequest &request,
                         Tufao::HttpServerResponse &response);

    void hostAPDGetHandler(Tufao::HttpServerRequest &request,
                           Tufao::HttpServerResponse &response);

    void hostAPDSetHandler(Tufao::HttpServerRequest &request,
                           Tufao::HttpServerResponse &response);

    void parseHostAPDConf();

    void writeHostAPDConf();

    void terminateProcess(QProcess *process) const;

    int          m_caps = 0;
    QProcess    *m_uvcProcess,
                *m_picamProcess,
                *m_mavProcess,
                *m_gstProcess;

    QMap<QString,QString> m_hostApdConfig;
};

#endif // MAINHANDLER_H
