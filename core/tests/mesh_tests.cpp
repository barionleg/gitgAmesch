#include <catch.hpp>
#include <GigaMesh/mesh/mesh.h>

TEST_CASE("Testing basic mesh operations", "[mesh]")
{
	SECTION("Creating a new mesh with valid input")
	{
		bool success = false;
		Mesh testMesh("testdata/twotriangles.obj", success);
		REQUIRE(success == true);
	}

	SECTION("Creating a new mesh with invalid input")
	{
		bool success = false;
		Mesh testMesh("notavalidMeshname", success);
		REQUIRE(success == false);
	}
}
