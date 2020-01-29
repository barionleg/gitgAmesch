#ifndef MESHREADER_H
#define MESHREADER_H

#include "../meshseedext.h"
#include "ModelMetaData.h"

class MeshReader
{
	public:
		MeshReader();
		virtual ~MeshReader() = default;

		virtual bool readFile( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed ) = 0;

		ModelMetaData& getModelMetaDataRef();

	private:
		ModelMetaData mModelMetaData;
};

#endif // MESHREADER_H
