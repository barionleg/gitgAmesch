#ifndef SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_SURFACE_MSII_H
#define SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_SURFACE_MSII_H

#include <cstddef>

namespace spherical_intersection {
class Graph;
namespace algorithm {
//! @brief Computes the arc length of the intersection of the mesh's surface and
//! the sphere that define the intersection that is represented by the given
//! graph relative to the sphere's radius squared.
//! @param graph reference to the graph representing the intersection.
//! @return The arc length.
double get_sphere_surface_length(const Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif