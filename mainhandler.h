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


class MainHandler : public QObject,
                    public Tufao::AbstractHttpServerRequestHandler
{
    Q_OBJECT
public:
    explicit MainHandler(QObject *parent = 0);

    void setUvcEnabled(bool enabled = true) { m_hasUvc = enabled; }
    void setMavProxyEnabled(bool enabled = true) { m_hasMavProxy = enabled; }
    void setPiCamEnabled(bool enabled = true) { m_hasPiCam = enabled; }

    bool getmavEnabled() { return m_hasMavProxy; }
    bool getUvcEnabled() const { return m_hasUvc; }
    bool getPiCamEnabled() const { return m_hasPiCam; }


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

    bool         m_hasUvc,
                 m_hasPiCam,
                 m_hasMavProxy;
    QProcess    *m_uvcProcess,
                *m_picamProcess,
                *m_gstProcess,
                *m_mavproxyProcess;

    QMap<QString,QString> m_hostApdConfig;
};

#endif // MAINHANDLER_H
