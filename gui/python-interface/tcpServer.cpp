#include "tcpServer.h"
#include <typeinfo>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

using namespace std;
using json = nlohmann::json;
using namespace HTTP;


void parseCSV(string& data_csv, map<string,vector<double>>& data_map){
	istringstream rows;
	rows.str(data_csv);
	vector<string> varNames;
	bool first = 1;
	vector<double> allValues;

	for(string row; getline(rows,row);){
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

	for(size_t i=0; i<varNames.size();++i){
		vector<double> currValues;
		//cout << varNames[i] << ": "; 
		for(size_t j=0+i; j<allValues.size();j+=varNames.size()){
			currValues.push_back(allValues[j]);
			//cout << allValues[j] << ", "; 
		}
		data_map.insert(make_pair(varNames[i],currValues));
		//cout << endl; 
	}
	
}


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
        QGMMainWindow::Provider prov = static_cast<QGMMainWindow::Provider>( settings.value("provider").toInt() );
        refreshing = true;
        emit authenticateUser(&user, &prov);
    }
}


string TcpServer::statusCodeAsString(httpStatusCode c)
{
	switch (c)
	{
            case c200: return "200 OK\r\n";
            case c202: return "202 Accepted\r\n";
            case c404: return "424 Unknown command\r\n";
            case c424: return "424 No mesh loaded\r\n";
            case c500: return "500 \r\n";
            case c503: return "503 Service Unavailable\r\n";
            //default: throw Exception("Bad httpStatusCode");
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
	cout << endl << "[TcpServer::reading] Reading data..." << endl;

	bool headerEnd = false;
	int maxSize = 1000000;
	int bodySize = maxSize;
	int receivedBytes = 0;

	QByteArray reqHead;
	// wait for message body
	while(!headerEnd){
		QByteArray buffer;
		buffer = this->socket->readLine(maxSize);
		reqHead += buffer;
		string mess = buffer.toStdString();

		// search for content length info
		if(mess.find("Content-Length: ")!= -1){
			bodySize = std::stoi(mess.substr(mess.find("Content-Length: ")+16,std::string::npos));
		}
		// search for end of message header
		if(reqHead.toStdString().find("\r\n\r\n")!= -1){
			headerEnd = true;
		}
	}

	cout << endl << "[TcpServer::reading] Received request: " << endl;
	qDebug() << "Header: " << reqHead;
	request->httpParser(QString(reqHead).toStdString());

	QByteArray reqBody;
	while(receivedBytes < bodySize && HTTP::method_to_string(request->getMeth()) == "POST"){
		reqBody += this->socket->readLine(maxSize);
		receivedBytes = reqBody.size();
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
	int numbOfBytes = response->at(1).toLocal8Bit().size()+1;
	//mess += "Content-Length: " + QVariant(numbOfBytes).toString() + "\r\n";
	//mess += "Content-Type: json\r\n" ;
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


bool TcpServer::authenticateUser(QString *username, QGMMainWindow::Provider *provider)
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
        case QGMMainWindow::GITHUB:
            authorizationUrlBase = "https://github.com/login/oauth/authorize";
            accessTokenUrlBase = "https://github.com/login/oauth/access_token";
            userDataUrlBase = "https://api.github.com/user";
            clientId = "f31165013adac0da36ed";
            clientSecret = "32d6f2a7939c1b40cae13c20a36d4cd32942e60d";
            scope = "user";
            break;
        case QGMMainWindow::GITLAB:
            authorizationUrlBase = "https://gitlab.com/oauth/authorize";
            accessTokenUrlBase = "https://gitlab.com/oauth/token";
            userDataUrlBase = "https://gitlab.com/api/v4";
            //userDataUrlBase = "https://gitlab.com/api/v4/users?username=" + username->toStdString();
            clientId = "0838bf6d76153b656173";
            clientSecret = "c29a631dbcec4349c38d";
            scope = "public+write";
            break;
        case QGMMainWindow::ORCID:
            authorizationUrlBase = "https://orcid.org/oauth/authorize";
            accessTokenUrlBase = "https://orcid.org/oauth/token";
            userDataUrlBase = "https://api.gitlab.com/user";
            clientId = "APP-QTDRZ74P88AWIDF8";
            clientSecret = "38c556d3-ec89-4666-aff0-30d2e44c1750";
            scope = "/authenticate";
            redirectURI = "https://localhost:8080/authorize"; // orcid requests http(s)!
            break;
        case QGMMainWindow::REDDIT:
            authorizationUrlBase = "https://www.reddit.com/api/v1/authorize";
            accessTokenUrlBase = "https://www.reddit.com/api/v1/access_token";
            userDataUrlBase = "https://api.gitlab.com/user";
            clientId = "6QVMuM64f_ApLQ";
            clientSecret = "BKHdqqbTEu1pkvbwSlx5_QIUKKwolA";
            scope = "read edit";
            break;
        case QGMMainWindow::MATTERMOST:
            authorizationUrlBase = "http://localhost:8080/api/v4/users/login";
            accessTokenUrlBase = "";
            userDataUrlBase = "";
            clientId = "";
            clientSecret = "";
            scope = "";
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
                "&scope=" + scope + "&login=" + username + "&redirect_uri=" + redirectURI + "&state=" + state + "&duration=permanent";

        qDebug() << "[TcpServer::RequestAuthentication] Via: " << authorizationUrl;

        QDesktopServices::openUrl(QUrl(authorizationUrl));

        string code;
        QObject::connect(this, &TcpServer::codeReceived, this, [=](string code){
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);

            string s;
            s += code;
            cout << "[TcpServer:authenticateUser] Code Received: " << s << endl;

            QString pars = "?clientId=" + clientId + "&clientSecret=" + clientSecret + "&code=" + QString::fromStdString(s);

            QString accessTokenUrl = accessTokenUrlBase + pars;
            qDebug() << "[TcpServer:RequestAccessToken] Via: " << accessTokenUrl;

            QNetworkRequest request(accessTokenUrlBase);

            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

            QUrlQuery params;
            params.addQueryItem("client_id", clientId);
            params.addQueryItem("client_secret", clientSecret);
            params.addQueryItem("code", QString::fromStdString(s));
            params.addQueryItem("redirect_uri", redirectURI);

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
        QNetworkRequest request(userDataUrl);
        qDebug() << "TcpServer::authorizeUser] Requesting User Data via: " << userDataUrl;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setRawHeader(QByteArray("Authorization"),  QByteArray::fromStdString("token " + token));

        QObject::connect(manager, &QNetworkAccessManager::finished, this, &TcpServer::readUserData);

        manager->get(request);
    });

    if(refreshing){
        cout << "[TcpServer:authenticateUser] Refreshing Authentication" << endl;
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

    if(str.contains("access_token=", Qt::CaseSensitive)){
        int start = str.indexOf("access_token=", 1);
        int end = str.indexOf("&", 1);

        QString token = str.mid(start+14, end-start-14);
        emit tokenReceived(token.toStdString());
    }
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


QStringList TcpServer::parseCommand(Request req){

	string command=req.getFunc();
	map<string,string> pars = req.getPars();
	string return_type = "csv";
	if( HTTP::method_to_string(req.getMeth())=="GET"){
		if(pars.find("return_type") != pars.end()){
			return_type = pars["return_type"];
		}else{
			cout << "[QGMMainWindow::parseCommand] Missing Return Type - Set to Default (csv)." << endl;
		}
	}

	string body=req.getBody();
	map<string,vector<double>> bodyPars;
	if(body.size()>0){	
		parseCSV(body,bodyPars);
	}
	
	cout << "[QGMMainWindow::parseCommand] Received command: " << command << endl;

	json data;
	QString data_str;
	httpStatusCode statusCode;

	if(command == "load"){
		QString meshName;
		if(pars.find("filename") != pars.end()){
			meshName = QString::fromStdString(pars["filename"]);
		}else{
			cout << "[QGMMainWindow::parseCommand] Missing Data Path - Set to Default ('../../testdata/cube.obj')." << endl;
			meshName = QString::fromStdString("../../testdata/cube.obj");
		}

		this->mainWin->load(meshName);

		statusCode = c200;
	}
	else if(command == "exportVertices"){

		if(mainWin->getWidget()->getMesh() != nullptr){

			QString path;
			if(pars.find("filename") != pars.end()){
				path = QString::fromStdString(pars["filename"]);
			}else{
				cout << "[QGMMainWindow::parseCommand] Missing Data Path - Set to Default ('../../testdata/cube.obj')." << endl;
				path = QString::fromStdString("../../testdata/cube.obj");
			}

			if(this->mainWin->getWidget()->getMesh()->exportVertexCoordinatesToCSV(path,false)){
				statusCode = c200;
			}else{
				cout << "[QGMMainWindow::parseCommand] Error: Could not export!" << endl;
				statusCode = c500;
			}
		} else {
			std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getVertices"){

		if(mainWin->getWidget()->getMesh() != nullptr){

			std::vector<Vertex*> rVertices;
			this->mainWin->getWidget()->getMesh()->getVertexList(&rVertices );

			vector<Vertex*>::iterator itVertex;
                        data_str += "vertices_x,vertices_y,vertices_z\n";
			for( itVertex=rVertices.begin(); itVertex!=rVertices.end(); itVertex++ ) {
                                //data.push_back((double)(*itVertex)->getIndex());
				data.push_back((*itVertex)->getX());
				data.push_back((*itVertex)->getY());
				data.push_back((*itVertex)->getZ());
                                data_str += QString::number((*itVertex)->getX()) + "," + QString::number((*itVertex)->getY()) + "," + QString::number((*itVertex)->getZ()) + "\n";
			}

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getMeshVertexNormals"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			std::vector<Vector3D> normals;
			this->mainWin->getWidget()->getMesh()->getMeshVertexNormals(&normals);

                        data_str += "normals_x,normals_y,normals_z \n";
			for (vector<Vector3D>::iterator normal = normals.begin(); normal != normals.end(); normal++){
				data.push_back(normal->getX());
				data.push_back(normal->getY());
				data.push_back(normal->getZ());
				data_str += QString::number(normal->getX()) + "," + QString::number(normal->getY()) + "," + QString::number(normal->getZ()) + "\n";
			}

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getBoundingBoxSize"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			Vector3D boxSize;
			this->mainWin->getWidget()->getMesh()->getBoundingBoxSize(boxSize);

			data.push_back(boxSize.getX());
			data.push_back(boxSize.getY());
			data.push_back(boxSize.getZ());
			data_str += QString::number(boxSize.getX()) + "," + QString::number(boxSize.getY()) + "," + QString::number(boxSize.getZ()) + "\n";			
			
			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getVerticesInBeam"){
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

                                    this->mainWin->getWidget()->getMesh()->getVerticesInBeam(vertex,vertex+normal*beamLength[i]*2,beamSize[i],&verticesInBeam);

                                    json j_entry;
                                    for (set<Vertex*>::iterator v = verticesInBeam.begin(); v != verticesInBeam.end(); v++){
                                            Vertex* vert = *v;
                                            j_entry.push_back(vert->getX());
                                            j_entry.push_back(vert->getY());
                                            j_entry.push_back(vert->getZ());
                                            data_str += QString::number(vert->getX()) + "," + QString::number(vert->getY()) + "," + QString::number(vert->getZ()) + "\n";
                                    }
                                    data.push_back(j_entry);

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
	}
	else if(command=="assignFeatVec"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			vector<double> funcVals;
			if(bodyPars.find("func_values") != bodyPars.end()){
				funcVals = bodyPars["func_values"];
			}else{
				cout << "[QGMMainWindow::parseCommand] Missing function values for feature vectors." << endl;
			}

			bool succ = this->mainWin->getWidget()->getMesh()->assignFeatureVectors(funcVals,1);  // TODO second par
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
	else if(command=="compFeatVecLen"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			this->mainWin->actionFeatLengthEuc->trigger();

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::parseCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
        else if(command=="authorize"){
                string code;
                if(pars.find("code") != pars.end()){
                        code = pars["code"];
                        //cout << "[QGMMainWindow::parseCommand] Received Code '" << code << "'" << endl;
                        emit codeReceived(code);
                }else{
                        cout << "[QGMMainWindow::parseCommand] Missing Code." << endl;
                }
                string returnedState;
                if(pars.find("state") != pars.end()){
                        returnedState = pars["state"];
                        if(returnedState != "randomString"){
                            cout << "[QGMMainWindow::parseCommand] State differs!" << endl;
                        }
                }else{
                        cout << "[QGMMainWindow::parseCommand] Missing State." << endl;
                }
                /*
                string token;
                if(pars.find("access_token") != pars.end()){
                        token = pars["code"];
                        cout << "[QGMMainWindow::parseCommand] Received Token. '" << token << "'" << endl;
                        emit tokenReceived(token);
                }else{
                        cout << "[QGMMainWindow::parseCommand] Missing Token." << endl;
                }
                */
            }
	else{
		std::cout << "[QGMMainWindow::parseCommand] Error: Unknown command!" << std::endl;
		statusCode = c404;
	}

	QStringList response;
    response << QString::fromStdString(TcpServer::statusCodeAsString(statusCode));
	if(return_type == "json"){
		response << QString::fromStdString(data.dump());
	}else{
		response << data_str;
	}

	return response;
}


void TcpServer::setMainWindow(QGMMainWindow *mainWindow)
{
	this->mainWin = mainWindow;
}


