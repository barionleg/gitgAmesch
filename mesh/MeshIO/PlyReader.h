#ifndef PLYREADER_H
#define PLYREADER_H

#include "MeshReader.h"
#include <vector>

class PlyReader : public MeshReader
{
	public:
		PlyReader();
		~PlyReader() override = default;

		bool readFile( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed ) override;

		void setIsBigEndian(bool bigEndian);
	private:

		bool   mSystemIsBigEndian; //!< Flag for proper Byte ordering during write/read.
};

#endif // PLYREADER_H
