#ifndef TXTREADER_H
#define TXTREADER_H

#include "MeshReader.h"

class TxtReader : public MeshReader
{
	public:
		TxtReader() = default;

		// MeshReader interface
	public:
		bool readFile(const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;
};

#endif // TXTREADER_H
