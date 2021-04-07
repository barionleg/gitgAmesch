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

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <cstdint>
#include <cstring>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


namespace HTTP
{
    enum class Method
	{
            GET,
            HEAD,
            POST,
            PUT,
            DELETE,
            TRACE,
            OPTIONS,
            CONNECT,
            PATCH
	};

	inline std::string method_to_string(Method method)
	{
            switch(method)
            {
                case Method::GET:
                    return "GET";
                case Method::HEAD:
                    return "HEAD";
                case Method::POST:
                    return "POST";
                case Method::PUT:
                    return "PUT";
                case Method::DELETE:
                    return "DELETE";
                case Method::TRACE:
                    return "TRACE";
                case Method::OPTIONS:
                    return "OPTIONS";
                case Method::CONNECT:
                    return "CONNECT";
                case Method::PATCH:
                    return "PATCH";
                default:
                    return "GET";
            }
	}

	inline Method method_from_string (const std::string& method) noexcept
	{
            if (method == method_to_string(Method::GET))
            {
                return Method::GET;
            }
            else if (method == method_to_string(Method::HEAD))
            {
                return Method::HEAD;
            }
            else if (method == method_to_string(Method::POST))
            {
                return Method::POST;
            }
            else if (method == method_to_string(Method::PUT))
            {
                return Method::PUT;
            }
            else if (method == method_to_string(Method::DELETE))
            {
                return Method::DELETE;
            }
            else if (method == method_to_string(Method::TRACE))
            {
                return Method::TRACE;
            }
            else if (method == method_to_string(Method::OPTIONS))
            {
                return Method::OPTIONS;
            }
            else if (method == method_to_string(Method::CONNECT))
            {
                return Method::CONNECT;
            }
            else if (method == method_to_string(Method::PATCH))
            {
                return Method::PATCH;
            }
            else
            {
                return Method::GET;
            }
	}

}
