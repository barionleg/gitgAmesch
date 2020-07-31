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
