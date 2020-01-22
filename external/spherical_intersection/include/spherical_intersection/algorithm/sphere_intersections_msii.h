#ifndef SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_INTERSECTION_MSII_H
#define SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_INTERSECTION_MSII_H

#include <cstddef>
#include <vector>

namespace spherical_intersection {
class Graph;
namespace algorithm {
//! @brief Accesses the calculated intersections of the mesh's surface and the sphere
//! which are contained in the given precomputed graph.
//! @param graph reference to the graph representing the intersection.
//! @return matrix with vertex number, intersection number and intersection coordinates x/y/z.
std::vector<double> get_sphere_intersections(const Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif
