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

#include <httpParser.h>


using namespace std;
using namespace HTTP;


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


Request::Request(Method method, string version, string function, map<string, string> parameters, string body)
{
        this->method = method;
        this->version = version;
        this->function = function;
        this->parameters = parameters;
	this->body = body;
}


Request::Request(Request &request)
{
        this->method = request.method;
        this->version = request.version;
        this->function = request.function;
        this->parameters = request.parameters;
        this->body = request.body;
}

void Request::httpParser(string request)
{
    cout << "[HTTP::" << __FUNCTION__ << "] Start parsing..." << endl;
    
    size_t found1 = request.find(" ");
    size_t found2 = request.find(" ",found1+1);

    string method = request.substr(0,found1);
    HTTP::Method m = HTTP::method_from_string(method);

    string f,v,p;
    v = request.substr(found2+1, request.find("\r\n")-found2-1);
    if(request.find("?") != -1){
        f = request.substr(request.find("/")+1, request.find("?")-request.find("/")-1);
        p = request.substr(request.find("?")+1, found2-request.find("?")-1);
    }else{
        f = request.substr(request.find("/")+1, found2-request.find("/")-1);
        p = "";
    }

        this->method = m;
        this->version = v;
        this->function = f;
	if(p.size()>0){
            map<string,string> pars_map;
            setParameters(p, pars_map);
            this->parameters = pars_map;
	}

        cout << endl << "[HTTP::" << __FUNCTION__ << "] Parsing completed." << endl << endl;
}

Method Request::getMethod() {
    return method;
}

string Request::getVersion() {
    return version;
}

string Request::getFunction() {
    return function;
}

string Request::getBody() {
    return body;
}

map<string, string> Request::getParameters() {
    return parameters;
}

void Request::setParameters(string& parameters, map<string,string>& pars_map) {
    vector<string> pars_vec;
    char splitBy = '&';
    if(parameters.find("&") != -1){
        splitStr(parameters, pars_vec,splitBy);
    }else{
        pars_vec.push_back(parameters);
    }
    for (auto i = pars_vec.begin(); i != pars_vec.end(); ++i){
        vector<string> key_val;
        splitBy = '=';
        splitStr(*i, key_val, splitBy);
        pars_map[key_val[0]] = key_val[1];
    }
}

void Request::setParameters(map<string, string> parameters) {
    this->parameters = parameters;
}

void Request::setBody(string body) {
    this->body = body;
}

void Request::info(){
        cout << "[HTTP::" << __FUNCTION__ << "] Info about request: " << endl;
        cout << "Version: " << this->version << endl;
        cout << "Method: " << HTTP::method_to_string(this->method) << endl;
        cout << "Function: " << this->function << endl;
        cout << "Parameters: " << this->parameters.size() << endl;
}





