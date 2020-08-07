#include "tcpServer.h"
#include <typeinfo>


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
    QObject::connect(server, SIGNAL(newConnection()),
	    this, SLOT(newConnection()));

    QObject::connect(this->socket, SIGNAL(connected()),
	    this, SLOT(connected()));

    if(!server->listen(QHostAddress::Any, 8080))
    {
	qDebug() << "[TcpServer::TcpServer] Server could not start";
    }
    else
    {
	qDebug() << "[TcpServer::TcpServer] Server started.";
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


void TcpServer::connected()//QTcpSocket *socket)
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

        if(command=="load"){
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
        else if(command=="exportVertices"){

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

			/*
			for(map<string,vector<double>>::iterator it = bodyPars.begin();it != bodyPars.end();it++)	{
				cout << it->first << " ";
				data_str += QString::fromStdString(it->first);
				vector<double> values = it->second;
				for(size_t i=0; i<values.size();i++){
					cout << double(values[i]) << " " ;
					data_str += QString::fromStdString(",") + QString::number(values[i]);
				}
				cout << endl;
				data_str += QString::fromStdString("\n");
			}
			*/
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
        else if(command=="nonMaxSupp"){
                if(mainWin->getWidget()->getMesh() != nullptr){

                        bool parSucc = true;
                        double nmsDistances;
                        if(pars.find("nms_distance") != pars.end()){
                                nmsDistances = std::stod(pars["nms_distance"]);
                        }else{
                            cout << "[QGMMainWindow::parseCommand] Missing NMS distance." << endl;
                            parSucc = false;
                        }

                        if(parSucc){

                            if(this->mainWin->getWidget()->getMesh()->funcExpSuppNonMax(nmsDistances)){
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
        else if(command=="watershed"){
                if(mainWin->getWidget()->getMesh() != nullptr){

                        bool parSucc = true;
                        double delInput;
                        if(pars.find("deletable_input") != pars.end()){
                                delInput = std::stod(pars["deletable_input"]);
                        }else{
                            cout << "[QGMMainWindow::parseCommand] Missing deletable input." << endl;
                            parSucc = false;
                        }

                        if(parSucc){

                            if(this->mainWin->getWidget()->getMesh()->funcExpComputeWatershed(delInput)){
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
        else if(command=="clustering"){
                if(mainWin->getWidget()->getMesh() != nullptr){

                        bool parSucc = true;
                        int numbInter;
                        if(pars.find("number_iterations") != pars.end()){
                                numbInter = std::stoi(pars["number_iterations"]);
                        }else{
                            cout << "[QGMMainWindow::parseCommand] Missing number of iterations." << endl;
                            parSucc = false;
                        }

                        if(parSucc){

                            if(this->mainWin->getWidget()->getMesh()->funcExpComputeClustering(numbInter)){
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
        else if(command=="ransac"){
                if(mainWin->getWidget()->getMesh() != nullptr){

                        bool parSucc = true;

                        int numbInter;
                        if(pars.find("number_iterations") != pars.end()){
                                numbInter = std::stoi(pars["number_iterations"]);
                        }else{
                            cout << "[QGMMainWindow::parseCommand] Missing number of iterations - set to default (100)." << endl;
                            numbInter = 100;
                            parSucc = false;
                        }

                        string filename = this->mainWin->getWidget()->getMesh()->getFullName().string();
                        if(filename.find(".obj")!=-1) {filename = filename.substr(0,filename.find(".obj"));}
                        if(filename.find(".ply")!=-1) {filename = filename.substr(0,filename.find(".ply"));}
                        filename += ".wedges"; //.csv";

                        if(parSucc){
                            vector<vector<double>> meshIntrinsicExtractedTetraeders;
                            if(this->mainWin->getWidget()->getMesh()->funcExpComputeRANSAC(numbInter,
                                                                                           filename,
                                                                                           meshIntrinsicExtractedTetraeders,
                                                                                           false,
                                                                                           0.0,
                                                                                           false,
                                                                                           false,
                                                                                           true,
                                                                                           true)){
                                for (int i = 0; i < meshIntrinsicExtractedTetraeders.size(); i++) {
                                        for (int j = 0; j < meshIntrinsicExtractedTetraeders[i].size(); j++){
                                            data_str += QString::number(meshIntrinsicExtractedTetraeders[i][j] );
                                            if(j != meshIntrinsicExtractedTetraeders[i].size()-1){
                                                data_str += ",";
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
        }
        else if(command=="featureElementsByIndex"){
                if(mainWin->getWidget()->getMesh() != nullptr){

                        bool parSucc = true;
                        int numbElement;
                        if(pars.find("element_nr") != pars.end()){
                                numbElement = std::stoi(pars["element_nr"]);
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


