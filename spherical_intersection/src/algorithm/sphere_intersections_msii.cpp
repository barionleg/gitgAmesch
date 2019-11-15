#include "algorithm/sphere_intersections_msii.h"
#include "graph.h"
#include "graph_geometry.h"

using namespace spherical_intersection;

std::vector<double> algorithm::get_sphere_intersections(const Graph &graph) {
	const auto &graphs_nodes = graph.get_nodes();
	// int numbOfIntersections = 0;
	std::vector<double> intersections;

	for (const auto &node : graphs_nodes) {
		auto pos = get_location(node);
		intersections.push_back(pos.get(0));
		intersections.push_back(pos.get(1));
		intersections.push_back(pos.get(2));
	}

	return intersections;
}
