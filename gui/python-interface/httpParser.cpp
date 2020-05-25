
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "httpMethod.cpp"
#include "httpVersion.cpp"
#include <QObject>
#include <json/json.hpp>

// for convenience
using json = nlohmann::json;

using namespace std;

// example:
// curl localhost:8080/load?filename=blub.ply
// "GET /load?filename=blub.ply HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: curl/7.67.0\r\nAccept: */*\r\n\r\n"


template <class Container>
void mySplit(const std::string& str, Container& cont, char delim = ' ')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
	cont.push_back(token);
    }
}

namespace HTTP
{
	class Request : public QObject
	{
		Q_OBJECT

		private:
			string ver;
			Method meth;
			string func;
			//std::vector<std::string> pars;
			json pars;

		public:
			// Default-Constructor
			Request(){}

			// Non-Default-Constructor
			Request(Method meth, string ver,string func,json pars)//std::vector<std::string> pars)
			{
				this->meth = meth;
				this->ver = ver;
				this->func = func;
				this->pars = pars;
			}

			// Copy-Constructor
			Request(Request &r)
			{
				meth = r.meth;
				ver = r.ver;
				func = r.func;
				pars = r.pars;
			}

			void httpParser(string request)
			{
				cout << "[HTTP::httpParser] Start parsing..." << endl;
				string method = request.substr(0,request.find(" "));

				HTTP::Method m = HTTP::method_from_string(method);

				string f = request.substr(request.find("/")+1,request.find("?")-request.find("/")-1);

				string v,p;
				json j;
				if(request.rfind("]")!= -1){
					v = request.substr(request.rfind("]")+2,request.find("\r\n")-request.rfind("]")-2);

					p = request.substr(request.find("["),request.rfind("]")-request.find("[")+1);

				}
				else{
					v = request.substr(request.find("?")+2,request.find("\r\n")-request.find("?")-2);

					p = "[]";
				}

				j = json::parse(p);

				this->meth = m;
				this->ver = v;
				this->func = f;
				this->pars = j;//p;

				cout << "HTTP::httpParser] Parsing completed." << endl << endl;
			}

			Method getMeth() const {return meth;}

			string getVer() const {return ver;}

			string getFunc() const {return func;}

			//std::vector<std::string> getPars() const {return pars;}
			json getPars() const {return pars;}

			void setPars(string parameters) {json j = json::parse(parameters); this->pars = j;}

			void setPars(json parameters) {this->pars = parameters;}

			void info(){
				cout << "[HTTP::httpParser] Info about request: " << endl;
				cout << "Version: " << this->ver << endl;
				cout << "Method: " << HTTP::method_to_string(this->meth) << endl;
				cout << "Function: " << this->func << endl;
				cout << "Parameters: " << this->pars.size() << endl;
				//cout << this->pars.dump() << endl;

				//for(std::size_t i=0; i<this->pars.size(); ++i)
				//	cout << this->pars[i] << ", ";
				//cout << endl << endl;
			}
	};

}




