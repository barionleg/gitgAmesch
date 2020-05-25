#include "tcpServer.h"
//#include <string>
#include <QString>

MyTcpServer::MyTcpServer(QObject *parent) :
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
	qDebug() << "[myTcpServer::myTcpServer] Server could not start";
    }
    else
    {
	qDebug() << "[myTcpServer::myTcpServer] Server started.";
    }
}


string MyTcpServer::statusCodeAsString(httpStatusCode c)
{
	switch (c)
	{
	case c200: return "\n200 OK\r\n";
	case c202: return "\n202 Accepted\r\n";
	case c404: return "\n424 Unknown command\r\n";
	case c424: return "\n424 No mesh loaded\r\n";
	case c500: return "\n500 \r\n";
	case c503: return "\n503 Service Unavailable\r\n";
	//default: throw Exception("Bad httpStatusCode");
	}
}


void MyTcpServer::connected()//QTcpSocket *socket)
{
	httpStatusCode statusCode;
	if(this->socket != nullptr && this->socket->isOpen()){
		statusCode = c202;//"\n202 Accepted\r\n";
	}
	else{
		statusCode = c503; //"\n503 Service Unavailable\r\n";
	}

	QString verConnected = QString::fromStdString(MyTcpServer::statusCodeAsString(statusCode));
	this->socket->write(verConnected.toLocal8Bit());
	this->socket->flush();

	this->socket->waitForBytesWritten(verConnected.toUtf8().size());
	this->socket->waitForReadyRead();
}


void MyTcpServer::reading(HTTP::Request *request)
{
	cout << endl << "[myTcpServer::reading] Reading data..." << endl;

	bool headerEnd = false;

	int maxSize = 1000000;
	int bodySize = maxSize;
	int receivedBytes = 0;

	QByteArray reqHead;
	// wait for message body
	while(!headerEnd){ //while (socket->canReadLine()){//
		QByteArray buffer;
		buffer = this->socket->readLine(maxSize);
		reqHead += buffer;
		string mess = buffer.toStdString();

		// search for content length info
		if(mess.find("Content-Length: ")!= -1){
			//cout << mess.substr(mess.find("Content-Length: ")+16,std::string::npos) << endl;
			bodySize = std::stoi(mess.substr(mess.find("Content-Length: ")+16,std::string::npos));
		}
		// search for end of message header
		if(reqHead.toStdString().find("\r\n\r\n")!= -1){
			headerEnd = true;
		}
	}

	cout << endl << "[myTcpServer::reading] Received request: " << endl;

	qDebug() << "Header: " << reqHead;

	QByteArray reqBody;
	// read message body
	while(receivedBytes < bodySize ){
		reqBody += this->socket->readLine(maxSize);
		receivedBytes = reqBody.size();
	}

	qDebug() << "Body: " <<reqBody << "\n";

	request->httpParser(QString(reqHead).toStdString());

	if(HTTP::method_to_string(request->getMeth()) == "POST" && reqBody.size() != 0){
		request->setPars(reqBody.toStdString());
	}
	/*
	if(r.getMeth() == 'GET' && messBody.size() == 0){
		cout <<
	}*/

	request->info();
}


void MyTcpServer::sending(QStringList *response)
{
	cout << "[myTcpServer::sending] Sending data..." << endl;

	QByteArray mess;
	mess += response->at(0).toLocal8Bit(); //responseBody.toUtf8().size()
	int numbOfBytes = response->at(1).toLocal8Bit().size();
	mess += "Content-Length: " + QVariant(numbOfBytes).toString() + "\r\n";
	//mess += "Content-Type: json\r\n" ;
	mess += "\r\n\r\n" ;
	mess += response->at(1).toLocal8Bit();

	socket->write(mess);
	socket->flush();

	//this->socket->waitForBytesWritten(response->at(0).toUtf8().size());
	//this->socket->waitForReadyRead();

	cout << "[myTcpServer::sending] Finished sending." << endl;
}


void MyTcpServer::newConnection()
{
	this->socket = server->nextPendingConnection();

	emit connected();

	HTTP::Request r;
	emit reading(&r);

	QStringList requestData = parseCommand(r.getFunc(), r.getPars());

	emit sending(&requestData);

	this->socket->close();
}


QStringList MyTcpServer::parseCommand(string command,json parameters){//std::vector<std::string> parameters); {

	cout << endl << "[QGMMainWindow::receiveCommand] Received command: " << command << endl;

	QString p1,p2,p3,p4,p5;
	json j1, j2, j3,j4,j5;
	if(parameters.size()>=1){
		p1 = QString::fromStdString(parameters[0].dump());
		j1 = parameters[0];
	}
	if(parameters.size()>=2){
		p2 = QString::fromStdString(parameters[1].dump());
		j2 = parameters[1];
	}
	if(parameters.size()>=3){
		p3 = QString::fromStdString(parameters[2].dump());
		j3 = parameters[2];
	}
	if(parameters.size()>=4){
		p4 = QString::fromStdString(parameters[3].dump());
		j4 = parameters[3];
	}

	json data;
	//QString data;
	httpStatusCode statusCode;

	if(command == "load"){
		QString meshName = QString::fromStdString(p1.toStdString().substr(1,p1.size()-2));

		this->mainWin->load(meshName);

		statusCode = c200;
	}
	else if(command == "exportVertices"){

		if(mainWin->getWidget()->getMesh() != nullptr){
			if(this->mainWin->getWidget()->getMesh()->exportVertexCoordinatesToCSV(p1,false)){
				statusCode = c200;
			}else{
				cout << "[QGMMainWindow::receiveCommand] Error: Could not export!" << endl;
				statusCode = c500;
			}
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getVertices"){

		if(mainWin->getWidget()->getMesh() != nullptr){

			std::vector<Vertex*> rVertices;
			this->mainWin->getWidget()->getMesh()->getVertexList(&rVertices );

			vector<Vertex*>::iterator itVertex;
			for( itVertex=rVertices.begin(); itVertex!=rVertices.end(); itVertex++ ) {
				data.push_back((double)(*itVertex)->getIndex());
				data.push_back((*itVertex)->getX());
				data.push_back((*itVertex)->getY());
				data.push_back((*itVertex)->getZ());
			}

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getMeshVertexNormals"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			std::vector<Vector3D> normals;
			this->mainWin->getWidget()->getMesh()->getMeshVertexNormals(&normals);

			for (vector<Vector3D>::iterator normal = normals.begin(); normal != normals.end(); normal++){
				data.push_back(normal->getX());
				data.push_back(normal->getY());
				data.push_back(normal->getZ());
			}

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
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
			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="getVerticesInBeam"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			if(j1.size()/4+j2.size()==j1.size()){
				cout << "Number of vertices and normals unequal." << endl;
			}

			int i = 1,j = 0;
			while(i+3<=j1.size() && j+2<=j2.size()){ //j1.size();i+=4){
				set<Vertex*> verticesInBeam;
				Vector3D vertex;
				Vector3D normal;
				if(j1.size()>2){
					vertex = Vector3D(double(j1[i]),double(j1[i+1]),double(j1[i+2]));
				}
				if(j2.size()>2){
					normal = Vector3D(double(j2[j]),double(j2[j+1]),double(j2[j+2]));
				}

				this->mainWin->getWidget()->getMesh()->getVerticesInBeam(vertex,vertex+normal*j3[int(j/3)]*2,j4[int(j/3)],&verticesInBeam);

				json j_entry;
				for (set<Vertex*>::iterator v = verticesInBeam.begin(); v != verticesInBeam.end(); v++){
					Vertex* vert = *v;
					j_entry.push_back(vert->getX());
					j_entry.push_back(vert->getY());
					j_entry.push_back(vert->getZ());
				}
				data.push_back(j_entry);

				i+=4;
				j+=3;
			}

			statusCode = c200;
		} else {
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else if(command=="assignFeatVec"){
		if(mainWin->getWidget()->getMesh() != nullptr){

			vector<double> funcVals;
			for (int i = 0; i < j1.size();i++){
				funcVals.push_back(double(j1[i]));
			}

			bool succ = this->mainWin->getWidget()->getMesh()->assignFeatureVectors(funcVals,j2);//(const vector<double>&   rFeatureVecs = j1, const uint64_t&rMaxFeatVecLen = 1);
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
			std::cout << "[QGMMainWindow::receiveCommand] Error: No mesh loaded!" << std::endl;
			statusCode = c424;
		}
	}
	else{
		std::cout << "[QGMMainWindow::receiveCommand] Error: Unknown command!" << std::endl;
		statusCode = c404;
	}

	QStringList response;
	response << QString::fromStdString(MyTcpServer::statusCodeAsString(statusCode)) << QString::fromStdString(data.dump());

	return response;
}


void MyTcpServer::setMainWindow(QGMMainWindow *mainWindow)
{
	this->mainWin = mainWindow;
}


