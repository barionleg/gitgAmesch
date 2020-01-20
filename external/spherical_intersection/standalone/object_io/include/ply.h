#include <string>
#include <vector>

#include "object_io.h"

namespace object_io {
namespace ply {
enum Format { ascii, binary_little_endian, binary_big_endian };

Object_Information load_ply(const std::string &path);

void save_ply(const std::string &path,
	      const Object_Information &object_information,
	      const std::vector<double> &qualities, const Format format);
} // namespace ply
} // namespace object_io