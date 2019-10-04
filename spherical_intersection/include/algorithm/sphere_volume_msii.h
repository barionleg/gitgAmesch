#ifndef SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_VOLUME_MSII_H
#define SPHERICAL_INTERSECTION_ALGORITHM_SPHERE_VOLUME_MSII_H

#include <cstddef>

namespace spherical_intersection {
namespace math3d {
class Sphere;
}

class Graph;

namespace algorithm {
//! @brief Computes the area of the
//! intersection of the mesh's volume and the sphere that define the
//! intersection that is represented by the given graph relative to the sphere's
//! radius squared if the graph contains at least one cycle. Deletes the graph's
//! arcs in any case.
//! @param graph reference to the Graph representing the intersection.
//! @return The area if the graph contains at least one cycle.
//! std::numeric_limits<double>::is_NaN() otherwise.
double get_sphere_volume_area(Graph &graph);
} // namespace algorithm
} // namespace spherical_intersection

#endif