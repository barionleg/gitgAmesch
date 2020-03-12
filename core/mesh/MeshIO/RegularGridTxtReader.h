#ifndef REGULARGRIDTXTREADER_H
#define REGULARGRIDTXTREADER_H

#include "MeshReader.h"

class RegularGridTxtReader : public MeshReader
{
	public:
		RegularGridTxtReader() = default;

		// MeshReader interface
	public:
		bool readFile(const std::filesystem::path& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;
};

#endif // REGULARGRIDTXTREADER_H
