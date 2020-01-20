#ifndef VRMLWRITER_H
#define VRMLWRITER_H

#include "MeshWriter.h"

class VRMLWriter : public MeshWriter
{
	public:
		VRMLWriter() = default;

		// MeshWriter interface
	public:
		bool writeFile(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;
};

#endif // VRMLWRITER_H
