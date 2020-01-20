#ifndef OBJECT_IO__H
#define OBJECT_IO__H

#include <array>
#include <vector>

namespace object_io {
using Vertex_Location = std::array<double, 3>;
using Triangle_Indices = std::array<std::size_t, 3>;
using Object_Information =
    std::pair<std::vector<Vertex_Location>, std::vector<Triangle_Indices>>;
} // namespace object_io

#endif