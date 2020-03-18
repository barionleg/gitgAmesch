#include <catch.hpp>
#include <GigaMesh/icoSphereTree/IcoSphereTree.h>

TEST_CASE("icosphereTree construction")
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
