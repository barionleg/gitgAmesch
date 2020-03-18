#include <catch.hpp>
#include <GigaMesh/icoSphereTree/IcoSphereTree.h>
#include <random>

TEST_CASE("icosphereTree construction","[icosphere]")
{
	IcoSphereTree tree(3);

	SECTION("adding vertices close to an initial point should yield the same id")
	{
		const double t = (1.0 + sqrt(5.0)) / 2.0;
		Vector3D vec(-1.0, t, 0.0);

		auto index = tree.getNearestVertexIndexAt(vec);

		REQUIRE(index == 0);

		Vector3D vec1(-1.1, t      , 0.0);
		Vector3D vec2(-1.0, t + 0.1, 0.0);
		Vector3D vec3(-1.0, t      , 0.1);
		Vector3D vec4(-1.1, t + 0.1, 0.1);

		index = tree.getNearestVertexIndexAt(vec1);
		CHECK(index == 0);

		index = tree.getNearestVertexIndexAt(vec2);
		CHECK(index == 0);

		index = tree.getNearestVertexIndexAt(vec3);
		CHECK(index == 0);

		index = tree.getNearestVertexIndexAt(vec4);
		CHECK(index == 0);
	}
}

size_t getMinDistanceVertex(const Vector3D& vec, const std::vector<Vector3D>& treeVertices)
{
	auto minDistance = std::numeric_limits<double>::max();
	size_t minIndex = 0;

	for(size_t i = 0; i<treeVertices.size(); ++i)
	{
		const auto len = (vec - treeVertices[i]).getLength3Squared();
		if(len < minDistance)
		{
			minDistance = len;
			minIndex = i;
		}
	}

	return minIndex;
}

std::vector<Vector3D> generateTreeVertices(const IcoSphereTree& tree)
{
	const auto vertexData = tree.getVertices();

	std::vector<Vector3D> retVec;
	retVec.reserve(vertexData.size() / 3);

	for(size_t i = 0; i<vertexData.size(); i+=3)
	{
		retVec.push_back(Vector3D(static_cast<double>(vertexData[i    ]),
		                          static_cast<double>(vertexData[i + 1]),
		                          static_cast<double>(vertexData[i + 2])
		                     ));
	}

	return retVec;
}

TEST_CASE("icosphereTree stress test checking", "[.icosphere_stress]")
{
	IcoSphereTree tree(5);

	auto treeVertices = generateTreeVertices(tree);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.0,1.0);

	for(int i = 0; i<10000; ++i)
	{
		Vector3D vec(dis(gen), dis(gen), dis(gen));

		vec = normalize3(vec);

		auto treeIndex = tree.getNearestVertexIndexAt(vec);
		auto testIndex = getMinDistanceVertex(vec,treeVertices);

		REQUIRE(treeIndex == testIndex);
	}
}
