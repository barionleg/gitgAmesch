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

		signals:
			void handleRequest();

		public:
			Request();
			Request(Method method, std::string ver, std::string function, std::map<std::string, std::string> parameters, std::string body);
			Request(Request &request);
			void httpParser(std::string request);
			Method getMethod();	
			std::string getVersion();		
			std::string getFunction();		
			std::map<std::string, std::string> getParameters();
			std::string getBody();			
			void setParameters(std::string& parameters, std::map<std::string, std::string>& pars_map);		
			void setParameters(std::map<std::string, std::string> parameters);
			void setBody(std::string body);
			void info();

		private:
			std::string version;
			Method method;
			std::string function;
			std::map<std::string, std::string> parameters;
			std::string body;
	};

}


#endif // HTTP_PARSER_H
