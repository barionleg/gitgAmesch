#include "triangulation.h"

#ifdef LIBPSALM
#include<libpsalm/libpsalm.h>
#endif

#include <numeric>

std::vector<size_t> triangulateQuad(const std::vector<Vector3D>& vertices)
{
	if( (vertices[0] - vertices[2]).getLength3Squared() < (vertices[1] - vertices[3]).getLength3Squared() )
	{
		return {0,1,2,0,2,3};
	}

	return {0,1,3,1,2,3};
}
//! triangulates a ring of vertices
//! @param vertices input vertices (non closed polyline)
//! @return triangulated indices of the vertices
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

	if(vertices.size() == 4)
	{
		return triangulateQuad(vertices);
	}

#ifdef LIBPSALM
	std::vector<double> coordinates;
	coordinates.reserve(vertices.size() * 3);

	for(const auto& vertex : vertices)
	{
		coordinates.push_back(vertex.getX());
		coordinates.push_back(vertex.getY());
		coordinates.push_back(vertex.getZ());
	}

	int numNewVertices = 0;
	int numNewFaces = 0;
	double* newCoordinates = nullptr;
	long* newCoordinatesIndices = nullptr;
	fill_hole(vertices.size(), nullptr, coordinates.data(), nullptr, nullptr,&numNewVertices, &newCoordinates,&numNewFaces,&newCoordinatesIndices, true);

	std::vector<size_t> retIndices(numNewFaces * 3);

	for(size_t i = 0; i< numNewFaces * 3; ++i)
	{
		retIndices[i] = static_cast<size_t>(std::abs(newCoordinatesIndices[i]));
	}

	return retIndices;
#else
	return std::vector<size_t>();
#endif
}
