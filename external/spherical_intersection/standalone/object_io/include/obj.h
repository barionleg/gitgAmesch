#include <string>

#include "object_io.h"

namespace object_io {
namespace obj {
Object_Information load_obj(const std::string &path);

void save_obj(const std::string &path,
	      const Object_Information &object_information);
} // namespace ply
} // namespace object_io