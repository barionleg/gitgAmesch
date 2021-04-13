/* 
 * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "meshwidget_params.h"
#include "httpParser.h"
#include <string>
#include <vector>
#include <map>
#include <QVariant>


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
        void sViewUserInfo(MeshWidgetParams::eViewUserInfo,QString);

    public slots:
        void authenticateUser(QString *username, Provider *provider);

    private slots:
        void newConnection();
        void connected();
        void reading(HTTP::Request *request);
        void sending(QStringList *response);
        std::string statusCodeAsString(httpStatusCode c);
        void readToken(QNetworkReply *reply);
        void readUserData(QNetworkReply *reply);


    private:
        QTcpServer *server;
        QGMMainWindow *mainWin;
        QTcpSocket *socket;
        QStringList parseCommand(HTTP::Request r);
        QDataStream in;
        httpStatusCode statusCode;
        bool refreshing = false;
        void authorize(std::map<std::string,std::string>& parameters, QVariant& var);

        /*	command related functions
         *
         * 	Trigger respective functions in QGMMainWindow or Mesh.
         * 	Function parameters are passed from request header and body.
         *	Return data written into QVariant holding type QJsonDocument or QString.
        */
        void load(std::map<std::string, std::string>& parameters);
        void exportVertices(std::map<std::string, std::string>& parameters);
        void getVertices(QVariant& var);
        void getMeshVertexNormals(QVariant& var);
        void getBoundingBoxSize(QVariant& var);
        void getVerticesInBeam(std::map<std::string, std::vector<double>> bodyPars, QVariant& var);
        void nonMaxSupp(std::map<std::string, std::string>& parameters);
        void watershed(std::map<std::string, std::string>& parameters);
        void clustering(std::map<std::string, std::string>& parameters);
        void ransac(std::map<std::string, std::string>& parameters, QVariant& var);
        void featureElementsByIndex(std::map<std::string, std::string>& parameters);
        void assignFeatVec(std::map<std::string, std::vector<double>> bodyPars);
        void compFeatVecLen();
};


#endif // TCPSERVER_H
