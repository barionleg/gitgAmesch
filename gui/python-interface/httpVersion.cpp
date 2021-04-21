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
    enum class Version
	{
		HTTP_1_0,
		HTTP_1_1,
		HTTP_2_0
	};

	inline std::string version_to_string(Version version)
	{
		switch(version)
		{
		    case Version::HTTP_1_0:
			return "HTTP/1.0";

		    case Version::HTTP_1_1:
			return "HTTP/1.1";

		    case Version::HTTP_2_0:
			return "HTTP/2.0";
		}
	}

	inline Version version_from_string (const std::string& version) noexcept
	{
		if (version == version_to_string(Version::HTTP_1_0))
		{
		    return Version::HTTP_1_0;
		}
		else if (version == version_to_string(Version::HTTP_1_1))
		{
		    return Version::HTTP_1_1;
		}
		else if (version == version_to_string(Version::HTTP_2_0))
		{
		    return Version::HTTP_2_0;
		}
	}

}
