#include "TxtWriter.h"

TxtWriter::TxtWriter()
{

}


bool TxtWriter::writeFile(const std::filesystem::path& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed)
{
	std::cerr << "[TxtWriter::" << __FUNCTION__ << "] NOT IMPLEMENTED - File " << rFilename << "not written!" << std::endl;
	std::cerr << "[TxtWriter::" << __FUNCTION__ << "] Vertex count:  " << rVertexProps.size() << "not written!" << std::endl;
	std::cerr << "[TxtWriter::" << __FUNCTION__ << "] Face count:  " << rFaceProps.size() << "not written!" << std::endl;

	return false;
}
