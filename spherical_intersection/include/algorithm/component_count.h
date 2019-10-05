#ifndef SPHERICAL_INTERSECTION_ALGORITHM_COMPONENT_COUNT_H
#define SPHERICAL_INTERSECTION_ALGORITHM_COMPONENT_COUNT_H

#include <cstddef>

namespace spherical_intersection {
class Graph;
namespace algorithm {
//! @brief Empties the given graph and counts the connected components of its
//! underlying undirected graph.
//! @param graph reference to the Graph whose components will be counted.
//! @return The number of components.
std::size_t get_component_count(Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif