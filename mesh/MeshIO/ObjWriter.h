#ifndef OBJWRITER_H
#define OBJWRITER_H

#include "MeshWriter.h"

class ObjWriter : public MeshWriter
{
	public:
		ObjWriter();

		// MeshWriter interface
	public:
		bool writeFile(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;
};

#endif // OBJWRITER_H
