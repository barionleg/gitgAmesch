
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QtCore>
#include <QtNetwork>
#include <QGMMainWindow.h>
#include "meshwidget.h"
#include "httpParser.cpp"
#include <json.hpp>
#include <string>

// for convenience
using json = nlohmann::json;

class TcpServer : public QObject
{
    Q_OBJECT
    
public:
    explicit TcpServer(QObject *parent = 0);//,QGMMainWindow *mainWindow = 0);
    enum httpStatusCode { c200, c202, c404, c424, c500, c503 };
    void setMainWindow(QGMMainWindow *mainWindow);

public slots:
    void newConnection();
    void connected();//QTcpSocket *socket);
    void reading(HTTP::Request *request);
    void sending(QStringList *response);
    string statusCodeAsString(httpStatusCode c);
    //void passData(QString);
    
private:
    QTcpServer *server;
    QGMMainWindow *mainWin;
    QTcpSocket *socket;

    QStringList parseCommand(string command,json parameters);//std::vector<std::string> parameters);
    
};


#endif // TCPSERVER_H

