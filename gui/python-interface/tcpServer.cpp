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

#include "tcpServer.h"
#include <typeinfo>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <type_traits>

using namespace std;
using namespace HTTP;
using json = nlohmann::json;


//! \brief parses a string in csv format into a map
//! \param data_csv: std::string in csv format (colname1, colname2 \n c11, c12 \n c12, c22)
//! \param data_map: map with the i'th column-name as key and the (i+1, ..., n) column in a vector as value (colname1:{c11,c12})
void parseCSV(string& data_csv, map<string, vector<double>>& data_map){
    istringstream rows;
    rows.str(data_csv);
    vector<string> varNames;
    bool first = 1;
    vector<double> allValues;

    for(string row; getline(rows, row);){
        vector<string> vals;
        char splitBy=',';
        splitStr(row,vals,splitBy);
        for (std::vector<string>::iterator val = begin(vals); val != end(vals); ++val){
            if(first){
                varNames.push_back(*val);
            }else{
                allValues.push_back(atof((*val).c_str()));
            }
        }
        first=0;
    }

    for(size_t i=0; i < varNames.size(); ++i){
        vector<double> currValues;
        for(size_t j = 0+i; j < allValues.size(); j += varNames.size()){
            currValues.push_back(allValues[j]);
        }
        data_map.insert(make_pair(varNames[i], currValues));
    }
	
}


/*  Python/curl interface process:
 *
 *
 *
 *
*/

TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{
    server = new QTcpServer(this);

    this->socket = new QTcpSocket();

    // whenever a user connects, it will emit signal
    QObject::connect(server, &QTcpServer::newConnection, this, &TcpServer::newConnection);
    QObject::connect(this->socket, &QTcpSocket::connected, this, &TcpServer::connected);

    in.setDevice(this->socket);

    if(!server->listen(QHostAddress::Any, 8080))
    {
	qDebug() << "[TcpServer::TcpServer] Server could not start";
    }
    else
    {
	qDebug() << "[TcpServer::TcpServer] Server started.";

        QSettings settings;
        QString user = settings.value("userName").toString();
        Provider prov = static_cast<Provider>( settings.value("provider").toInt() );
        if(!settings.value("token").toString().isEmpty()){
            refreshing = true;
            emit authenticateUser(&user, &prov);
        }
    }
}


string TcpServer::statusCodeAsString(httpStatusCode c)
{
    switch (c)
    {
        case c200:
            return "200 OK\r\n";
        case c202:
            return "202 Accepted\r\n";
        case c404:
            return "424 Unknown command\r\n";
        case c424:
            return "424 No mesh loaded\r\n";
        case c500:
            return "500 \r\n";
        case c503:
            return "503 Service Unavailable\r\n";
    }
    return "500 \r\n";
}


void TcpServer::connected()
{
	httpStatusCode statusCode;
	if(this->socket != nullptr && this->socket->isOpen()){
		statusCode = c202;//"\n202 Accepted\r\n";
	}
	else{
		statusCode = c503; //"\n503 Service Unavailable\r\n";
	}

	QString verConnected = QString::fromStdString("HTTP/1.0 ");
        verConnected += QString::fromStdString(TcpServer::statusCodeAsString(statusCode));
        this->socket->write(verConnected.toLocal8Bit());
	this->socket->flush();

        this->socket->waitForBytesWritten(verConnected.toUtf8().size());
	this->socket->waitForReadyRead();
}


void TcpServer::reading(HTTP::Request *request)
{
        cout << "[TcpServer::reading] Reading data..." << endl;

        if(!this->socket->canReadLine()){
            return ;
        }

        QByteArray reqHead;
	// wait for message body
        while(this->socket->canReadLine()){
		QByteArray buffer;
                buffer = this->socket->readLine();
		reqHead += buffer;
		string mess = buffer.toStdString();

		// search for end of message header
		if(reqHead.toStdString().find("\r\n\r\n")!= -1){
                        break;
		}
	}

        cout << "[TcpServer::reading] Received Request: " << endl;
        qDebug() << "Header: " << reqHead << "\n";
	request->httpParser(QString(reqHead).toStdString());

	QByteArray reqBody;
        while(this->socket->canReadLine()){
                reqBody += this->socket->readLine();

	}
	qDebug() << "Body: " << reqBody << "\n";
	request->setBody(reqBody.toStdString());

	request->info();
}


void TcpServer::sending(QStringList *response)
{
	cout << "[TcpServer::sending] Sending data..." << endl;

	QByteArray mess;
	mess += "HTTP/1.0 ";
	mess += response->at(0).toLocal8Bit(); 
	mess += "\r\n\r\n" ;
	mess += response->at(1).toLocal8Bit();
	mess += "\r\n\r\n" ;

	socket->write(mess);
	socket->flush();

	cout << "[TcpServer::sending] Finished sending." << endl;
}


void TcpServer::newConnection()
{
	this->socket = server->nextPendingConnection();

	emit connected();

	HTTP::Request r;
	emit reading(&r);

	QStringList requestData = parseCommand(r);

	emit sending(&requestData);

	this->socket->close();
}



/*  OAuth log-in process:
 *
 *
 *
 *
*/



bool TcpServer::authenticateUser(QString *username, Provider *provider)
{
    QSettings settings;
    QString clientId = "f31165013adac0da36ed";
    QString clientSecret = "32d6f2a7939c1b40cae13c20a36d4cd32942e60d";
    QString redirectURI = "http://localhost:8080/authorize";
    QString state = "randomString";
    QString authorizationUrlBase;
    QString accessTokenUrlBase;
    string userDataUrlBase;
    QString scope;

    switch(*provider)
    {
        case GITHUB:
            authorizationUrlBase = "https://github.com/login/oauth/authorize";
            accessTokenUrlBase = "https://github.com/login/oauth/access_token";
            userDataUrlBase = "https://api.github.com/user";
            clientId = "f31165013adac0da36ed";
            clientSecret = "32d6f2a7939c1b40cae13c20a36d4cd32942e60d";
            scope = "user";
            break;
        case GITLAB:
            authorizationUrlBase = "https://gitlab.com/oauth/authorize";
            accessTokenUrlBase = "https://gitlab.com/oauth/token";
            userDataUrlBase = "https://gitlab.com/api/v4/user";
            clientId = "0838bf6d76153b656173387328777f369bf7ceb5074138606aacb77dabc66be9";
            clientSecret = "c29a631dbcec4349c38dfa4887a9fc8669d4dbdd9883a4aa9e480d00f4b03832";
            scope = "read_user+api";
            break;
        case ORCID:
            authorizationUrlBase = "https://orcid.org/oauth/authorize";
            accessTokenUrlBase = "https://orcid.org/oauth/token";
            userDataUrlBase = "https://api.gitlab.com/user";
            clientId = "APP-QTDRZ74P88AWIDF8";
            clientSecret = "38c556d3-ec89-4666-aff0-30d2e44c1750";
            scope = "/authenticate";
            redirectURI = "https://localhost:8080/authorize"; // orcid requests http(s)!
            break;
        case REDDIT:
            authorizationUrlBase = "https://www.reddit.com/api/v1/authorize";
            accessTokenUrlBase = "https://www.reddit.com/api/v1/access_token";
            userDataUrlBase = "https://oauth.reddit.com/api/v1/me";// "https://api.gitlab.com/user";
            clientId = "6QVMuM64f_ApLQ";
            clientSecret = "BKHdqqbTEu1pkvbwSlx5_QIUKKwolA";
            scope = "identity";
            break;
        case MATTERMOST:
            authorizationUrlBase = "https://gitlab.rlp.net/oauth/authorize";
            accessTokenUrlBase = "https://gitlab.rlp.net/oauth/token";
            userDataUrlBase = "https://gitlab.rlp.net/api/v4/user";
            clientId = "82ff8b175f9d64aab9a08ca1b9d6b7ba8383c246f106502f74694dd6deef53dc";
            clientSecret = "580282d1147516a99bdde41d2f74be1b17b67cc9bcff8b8ae0aa59859c27b5a3";
            scope = "api";
            break;
        default:
            authorizationUrlBase = "https://github.com/login/oauth/authorize";
            accessTokenUrlBase = "https://github.com/login/oauth/access_token";
            userDataUrlBase = "https://api.github.com/user";
            clientId = "f31165013adac0da36ed";
            clientSecret = "32d6f2a7939c1b40cae13c20a36d4cd32942e60d";
            scope = "user";
    }

    if(!refreshing)
    {
        QString authorizationUrl = authorizationUrlBase + "?response_type=code&client_id=" + clientId +
                "&scope=" + scope + "&login=" + username + "&redirect_uri=" + redirectURI + "&state=" +
                state + "&duration=permanent";

        qDebug() << "[TcpServer::RequestAuthentication] Via: " << authorizationUrl;

        QDesktopServices::openUrl(QUrl(authorizationUrl));

        QObject::connect(this, &TcpServer::codeReceived, this, [=](string code){

            QSettings settings;
            settings.setValue("code", QString::fromStdString(code));
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            cout << "[TcpServer:authenticateUser] Code Received: " << code << endl;

            QString pars = "?clientId=" + clientId + "&clientSecret=" + clientSecret + "&code=" +
                    QString::fromStdString(code) + "&redirect_uri=" + redirectURI;
            QString accessTokenUrl = accessTokenUrlBase + pars;
            qDebug() << "[TcpServer:RequestAccessToken] Via: " << accessTokenUrl;

            QNetworkRequest request(accessTokenUrlBase);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            request.setRawHeader(QByteArray("WWW-Authenticate"),  QByteArray::fromStdString("code " + code));

            QUrlQuery params;
            params.addQueryItem("client_id", clientId);
            params.addQueryItem("client_secret", clientSecret);
            params.addQueryItem("code", QString::fromStdString(code));
            params.addQueryItem("redirect_uri", redirectURI);
            params.addQueryItem("grant_type", "authorization_code"); //needed for gitlab.rlp.net

            QObject::connect(manager, &QNetworkAccessManager::finished, this, &TcpServer::readToken);

            manager->post(request, params.query().toUtf8());
        });
    }

    QObject::connect(this, &TcpServer::tokenReceived, this, [=](string token){
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);

        cout << "[TcpServer:authenticateUser] Token Received: " << token << endl;
        QSettings settings;
        settings.setValue( "token", QString::fromStdString(token));

        QString userDataUrl = QString::fromStdString(userDataUrlBase + "?token=" + token);
        if(settings.value("provider") == 4 || 1) userDataUrl = QString::fromStdString(userDataUrlBase +
                    "?access_token=" + token);

        QNetworkRequest request(userDataUrl);
        qDebug() << "[TcpServer::authenticateUser] Requesting User Data via: " << userDataUrl;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setRawHeader(QByteArray("Authorization"),  QByteArray::fromStdString("token " + token));

        QObject::connect(manager, &QNetworkAccessManager::finished, this, &TcpServer::readUserData);

        manager->get(request);
    });

    if(refreshing){
        cout << "[TcpServer::authenticateUser] Refreshing Authentication" << endl;
        QSettings settings;
        emit tokenReceived(settings.value("token").toString().toStdString());
    }

    QObject::connect(this, &TcpServer::userDataReceived, this, [=](QJsonObject data){
        cout << "[TcpServer::authenticateUser] User Data Received" << endl;
        emit mainWin->authenticated(data);
    });

    this->refreshing = false;

    return true;
}


void TcpServer::readToken(QNetworkReply *reply)
{
    QByteArray bts = reply->readAll();
    QString str(bts);
    qDebug() << "[TcpServer:readReply] Received Reply: " << str;

    // if reply contains parameters as text
    if(str.contains("access_token=", Qt::CaseSensitive)){
        int start = str.indexOf("access_token=", 1);
        int end = str.indexOf("&", 1);

        QString token = str.mid(start+14, end-start-14);
        emit tokenReceived(token.toStdString());
        return;
    }

    // if reply contains data as json
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bts);
    if(jsonDoc.isEmpty()){
        cout << "[TcpServer::readToken] No json provided." << endl;
    }else{
        if(!jsonDoc.object().value("access_token").isObject()){
            emit tokenReceived(jsonDoc.object().value("access_token").toString().toStdString());
        }
    }
    return;
}


void TcpServer::readUserData(QNetworkReply *reply)
{
    const auto bts = reply->readAll();

    const auto document = QJsonDocument::fromJson(bts);
    QString str(bts);
    qDebug() << "[TcpServer::readUserData] Received User Data: " << str;
    Q_ASSERT(document.isObject());
    const auto rootObject = document.object();
    const auto dataValue = rootObject.value("data");
    Q_ASSERT(dataValue.isObject());
    const auto dataObject = dataValue.toObject();

    if(!document.isEmpty()){
        emit userDataReceived(rootObject);
    }else{
        qDebug() << "[TcpServer::readUserData] Json object is empty.";
    }

}


void TcpServer::load(std::map<std::string,std::string>& parameters){
    QString meshName;
    if(parameters.find("filename") != parameters.end()){
        meshName = QString::fromStdString(parameters["filename"]);
    }else{
        cout << "[QGMMainWindow::parseCommand] Missing Data Path - "
                "Set to Default ('../../testdata/cube.obj')." << endl;
        meshName = QString::fromStdString("../../testdata/cube.obj");
    }

    this->mainWin->load(meshName);

    statusCode = c200;
}

void TcpServer::exportVertices(map<string,string>& parameters){

    if(this->mainWin->getWidget()->getMesh() != nullptr){
        QString path;
        if(parameters.find("filename") != parameters.end()){
            path = QString::fromStdString(parameters["filename"]);
        }else{
            cout << "[QGMMainWindow::" << __FUNCTION__ << "] Missing Data Path - "
                    "Set to Default ('../../testdata/cube.obj')." << endl;
            path = QString::fromStdString("../../testdata/cube.obj");
        }

        if(mainWin->getWidget()->getMesh()->exportVertexCoordinatesToCSV(path,false)){
            statusCode = c200;
        }else{
            cout << "[QGMMainWindow::" << __FUNCTION__ << "] Error: Could not export!" << endl;
            statusCode = c500;
        }
    } else {
        std::cout << "[QGMMainWindow::" << __FUNCTION__ << "] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }
}

void TcpServer::getVertices(QVariant& var){
    QJsonArray data_json;
    QString data_str;
    if(mainWin->getWidget()->getMesh() != nullptr){

        std::vector<Vertex*> rVertices;
        this->mainWin->getWidget()->getMesh()->getVertexList(&rVertices );

        vector<Vertex*>::iterator itVertex;
        QJsonArray arr;
        arr.append("vertices_x");
        arr.append("vertices_y");
        arr.append("vertices_z");
        data_json.append(arr);
        data_str += "vertices_x,vertices_y,vertices_z\n";
        for( itVertex=rVertices.begin(); itVertex!=rVertices.end(); itVertex++ ) {
            QJsonArray arr;
            arr.append((*itVertex)->getX());
            arr.append((*itVertex)->getY());
            arr.append((*itVertex)->getZ());
            data_str += QString::number((*itVertex)->getX()) + "," +
                    QString::number((*itVertex)->getY()) + "," + QString::number((*itVertex)->getZ()) + "\n";
            data_json.append(arr);
        }

        statusCode = c200;
    } else {
        std::cout << "[TcpServer::" << __FUNCTION__ << "] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

    QJsonDocument json_doc;
    if(var.type() == 10){ // return-type is QString
        var = data_str;
    }else if(var.type() == 48){ // return-type is QJsonDocument
        json_doc.setArray(data_json);
        var = json_doc;
    }

}

void TcpServer::getMeshVertexNormals(QVariant& var){
    QJsonArray data_json;
    QString data_str;
    if(mainWin->getWidget()->getMesh() != nullptr){

        std::vector<Vector3D> normals;
        this->mainWin->getWidget()->getMesh()->getMeshVertexNormals(&normals);

        data_str += "normals_x,normals_y,normals_z \n";
        for (vector<Vector3D>::iterator normal = normals.begin(); normal != normals.end(); normal++){
            data_json.append(normal->getX());
            data_json.append(normal->getY());
            data_json.append(normal->getZ());
            data_str += QString::number(normal->getX()) + "," + QString::number(normal->getY()) + ","
                    + QString::number(normal->getZ()) + "\n";
        }

        statusCode = c200;
    } else {
        std::cout << "[TcpServer::" << __FUNCTION__ << "] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

    QJsonDocument json_doc;
    if(var.type() == 10){ // return-type is QString
        var = data_str;
    }else if(var.type() == 48){ // return-type is QJsonDocument
        json_doc.setArray(data_json);
        var = json_doc;
    }

}

void TcpServer::getBoundingBoxSize(QVariant& var){
    QJsonArray data_json;
    QString data_str;
    if(mainWin->getWidget()->getMesh() != nullptr){

        Vector3D boxSize;
        this->mainWin->getWidget()->getMesh()->getBoundingBoxSize(boxSize);

        data_json.append(boxSize.getX());
        data_json.append(boxSize.getY());
        data_json.append(boxSize.getZ());
        data_str += QString::number(boxSize.getX()) + "," + QString::number(boxSize.getY()) + ","
                + QString::number(boxSize.getZ()) + "\n";

        statusCode = c200;
    } else {
        std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

    QJsonDocument json_doc;
    if(var.type() == 10){ // return-type is QString
        var = data_str;
    }else if(var.type() == 48){ // return-type is QJsonDocument
        json_doc.setArray(data_json);
        var = json_doc;
    }

}

void TcpServer::getVerticesInBeam(map<string,vector<double>> bodyPars, QVariant& var){
    QJsonArray data_json;
    QString data_str;
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;

        vector<double> vertices_x;
        if(bodyPars.find("vertices_x") != bodyPars.end()){
            vertices_x = bodyPars["vertices_x"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing x coordinates of vertices." << endl;
            parSucc = false;
        }
        vector<double> vertices_y;
        if(bodyPars.find("vertices_y") != bodyPars.end()){
            vertices_y = bodyPars["vertices_y"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing y coordinates of vertices." << endl;
            parSucc = false;
        }
        vector<double> vertices_z;
        if(bodyPars.find("vertices_z") != bodyPars.end()){
            vertices_z = bodyPars["vertices_z"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing z coordinates of vertices." << endl;
            parSucc = false;
        }

        vector<double> normals_x;
        if(bodyPars.find("normals_x") != bodyPars.end()){
            normals_x = bodyPars["normals_x"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing x coordinates of normals." << endl;
            parSucc = false;
        }
        vector<double> normals_y;
        if(bodyPars.find("normals_y") != bodyPars.end()){
            normals_y = bodyPars["normals_y"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing y coordinates of normals." << endl;
        }
        vector<double> normals_z;
        if(bodyPars.find("normals_z") != bodyPars.end()){
            normals_z = bodyPars["normals_z"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing z coordinates of normals." << endl;
            parSucc = false;
        }

        vector<double> beamSize;
        if(bodyPars.find("beam_size") != bodyPars.end()){
            beamSize = bodyPars["beam_size"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing size of beam." << endl;
            parSucc = false;
        }

        vector<double> beamLength;
        if(bodyPars.find("beam_length") != bodyPars.end()){
            beamLength = bodyPars["beam_length"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing length of beam." << endl;
            parSucc = false;
        }

        if(parSucc){
            data_str += "x,y,z\n";
            int i = 0;
            while(i < vertices_x.size()){

                set<Vertex*> verticesInBeam;
                Vector3D vertex;
                Vector3D normal;

                vertex = Vector3D(double(vertices_x[i]),double(vertices_y[i]),double(vertices_z[i]));
                normal = Vector3D(double(normals_x[i]),double(normals_y[i]),double(normals_z[i]));

                this->mainWin->getWidget()->getMesh()->getVerticesInBeam(vertex,vertex+normal*beamLength[i]*2,
                                                                         beamSize[i],&verticesInBeam);

                QJsonArray arr;
                for (set<Vertex*>::iterator v = verticesInBeam.begin(); v != verticesInBeam.end(); v++){
                    Vertex* vert = *v;
                    arr.append(vert->getX());
                    arr.append(vert->getY());
                    arr.append(vert->getZ());
                    data_str += QString::number(vert->getX()) + "," + QString::number(vert->getY()) + "," +
                            QString::number(vert->getZ()) + "\n";
                }
                data_json.append(arr); //! todo: check if json nested correctly

                i+=1;
            }
            statusCode = c200;
        }else{
            cout << "[QGMMainWindow::parseCommand] Could not execute command due to missing parameter." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

    QJsonDocument json_doc;
    if(var.type() == 10){ // return-type is QString
        var = data_str;
    }else if(var.type() == 48){ // return-type is QJsonDocument
        json_doc.setArray(data_json);
        var = json_doc;
    }

}

void TcpServer::nonMaxSupp(map<string,string>& parameters){
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;
        double nmsDistances;
        if(parameters.find("nms_distance") != parameters.end()){
            nmsDistances = std::stod(parameters["nms_distance"]);
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing NMS distance." << endl;
            parSucc = false;
        }

        if(parSucc){

            if(this->mainWin->getWidget()->getMesh()->funcWeExSuppNonMax(nmsDistances)){
                statusCode = c200;
            }else{
                cout << "[QGMMainWindow::parseCommand] Error in execution of command." << endl;
                statusCode = c424;
            }
        }else{
            cout << "[QGMMainWindow::parseCommand] Could not execute command due to missing parameter." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

}

void TcpServer::watershed(map<string,string>& parameters){
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;
        double delInput;
        if(parameters.find("deletable_input") != parameters.end()){
                delInput = std::stod(parameters["deletable_input"]);
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing deletable input." << endl;
            parSucc = false;
        }

        if(parSucc){

            if(this->mainWin->getWidget()->getMesh()->funcWeExComputeWatershed(delInput)){
                statusCode = c200;
            }else{
                cout << "[QGMMainWindow::parseCommand] Error in execution of command." << endl;
                statusCode = c424;
            }
        }else{
            cout << "[QGMMainWindow::parseCommand] Could not execute command due to missing parameter." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

}

void TcpServer::clustering(map<string,string>& parameters){
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;
        int numbInter;
        if(parameters.find("number_iterations") != parameters.end()){
                numbInter = std::stoi(parameters["number_iterations"]);
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing number of iterations." << endl;
            parSucc = false;
        }

        if(parSucc){

            if(this->mainWin->getWidget()->getMesh()->funcWeExComputeClustering(numbInter)){
                statusCode = c200;
            }else{
                cout << "[QGMMainWindow::parseCommand] Error in execution of command." << endl;
                statusCode = c424;
            }
        }else{
            cout << "[QGMMainWindow::parseCommand] Could not execute command due to missing parameter." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

}

void TcpServer::ransac(map<string,string>& parameters, QVariant& var){
    QJsonArray data_json;
    QString data_str;
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;

        int numbInter;
        if(parameters.find("number_iterations") != parameters.end()){
            numbInter = std::stoi(parameters["number_iterations"]);
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing number of iterations - set to default (100)." << endl;
            numbInter = 100;
            parSucc = false;
        }

        string filename = this->mainWin->getWidget()->getMesh()->getFullName().string();
        if(filename.find(".obj")!=-1) {filename = filename.substr(0,filename.find(".obj"));}
        if(filename.find(".ply")!=-1) {filename = filename.substr(0,filename.find(".ply"));}
        filename += ".wedges";

        if(parSucc){
            //! todo: pars of func changed; access meshIntrinsicExtractedTetraeders
            vector<vector<double>> meshIntrinsicExtractedTetraeders;
            if(this->mainWin->getWidget()->getMesh()->funcWeExComputeRANSAC(numbInter)){
                for (int i = 0; i < meshIntrinsicExtractedTetraeders.size(); i++) {
                    for (int j = 0; j < meshIntrinsicExtractedTetraeders[i].size(); j++){
                        data_str += QString::number(meshIntrinsicExtractedTetraeders[i][j] );
                        if(j != meshIntrinsicExtractedTetraeders[i].size()-1){
                            data_str += ","; //! todo: data as json
                        }
                    }
                    data_str += "\n";
                }
                statusCode = c200;
            }else{
                cout << "[QGMMainWindow::parseCommand] Error in execution of command." << endl;
                statusCode = c424;
            }
        }else{
            cout << "[QGMMainWindow::parseCommand] Executed command with default parameters." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

    QJsonDocument json_doc;
    if(var.type() == 10){ // return-type is QString
        var = data_str;
    }else if(var.type() == 48){ // return-type is QJsonDocument
        json_doc.setArray(data_json);
        var = json_doc;
    }

}

void TcpServer::featureElementsByIndex(map<string,string>& parameters){
    if(mainWin->getWidget()->getMesh() != nullptr){

        bool parSucc = true;
        int numbElement;
        if(parameters.find("element_nr") != parameters.end()){
            numbElement = std::stoi(parameters["element_nr"]);
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing number of elements." << endl;
            parSucc = false;
        }

        if(parSucc){

            if(this->mainWin->getWidget()->getMesh()->funcVertFeatureVecElementByIndex(numbElement)){
                statusCode = c200;
            }else{
                cout << "[QGMMainWindow::parseCommand] Error in execution of command." << endl;
                statusCode = c424;
            }
        }else{
            cout << "[QGMMainWindow::parseCommand] Could not execute command due to missing parameter." << endl;
            statusCode = c424;
        }

    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

}

void TcpServer::assignFeatVec(map<string,vector<double>> bodyPars){
    if(mainWin->getWidget()->getMesh() != nullptr){

        vector<double> funcVals;
        if(bodyPars.find("func_values") != bodyPars.end()){
            funcVals = bodyPars["func_values"];
        }else{
            cout << "[QGMMainWindow::parseCommand] Missing function values for feature vectors." << endl;
        }

        bool succ = this->mainWin->getWidget()->getMesh()->assignFeatureVectors(funcVals, 1);
        if(!succ){
            cout << "[QGMMainWindow::receiveCommand] Could not assign feature values!" << endl;
            statusCode = c424;
        }
        else{
            statusCode = c200;
        }
    } else {
        std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }

}

void TcpServer::compFeatVecLen(){
    if(mainWin->getWidget()->getMesh() != nullptr){

        this->mainWin->actionFeatLengthEuc->trigger();

        statusCode = c200;
    } else {
        std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
        statusCode = c424;
    }
}

void TcpServer::authorize(std::map<std::string,std::string>& parameters, QVariant& var){
    QString data_str;

    string code;
    if(parameters.find("token") != parameters.end()){
        string token = parameters["token"];
        cout << "[QGMMainWindow::parseCommand] Received Token '" << token << "'" << endl;
        emit tokenReceived(token);

        QFile file("../../gui/python-interface/websiteSuccess.html");
        QString dataToSend;
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Current path:" << QDir::currentPath();
            QMessageBox::information(0, "error", file.errorString());
        }else{
            QTextStream in(&file);
            dataToSend = in.readAll();
        }
        data_str = dataToSend;
    }else{
        cout << "[QGMMainWindow::parseCommand] Missing Token." << endl;
        QFile file("../../gui/python-interface/websiteFail.html");
        QString dataToSend;
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Current path:" << QDir::currentPath();
            QMessageBox::information(0, "error", file.errorString());
        }else{
            QTextStream in(&file);
            dataToSend = in.readAll();
        }
        data_str = dataToSend;
    }

    if(parameters.find("code") != parameters.end()){
        code = parameters["code"];
        //cout << "[QGMMainWindow::parseCommand] Received Code '" << code << "'" << endl;
        emit codeReceived(code);

        QFile file("../../gui/python-interface/websiteSuccess.html");
        QString dataToSend;
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Current path:" << QDir::currentPath();
            QMessageBox::information(0, "error", file.errorString());
        }else{
            QTextStream in(&file);
            dataToSend = in.readAll();
        }
        data_str = dataToSend;
    }else{
        cout << "[QGMMainWindow::parseCommand] Missing Code." << endl;
        QFile file("../../gui/python-interface/websiteFail.html");
        QString dataToSend;
        if(!file.open(QIODevice::ReadOnly)){
            qDebug() << "Current path:" << QDir::currentPath();
            QMessageBox::information(0, "error", file.errorString());
        }else{
            QTextStream in(&file);
            dataToSend = in.readAll();
            qDebug() << "[TcpServer::authorizeUser] Opened File: " << dataToSend;
        }
        data_str = dataToSend;
    }

    string returnedState;
    if(parameters.find("state") != parameters.end()){
            returnedState = parameters["state"];
            if(returnedState != "randomString"){
                cout << "[QGMMainWindow::parseCommand] State differs!" << endl;
            }
    }else{
            cout << "[QGMMainWindow::parseCommand] Missing State." << endl;
    }

    var = data_str;
}



QStringList TcpServer::parseCommand(Request req){

        // check if return_type is set in parameters of request, otherwise set to default (csv)
        map<string,string> pars = req.getParameters();
	string return_type = "csv";
        if( HTTP::method_to_string(req.getMethod()) == "GET"){
		if(pars.find("return_type") != pars.end()){
			return_type = pars["return_type"];
		}else{
                        cout << "[TcpServer::" << __FUNCTION__ << "] Missing Return Type - "
                                "Set to Default (csv)." << endl;
		}
        }

        // set QVariant type according to selected return_type
        json data;
        QString data_str;
        QVariant v; // holds data either as QJsonDocument or QString (csv)
        if(return_type == "csv"){
            v = QVariant(QString());
        }else if (return_type == "json") {
            v = QVariant(QJsonDocument());
        }

        // check for additional data in the request body
        string body = req.getBody();
	map<string,vector<double>> bodyPars;
        if(body.size() > 0){
                parseCSV(body, bodyPars); //! data type not secured, json not selectible
	}
	
        // extract command from request
        string command = req.getFunction();
        cout << "[TcpServer::" << __FUNCTION__ << "] Received command: " << command << endl;   

        // search for respective function to call
	if(command == "load"){
            load(pars);
	}
	else if(command == "exportVertices"){
            exportVertices(pars);
	}
        else if(command == "getVertices"){
            getVertices(v);
	}
	else if(command=="getMeshVertexNormals"){
            getMeshVertexNormals(v);
	}
	else if(command=="getBoundingBoxSize"){
            getBoundingBoxSize(v);
	}
	else if(command=="getVerticesInBeam"){
            getVerticesInBeam(bodyPars, v);
	}
        else if(command=="nonMaxSupp"){
            nonMaxSupp(pars);
        }
        else if(command=="watershed"){
            watershed(pars);
        }
        else if(command=="clustering"){
            clustering(pars);
        }
        else if(command=="ransac"){
            ransac(pars, v);
        }
        else if(command=="featureElementsByIndex"){
            featureElementsByIndex(pars);
        }
	else if(command=="assignFeatVec"){
            assignFeatVec(bodyPars);
	}
	else if(command=="compFeatVecLen"){
            compFeatVecLen();
	}
        else if(command=="authorize"){
            authorize(pars, v);
        }
        else{
            std::cout << "[QGMMainWindow::parseCommand] Error: Unknown command!" << std::endl;
            statusCode = c404;
	}

        // dump return data
	QStringList response;
        response << QString::fromStdString(TcpServer::statusCodeAsString(statusCode));
        if(return_type == "json"){
            response << v.toJsonDocument().toJson();
            qDebug() << "[TcpServer::" << __FUNCTION__ << "] Data: " << v.toJsonDocument().toJson();
	}else{
            response << v.toString();
            qDebug() << "[TcpServer::" << __FUNCTION__ << "] Data: " << v.toString();
	}

	return response;
}


void TcpServer::setMainWindow(QGMMainWindow *mainWindow)
{
	this->mainWin = mainWindow;
}


