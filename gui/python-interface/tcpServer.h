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
#include "httpParser.h"
#include <json.hpp>
#include <string>
#include <vector>
#include <map>


void parseCSV(std::string& data_csv, std::map<std::string,std::vector<double>>& data_map);


class TcpServer : public QObject
{
    Q_OBJECT
    
public:
    explicit TcpServer(QObject *parent = 0);
    enum httpStatusCode { c200, c202, c404, c424, c500, c503 };
    void setMainWindow(QGMMainWindow *mainWindow);

signals:
	void codeReceived(std::string code);
	void tokenReceived(std::string token);
	void userDataReceived(QJsonObject data);

public slots:
    void newConnection();
    void connected();
    void reading(HTTP::Request *request);
    void sending(QStringList *response);
    std::string statusCodeAsString(httpStatusCode c);
	bool authenticateUser(QString *username);
	void readToken(QNetworkReply *reply);
	void readUserData(QNetworkReply *reply);

    
private:
    QTcpServer *server;
    QGMMainWindow *mainWin;
    QTcpSocket *socket;
    QStringList parseCommand(HTTP::Request r);
	QDataStream in;
    
};


#endif // TCPSERVER_H
