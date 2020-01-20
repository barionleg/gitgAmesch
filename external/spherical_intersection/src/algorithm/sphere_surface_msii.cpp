#include "algorithm/sphere_surface_msii.h"
#include "graph.h"
#include "graph_geometry.h"

using namespace spherical_intersection;

double algorithm::get_sphere_surface_length(const Graph &graph) {
	double total_length = 0;
	const auto &arcs = graph.get_arcs();

	for (const auto &arc : arcs) {
		total_length +=
		    Arc_Curve(arc).get_length() / arc.get_sphere().get_radius();
	}

	return total_length;
}
