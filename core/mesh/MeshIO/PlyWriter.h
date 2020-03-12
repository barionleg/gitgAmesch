#ifndef PLYWRITER_H
#define PLYWRITER_H

#include "MeshWriter.h"

class PlyWriter : public MeshWriter
{
	public:
		PlyWriter();
		~PlyWriter() override = default;

		// MeshWriter interface
	public:
		bool writeFile(const std::filesystem::path& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;

};

#endif // PLYWRITER_H
