#include "triangulation.h"

std::vector<size_t> GigaMesh::Util::triangulateNgon(const std::vector<Vector3D>& vertices)
{
	if(vertices.empty())
	{
		return {};
	}
	if(vertices.size() == 1)
	{
		return {0};
	}
	if(vertices.size() == 2)
	{
		return {0,1};
	}
	if(vertices.size() == 3)
	{
		return {0,1,2};
	}


	std::vector<size_t> indices;
	indices.reserve((vertices.size() - 2) * 3); //reserve enough space for all triangle indices

	const Vector3D ngonNormal = vertices[0] % vertices[1];
	const Vector3D targetNormal(0.0,0.0,-1.0);



	return indices;
}
