#ifndef TXTWRITER_H
#define TXTWRITER_H

#include "MeshWriter.h"

class TxtWriter : public MeshWriter
{
	public:
		TxtWriter();

		// MeshWriter interface
	public:
		bool writeFile(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;
};

#endif // TXTWRITER_H
