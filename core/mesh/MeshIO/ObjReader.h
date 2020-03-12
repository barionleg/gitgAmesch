#ifndef OBJREADER_H
#define OBJREADER_H

#include <vector>
#include "MeshReader.h"

class ObjReader : public MeshReader
{
	public:
		ObjReader() = default;
		~ObjReader() override = default;
		bool readFile( const std::filesystem::path& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed ) override;
};

#endif // OBJREADER_H
