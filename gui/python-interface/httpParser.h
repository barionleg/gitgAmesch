#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "httpMethod.cpp"
#include "httpVersion.cpp"
#include <QObject>
#include <map>
#include <json.hpp>


template <class Container>
void splitStr(const std::string& str, Container& cont, char delim = ' ');


namespace HTTP
{

	class Request : public QObject
	{
		Q_OBJECT

		private:
			std::string ver;
			Method meth;
			std::string func;
			std::map<std::string, std::string> pars;
			std::string body;

		signals:
			void handleRequest();

		public:
			Request();

			Request(Method meth, std::string ver, std::string func, std::map<std::string, std::string> pars, std::string body);

			Request(Request &r);

			void httpParser(std::string request);

			Method getMeth();
			
			std::string getVer();
			
			std::string getFunc();
			
			std::map<std::string, std::string> getPars();

			std::string getBody();
			
			void setPars(std::string& parameters, std::map<std::string, std::string>& pars_map);

			void setBody(std::string b);
			
			void setPars(std::map<std::string, std::string> parameters);
			
			void info();
	};

}


#endif // HTTP_PARSER_H
