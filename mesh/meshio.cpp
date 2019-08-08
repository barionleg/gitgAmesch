#include "meshio.h"

#include <chrono>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <algorithm>  // e.g. ::transform
#include <sstream>    // stringstream
#include <iterator>   // istream_iterator

#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <cctype>

#include <filesystem>
#include <chrono>

#include "primitive.h"
#include "../normalSphereSelection/IcoSphereTree.h"
#include "MeshIO/ObjReader.h"
#include "MeshIO/PlyReader.h"
#include "MeshIO/TxtReader.h"
#include "MeshIO/RegularGridTxtReader.h"

#include "MeshIO/PlyWriter.h"
#include "MeshIO/ObjWriter.h"
#include "MeshIO/VRMLWriter.h"
#include "MeshIO/TxtWriter.h"

using namespace std;

//! Constructor
//! checks for endianness as suggested here: http://www.ibm.com/developerworks/aix/library/au-endianc/index.html?ca=drs-#list5
MeshIO::MeshIO()
{
	int  intTest = 1;
	mSystemIsBigEndian = ( (*reinterpret_cast<char*>(&intTest)) == 0 );
	if( mSystemIsBigEndian ) {
		std::cout << "[MeshIO::" << __FUNCTION__ << "] System is BIG Endian." << std::endl;
	} else {
		std::cout << "[MeshIO::" << __FUNCTION__ << "] System is LITTLE Endian." << std::endl;
	}

	// Init flags:
	for( bool& exportFlag : mExportFlags ) {
		exportFlag = true;
	}

	// Initialize strings holding meta-data
	if( !mModelMetaData.clearModelMetaStrings() ) {
		std::cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: clearModelMetaStrings failed!" << std::endl;
	}

	mExportFlags[EXPORT_TEXTURE_COORDINATES] = true;	//! \todo remove this, and set it elsewhere (e.g. based on if the mesh has actual texture-coordinates or not)
}

//! Destructor
MeshIO::~MeshIO() {

}

// READ ------------------------------------------------------------------------

//! Reads a file with a 3D-mesh. This method wraps around the other read-methods.
//! The method for reading a file is automatically choosen by its file extension.
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::readFile(
                const string& rFileName,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	// Extension - lower case and without dot
	std::string fileExtension = std::filesystem::path( rFileName ).extension().string();
	for( char& character : fileExtension ) {
		character = static_cast<char>( std::tolower( static_cast<unsigned char>( character ) ) );
	}
	if( fileExtension.at(0) == '.' ) {
		fileExtension.erase( fileExtension.begin() );
	}
	cout << "[MeshIO::" << __FUNCTION__ << "] Extension (lowercase): " << fileExtension << endl;

	bool readStatus = false;
	std::unique_ptr<MeshReader> reader = nullptr;
	if( fileExtension == "obj" ) {
		reader = std::make_unique<ObjReader>();
		//readStatus = readOBJ( rFileName, rVertexProps, rFaceProps );
	} else if( fileExtension == "txt" ) {
		bool askRegular;
		if( !readIsRegularGrid( &askRegular ) ) {
			cerr << "[MeshIO::" << __FUNCTION__ << "] Read TXT file canceled!" << endl;
			return( false );
		}
		if( askRegular ) {
			reader = std::make_unique<RegularGridTxtReader>();
		} else {
			reader = std::make_unique<TxtReader>();
		}
	} else if( fileExtension == "xyz" ) {
		reader = std::make_unique<TxtReader>();
	} else if( fileExtension == "ply" ) {
		reader = std::make_unique<PlyReader>();
		dynamic_cast<PlyReader*>(reader.get())->setIsBigEndian(mSystemIsBigEndian);
	} else {
		cerr << "[MeshIO::" << __FUNCTION__ << "] Error: Unknown extension/type '" << fileExtension << "' specified!" << endl;
		return( false );
	}

	if(reader != nullptr)
	{
		readStatus = reader->readFile(rFileName, rVertexProps, rFaceProps, *this);
	}

	if( !readStatus ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Read failed!" << endl;
		return( false );
	}

	mModelMetaData = reader->getModelMetaDataRef();

	if(!mModelMetaData.getModelMetaString(ModelMetaData::META_TEXTUREFILE).empty())
	{
		std::filesystem::path texturePath(mModelMetaData.getModelMetaString(ModelMetaData::META_TEXTUREFILE));
		if(texturePath.is_relative())
		{
			auto prevPath = std::filesystem::current_path();
			std::filesystem::current_path(std::filesystem::absolute(rFileName).parent_path());
			mModelMetaData.setModelMetaString(ModelMetaData::META_TEXTUREFILE, std::filesystem::absolute( mModelMetaData.getModelMetaString(ModelMetaData::META_TEXTUREFILE)).string());
			std::filesystem::current_path(prevPath);
		}
	}

	mExportFlags[EXPORT_TEXTURE_COORDINATES]= !(mModelMetaData.getModelMetaString(ModelMetaData::META_TEXTUREFILE).empty());

	// Store filename with absolute path.
	mFileNameFull = std::filesystem::absolute( rFileName ).string();
	cout << "[MeshIO::" << __FUNCTION__ << "] File - Absolute:    " << mFileNameFull << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] File - Stem:        " << std::filesystem::path( mFileNameFull ).stem() << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] File - Extension:   " << std::filesystem::path( mFileNameFull ).extension() << endl;

	return( true );
}

//! Ask the user if a regular grid is to be read.
//! Has to be overloaded by GUI class e.g. MeshQt.
//! Returns false by default.
bool MeshIO::readIsRegularGrid( bool* rIsGrid ) {
	char userAnswer = 'n';
	std::cout << "[MeshIO::" << __FUNCTION__ << "] USER Input: Is this file a regular grid [y/N]? ";
	std::cin >> userAnswer;
	if( userAnswer == 'y' || userAnswer == 'Y' ) {
		(*rIsGrid) = true;
		std::cout << "[MeshIO::" << __FUNCTION__ << "] Your answer was: YES." << std::endl;
	} else {
		(*rIsGrid) = false;
		std::cout << "[MeshIO::" << __FUNCTION__ << "] Your answer was: NO." << std::endl;
	}
	return( true );
}

// Texturemap ----------------------------------------------------------------------

//! Imports a texture map (per Vertex) with the format int originalIndex, float red,
//! float green, float blue from a given ASCII file.
//!
//! Note missing Vertex indices will get a default color (specified elsewhere).
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::importTEXMap(const string&          rFileName,
                           int*            rNrLines,
                           uint64_t** rRefToPrimitves,
                           unsigned char** rTexMap
) {
	fstream filestr;
	string  lineToParse;

	int timeStart, timeStop; // for performance mesurement

	timeStart = clock();
	filestr.open( rFileName.c_str(), fstream::in );
	if( !filestr.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] Could not open: '" << rFileName << "'!" << endl;
		return( false );
	}
	cout << "[MeshIO::" << __FUNCTION__ << "] Open: '" << rFileName << "'" << endl;

	// count lines to be read to ....
	while( !filestr.eof() ) {
		getline( filestr, lineToParse );
		(*rNrLines)++;
	}
	(*rNrLines)--;
	cout << "[MeshIO::" << __FUNCTION__ << "] Lines to read: " << (*rNrLines) << "." << endl;

	// initalize values:
	*rNrLines        = 0;
	*rRefToPrimitves = nullptr;
	*rTexMap         = nullptr;
	float texRtmp;
	float texGtmp;
	float texBtmp;

	// ... allocate enough memory:
	*rRefToPrimitves = static_cast<uint64_t*>(calloc( (*rNrLines), sizeof(uint64_t) ));
	*rTexMap = static_cast<unsigned char*>(calloc( (*rNrLines)*3, sizeof(unsigned char) ));

	filestr.clear(); // since the stream object has encountered eof it is in a bad state
	filestr.seekg( 0, ios::beg ); // now rewind (won't work without the previous call of clear).

	for( int i=0; i<(*rNrLines); i++ ) {
		getline( filestr, lineToParse );
		int refToPrimitveIdx;
		if( sscanf( lineToParse.c_str(), "%i %f %f %f", &refToPrimitveIdx, &texRtmp, &texGtmp, &texBtmp ) == 4 ) {
			(*rRefToPrimitves)[i] = static_cast<uint64_t>(refToPrimitveIdx);
			(*rTexMap)[i*3]   = static_cast<unsigned char>(texRtmp*255);
			(*rTexMap)[i*3+1] = static_cast<unsigned char>(texGtmp*255);
			(*rTexMap)[i*3+2] = static_cast<unsigned char>(texBtmp*255);
		} else {
			cerr << "[MeshIO::" << __FUNCTION__ << "] Problem in parsing line no. " << i << " of " << rFileName.c_str() << endl;
		}
	}

	filestr.close();

	timeStop  = clock();
	cout << "[MeshIO::" << __FUNCTION__ << "] took " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	
	return( true );
}

//! Import an ASCII file with normal vectors.
//! Expected format: <integer/primitive_id> <double/x-component> <double/y-component> <double/z-component>
bool MeshIO::importNormals( const string& rFileName, vector<grVector3ID>* rNormals ) {
	std::fstream fileStream;
	fileStream.open( rFileName, std::fstream::in );
	if( !fileStream.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Could not open " << rFileName.c_str() << "!" << endl;
		return false;
	}
	cout << "[MeshIO::" << __FUNCTION__ << "] Reading from '" << rFileName << "'" << endl;
	int primID;
	while( fileStream >> primID ) {
		double normalX;
		double normalY;
		double normalZ;
		fileStream >> normalX;
		fileStream >> normalY;
		fileStream >> normalZ;
		rNormals->push_back( grVector3ID{ primID, normalX, normalY, normalZ } );
		//cout << "[MeshIO::" << __FUNCTION__ << "] " << primID << " " << normalX << " " << normalY << " " << normalZ << endl;
	}
	if( !fileStream.eof() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: File '" << rFileName.c_str() << "' not parsed to its end!" << endl;
		fileStream.close();
		return false;
	}
	fileStream.close();
	return true;
}

// WRITE -----------------------------------------------------------------------

//! Set an export flag see MeshIO::eExportFlags
bool MeshIO::setFlagExport( eExportFlags rFlag, bool rSetTo ) {
	mExportFlags[rFlag] = rSetTo;
	return true;
}

//! Write a Mesh to a file using a filename given by the user.
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::writeFileUserInteract() {
	// STUB
	cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Not impemented!" << endl;
	return( false );
}

//! Write a Mesh to a file. The file type is automatically determined by the file
//! extension. See methods like: MeshIO::writeOBJ, MeshIO::writePLY and MeshIO::writeVRML
bool MeshIO::writeFile(
                const string& rFileName,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	size_t foundDot;
	foundDot = rFileName.rfind( '.' );
	if( foundDot == string::npos ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] No extension/type for file '" << rFileName << "' specified!" << endl;
		return false;
	}

	string fileExtension = rFileName.substr( ++foundDot );

	for(char& character : fileExtension)
	{
		character = std::tolower(character);
	}

	cout << "[MeshIO::" << __FUNCTION__ << "] extension: " << fileExtension << endl;
	if( mExportFlags[EXPORT_BINARY] ) {
		cout << "[MeshIO::" << __FUNCTION__ << "] BINARY." << endl;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ << "] ASCII." << endl;
	}

	std::unique_ptr<MeshWriter> writer = nullptr;

	bool fileWriteOk = false;
	if( fileExtension == "obj" ) {
		writer = std::make_unique<ObjWriter>();
	}
	else if( fileExtension == "wrl" ) {
		writer = std::make_unique<VRMLWriter>();
	}
	else if( fileExtension == "txt" ) {
		writer = std::make_unique<TxtWriter>();
	}
	else if( fileExtension == "xyz" ) {
		writer = std::make_unique<TxtWriter>();
	}
	else if( fileExtension == "ply" ) {
		writer = std::make_unique<PlyWriter>();
	}

	if(writer != nullptr)
	{
		writer->setModelMetaData(mModelMetaData);

		writer->setIsBigEndian(mSystemIsBigEndian);
		writer->setExportBinary(mExportFlags[EXPORT_BINARY]);
		writer->setExportVertColor(mExportFlags[EXPORT_VERT_COLOR]);
		writer->setExportVertNormal(mExportFlags[EXPORT_VERT_NORMAL]);
		writer->setExportVertFlags(mExportFlags[EXPORT_VERT_FLAGS]);
		writer->setExportVertLabel(mExportFlags[EXPORT_VERT_LABEL]);
		writer->setExportVertFeatureVector(mExportFlags[EXPORT_VERT_FTVEC]);
		writer->setExportPolyline(mExportFlags[EXPORT_POLYLINE]);
		writer->setExportTextureCoordinates(mExportFlags[EXPORT_TEXTURE_COORDINATES]);

		fileWriteOk = writer->writeFile(rFileName, rVertexProps, rFaceProps, *this);
	}

	if( fileWriteOk ) {
		mFileNameFull = rFileName;
		return true;
	}
	cerr << "[MeshIO::" << __FUNCTION__ << "] Unknown extension/type '" << fileExtension << "' specified!" << endl;

	return false;
}

ModelMetaData& MeshIO::getModelMetaDataRef()
{
	return mModelMetaData;
}

// Provide Information ---------------------------------------------------------

//! Get the extension of the file name in lower case without dot.
//!
//! @returns extension of the file name in lower case without dot.
string MeshIO::getFileExtension() {
	// Extension - lower case and without dot
	std::string fileExtension = std::filesystem::path( mFileNameFull ).extension().string();
	for( char& character : fileExtension ) {
		character = static_cast<char>( std::tolower( static_cast<unsigned char>( character ) ) );
	}
	if( fileExtension.at(0) == '.' ) {
		fileExtension.erase( fileExtension.begin() );
	}
	return fileExtension;
}

//! Returns the base name = rFileName without extension nor path.
string MeshIO::getBaseName() {
	return std::filesystem::path( mFileNameFull ).stem().string();
}

//! Returns the path of the file.
string MeshIO::getFileLocation() {
	return std::filesystem::path( mFileNameFull ).parent_path().string();
}

//! Returns the full name and path of the file.
string MeshIO::getFullName() {
	return mFileNameFull;
}

bool MeshIO::writeIcoNormalSphereData(const string& rFilename, const std::vector<sVertexProperties>& rVertexProps, int subdivisions, bool sphereCoordinates)
{
	fstream filestr;
	filestr.imbue(std::locale("C"));
	filestr.open( rFilename.c_str(), fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[MeshIO] Could not open file: '" << rFilename << "'." << endl;
		return false;
	} else {
		cout << "[MeshIO] File open for writing: '" << rFilename << "'." << endl;
	}

	IcoSphereTree icoSphereTree(subdivisions);

	for(const auto& vertexProp : rVertexProps)
	{
		size_t selIndex = icoSphereTree.getNearestVertexIndexAt(Vector3D(vertexProp.mNormalX, vertexProp.mNormalY, vertexProp.mNormalZ));
		icoSphereTree.incData(selIndex);
	}

	std::vector<float> normals = icoSphereTree.getVertices();
	const std::vector<unsigned int>& normalNums = *icoSphereTree.getVertexDataP();

	for(size_t i = 0; i<normalNums.size(); ++i)
	{
		float* n = &normals[i*3];
		if(sphereCoordinates)
		{
			float theta = acos(n[2]);
			float phi = atan2(n[1], n[0]);

			filestr << theta << "," << phi << "," << normalNums[i] << "\n";
		}
		else
		{
			filestr << n[0] << "," << n[1] << "," << n[2] << "," << normalNums[i] << "\n";
		}

	}

	filestr.close();

	return true;
}
