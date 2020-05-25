
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

		signals:
			void handleRequest();

		public:
			// Default-Constructor
			Request();

			// Non-Default-Constructor
			Request(Method meth, string ver,string func,json pars);

			// Copy-Constructor
			Request(Request &r);

			void httpParser(string request);

			Method getMeth();
			
			string getVer();
			
			string getFunc();
			
			json getPars();
			
			void setPars(string parameters);
			
			void setPars(json parameters);
			
			void info();
	};

}




