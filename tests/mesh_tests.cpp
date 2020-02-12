#include <catch.hpp>
#include <GigaMesh/mesh/mesh.h>

//catch TEST_CASE / SECTION format
TEST_CASE("Testing basic mesh with invalid file", "[mesh]")
{
	SECTION("Creating a new mesh with invalid input")
	{
		bool success = false;
		Mesh testMesh("notavalidMeshname", success);
		REQUIRE(success == false);
	}
}

//catch BDD style testing
SCENARIO("Loading a valid mesh", "[mesh]")
{
	GIVEN("A mesh with valid data")
	{
		bool success = false;
		Mesh testMesh("testdata/singletriangle.obj", success);
		REQUIRE(success == true);  //REQUIRE => abort if fails
		WHEN("Loading the singletriangle.obj")
		{
			THEN("The position should be the center of the triangle")
			{
				/*
				 * the points of singletriangle.obj:
				 * p1(10.0, 10.0, 10.0);
				 * p2(200.0, 20.0, 20.0);
				 * p3(100.0, 200.0, 30.0);
				 *
				 * getX/Y/Z is defined by (BBoxMax + BBoxMin) / 2
				*/

				Vector3D pmin(10.0, 10.0, 10.0);
				Vector3D pmax(200.0, 200.0, 30.0);
				Vector3D centerpoint = (pmin + pmax) / 2.0;

				CHECK(testMesh.getX() == Approx(centerpoint.getX())); //CHECK => continue, even with failure
				CHECK(testMesh.getY() == Approx(centerpoint.getY()));
				CHECK(testMesh.getZ() == Approx(centerpoint.getZ()));
			}
			AND_THEN("The center of gravity should be in the middle of the triangle") //careful, each scope has its own mesh, so a new Mesh is created here!
			{
				// the points of singletriangle.obj:
				Vector3D p1(10.0, 10.0, 10.0);
				Vector3D p2(200.0, 20.0, 20.0);
				Vector3D p3(100.0, 200.0, 30.0);

				auto pCenter = (p1+p2+p3) / 3.0;

				auto cog = testMesh.getCenterOfGravity();

				CHECK(pCenter.getX() == Approx(cog.getX()));
				CHECK(pCenter.getY() == Approx(cog.getY()));
				CHECK(pCenter.getZ() == Approx(cog.getZ()));
			}
		}
	}
}
