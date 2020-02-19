#include <catch.hpp>
#include "../core/mesh/MeshIO/ObjReader.h"
#include "../core/mesh/MeshIO/PlyReader.h"
#include "../core/mesh/MeshIO/TxtWriter.h"
#include <GigaMesh/mesh/meshio.h>

const std::string gTestFilesPath("testdata/");

//unit test => check if the reader to their work properly
TEST_CASE("Mesh Reader Tests", "[meshio]")
{
	std::vector<sVertexProperties> vertexProperties;
	std::vector<sFaceProperties> faceProperties;
	MeshSeedExt meshSeed;

	SECTION("ObjReader - reading plain obj")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "cube.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 8);
		CHECK(faceProperties.  size() == 12);
	}

	SECTION("ObjReader - reading obj with quads")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "flat.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 25);
		CHECK(faceProperties.  size() == 16);

		for(auto& face : faceProperties)
		{
			REQUIRE(face.vertexIndices.size() == 4);
		}
	}

	SECTION("ObjReader - reading ngon obj")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "ngon_concave.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 5);
		REQUIRE(faceProperties.size() == 1);
		CHECK(faceProperties[0].vertexIndices.size() == 5);
		CHECK(faceProperties[0].textureCoordinates.size() == 0);
	}

	SECTION("PlyReader - reading ngon ascii ply")
	{
		PlyReader reader;
		reader.readFile(gTestFilesPath + "ngon_concave.ply", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 5);
		REQUIRE(faceProperties.size() == 1);
		CHECK(faceProperties[0].vertexIndices.size() == 5);
		CHECK(faceProperties[0].textureCoordinates.size() == 0);
	}
}


//unit test => no need to check if the mesh was loaded correctly, only that it was loaded
//==> correct functionality is checked by the unit test
TEST_CASE("MeshIO integration test", "[meshio]")
{
	MeshIO meshIO;

	std::vector<sVertexProperties> vertexProperties;
	std::vector<sFaceProperties> faceProperties;
	SECTION("MeshIO obj integration")
	{
		meshIO.readFile(gTestFilesPath + "singletriangle.obj", vertexProperties, faceProperties);

		CHECK(vertexProperties.empty() == false);
		CHECK(faceProperties.  empty() == false);

		auto baseName = meshIO.getBaseName();
		REQUIRE(baseName == "singletriangle");
		auto extension = meshIO.getFileExtension();
		REQUIRE(extension == "obj");

		auto filePath = meshIO.getFileLocation();
		auto fullName = meshIO.getFullName();

		REQUIRE(fullName == filePath + baseName + "." + extension);
	}
}
