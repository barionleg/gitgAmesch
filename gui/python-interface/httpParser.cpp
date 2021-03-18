#include <httpParser.h>


using namespace std;
using namespace HTTP;
using json = nlohmann::json;


template <class Container>
void splitStr(const string& str, Container& cont, char delim)
{
	std::stringstream ss(str);
	std::string token;
	while(std::getline(ss, token, delim)) {
		cont.push_back(token);
	}
}


Request::Request(){}


Request::Request(Method meth, string ver,string func,map<string, string> pars,string body)
{
	this->meth = meth;
	this->ver = ver;
	this->func = func;
	this->pars = pars;
	this->body = body;
}


Request::Request(Request &r)
{
	this->meth = r.meth;
	this->ver = r.ver;
	this->func = r.func;
	this->pars = r.pars;
	this->body = r.body;
}

void Request::httpParser(string request)
{
        cout << "[HTTP::" << __FUNCTION__ << "] Start parsing..." << endl;
    
    size_t found1= request.find(" ");
    size_t found2= request.find(" ",found1+1);

	string method = request.substr(0,found1);
	HTTP::Method m = HTTP::method_from_string(method);

    string f,v,p;
    json j;
    v = request.substr(found2+1,request.find("\r\n")-found2-1);
    if(request.find("?")!=-1){
        f = request.substr(request.find("/")+1,request.find("?")-request.find("/")-1);
        p=request.substr(request.find("?")+1,found2-request.find("?")-1);
    }else{
        f = request.substr(request.find("/")+1,found2-request.find("/")-1);
        p="";
    }

	this->meth = m;
	this->ver = v;
	this->func = f;
	if(p.size()>0){
		map<string,string> pars_map;
		setPars(p,pars_map);
		this->pars=pars_map;
	}

        cout << endl << "[HTTP::" << __FUNCTION__ << "] Parsing completed." << endl << endl;
}

Method Request::getMeth() {
    return meth;
}

string Request::getVer() {
    return ver;
}

string Request::getFunc() {
    return func;
}

string Request::getBody() {
    return body;
}

map<string, string> Request::getPars() {
    return pars;
}

void Request::setPars(string& parameters, map<string,string>& pars_map) {
	vector<string> pars_vec;
	char splitBy = '&';
	if(parameters.find("&")!=-1){
		splitStr(parameters,pars_vec,splitBy);
	}else{
		pars_vec.push_back(parameters);
	}
	for (auto i = pars_vec.begin(); i != pars_vec.end(); ++i){
		vector<string> key_val;
		splitBy='=';
		splitStr(*i,key_val,splitBy);
		pars_map[key_val[0]]=key_val[1];
	}
}

void Request::setPars(map<string, string> parameters) {
    this->pars = parameters;
}

void Request::setBody(string b) {
    this->body = b;
}

void Request::info(){
        cout << "[HTTP::" << __FUNCTION__ << "] Info about request: " << endl;
	cout << "Version: " << this->ver << endl;
	cout << "Method: " << HTTP::method_to_string(this->meth) << endl;
	cout << "Function: " << this->func << endl;
	cout << "Parameters: " << this->pars.size() << endl;
}





