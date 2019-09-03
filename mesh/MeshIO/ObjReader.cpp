#include "ObjReader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <clocale>
#include <list>
#include <filesystem>

#include "MtlParser.h"

using namespace std;

//tokens 0 : "f" tag for face, 1 ... n-1: obj face triple v1/vt1/vn1 ...
//----------------------------
// Allowed variants in OBJ:
//----------------------------
// f v1 v2 v3 ...
// f v1/vt1 v2/vt2 v3/vt3 ...
// f v1//vn1 v2//vn2 v3//vn3 ...
// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
bool parseFaceProperty(const std::vector<std::string>& tokens, int& faceIdx, std::vector<sFaceProperties>& rFaceProps, const std::vector<float>& textureCoordinates, MtlMaterial* material, std::unordered_map<std::string, unsigned char>& textureToIdMap)
{
	std::list<int> vertIndices;
	std::list<int> texCoordIndices;
	std::list<int> normalIndices;

	unsigned char textureId = 0;

	if(material != nullptr)
	{
		if(!material->map_Kd.empty())
		{
			textureId = textureToIdMap[material->map_Kd];
		}
	}

	for( size_t i = 1; i<tokens.size(); ++i)
	{
		std::stringstream tokenStream(tokens[i]);
		int index;
		while(!tokenStream.eof())
		{
			tokenStream >> index >> std::ws;
			vertIndices.push_back(index - 1);
			if(tokenStream.peek() == '/')	//check for first '/' => there are textureCoordinates/normals
			{
				tokenStream.get();
				if(tokenStream.peek() == '/') //check if immediate '/' => no texturecoordinates
				{
					texCoordIndices.push_back(-1);
					tokenStream.get();
					tokenStream >> index >> std::ws;
					normalIndices.push_back(index - 1);
				}
				else	//we have texturecoordinates
				{
					tokenStream >> index >> std::ws;
					texCoordIndices.push_back(index - 1);

					if(tokenStream.peek() == '/') //we have normals
					{
						tokenStream.get();
						tokenStream >> index >> std::ws;
						normalIndices.push_back(index - 1);
					}
					else
					{
						normalIndices.push_back(-1);
					}
				}
			}
			else //no texture-coordinates or normals
			{
				texCoordIndices.push_back(-1);
				normalIndices.push_back(-1);
			}
		}
	}

	if(vertIndices.size() < 3)
		return false;

	auto vertIt = vertIndices.begin();
	auto texIt = texCoordIndices.begin();
	auto normalIt = normalIndices.begin();

	int idxFirst = *vertIt++;
	int idxPrev  = *vertIt++;

	float uvXFirst = 0.0;
	float uvYFirst = 0.0;

	if(*texIt >= 0)
	{
		uvXFirst = textureCoordinates[*texIt * 2];
		uvYFirst = textureCoordinates[*texIt * 2 + 1];
	}

	++texIt;

	float uvXPrev = 0.0;
	float uvYPrev = 0.0;

	if(*texIt >= 0)
	{
		uvXPrev = textureCoordinates[*texIt * 2];
		uvYPrev = textureCoordinates[*texIt * 2 + 1];
	}

	++texIt;

	//TODO: use normals if present;
	++normalIt; ++normalIt;

	while(vertIt != vertIndices.end())
	{
		rFaceProps[faceIdx].mVertIdxA = idxFirst;
		rFaceProps[faceIdx].mVertIdxB = idxPrev;
		idxPrev = *vertIt++;
		rFaceProps[faceIdx].mVertIdxC = idxPrev;

		rFaceProps[faceIdx].textureCoordinates[0] = uvXFirst;
		rFaceProps[faceIdx].textureCoordinates[1] = uvYFirst;
		rFaceProps[faceIdx].textureCoordinates[2] = uvXPrev;
		rFaceProps[faceIdx].textureCoordinates[3] = uvYPrev;

		if(*texIt >= 0)
		{
			uvXPrev = textureCoordinates[*texIt * 2];
			uvYPrev = textureCoordinates[*texIt * 2 + 1];
		}

		rFaceProps[faceIdx].textureCoordinates[4] = uvXPrev;
		rFaceProps[faceIdx].textureCoordinates[5] = uvYPrev;

		rFaceProps[faceIdx].textureId = textureId;
		++texIt;
		++normalIt;
		++faceIdx;
	}

	return true;
}

//! Reads an plain-text Alias Wavefront OBJ file.
//!
//! After reading the file lots of other operations like meshing cleaning
//! are executed. Furthermore functions are tested here.
//!
//! see .OBJ specification: http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
bool ObjReader::readFile(const std::string &rFilename, std::vector<sVertexProperties> &rVertexProps, std::vector<sFaceProperties> &rFaceProps, MeshSeedExt& rMeshSeed)
{
	char* oldLocale = std::setlocale( LC_NUMERIC, nullptr );
	std::setlocale( LC_NUMERIC, "C" );

	// commmon variables for internal use
	string line;
	string linePrefix;

	std::unordered_map<std::string, MtlMaterial> materials;

	// Counters for .OBJ elements and lines:
	unsigned int  obj_linesTotal            = 0;
	unsigned int  obj_linesIgnoredTotal     = 0;
	unsigned int  obj_linesUnsupportedTotal = 0;
	uint64_t  obj_verticesTotal         = 0;
	unsigned int  objVerticesNormalsTotal   = 0;
	unsigned int  objTexCoordsTotal = 0;
	size_t  objFacesTotal             = 0;
	unsigned int  obj_commentsTotal         = 0;

	// for performance measurement:
	int timeStart;
	int timeStop;

	// functionality:
	ifstream fp( rFilename.c_str() );
	if( !fp.is_open() ) {
		cerr << "[ObjReader::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'.\n";
		std::setlocale( LC_NUMERIC, oldLocale );

		return false;
	}

	cout << "[ObjReader::" << __FUNCTION__ << "] File opened: '" << rFilename << "'.\n";

	std::filesystem::path prevRootPath = std::filesystem::current_path();
	std::filesystem::current_path(std::filesystem::path(rFilename).parent_path());

	// determine the amount of data by parsing the data for a start:
	while( fp.good() ) {
		fp >> linePrefix;
		// Vertex Data -------------------------------------------------------------------------
		if( linePrefix == "vt" ) {           // texture vertices
			objTexCoordsTotal++;
		} else if( linePrefix == "vn" ) {     // vertex normals
			objVerticesNormalsTotal++;
		} else if( linePrefix == "vp" ) {     // paramter space vertics
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "v" ) {      // vertices
			obj_verticesTotal++;
		} else if( linePrefix == "cstype" ) { // free-form curve/surface attributes
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "deg" ) {    // degree
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "bmat" ) {   // basis matrix
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "step" ) {   // step size
			obj_linesUnsupportedTotal++;
		} else
		// Elements ----------------------------------------------------------------------------
		if( linePrefix == "p" ) {             // point
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "l"  ) {      // line
			// obj_linesUnsupportedTotal++;
		} else if( linePrefix == "f" ) {       // Faces and triangle fans!
			bool isVertIdx = false;
			int  someIdx;
			int  indices  = 0;
			char nextByte = fp.peek();
			while( ( nextByte != '\n' ) && ( nextByte != '\r' ) && ( nextByte != -1 ) ) {
				if( ( nextByte == ' ' ) || ( nextByte == '\t' ) ) {
					nextByte = fp.get();
					nextByte = fp.peek();
					// Vertex Index is always loacted after whitespace
					isVertIdx = true;
					continue;
				}
				if( nextByte == '/' ) {
					nextByte = fp.get();
					nextByte = fp.peek();
					// No whitespace = no vertex index
					isVertIdx = false;
					continue;
				}
				fp >> someIdx;
				nextByte = fp.peek();
				if( isVertIdx ) {
					indices++;
				}
			}
			// As we can have a triangle fan here, we can compute the number of faces by:
			objFacesTotal += indices - 2;
		} else if( linePrefix == "curv" ) {   // curve
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "curv2" ) {  // 2d-curve
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "surf" ) {   // surface
			obj_linesUnsupportedTotal++;
		} else
		// Free-form curve/surface body statements ---------------------------------------------
		if( linePrefix == "parm" ) {          // parameter values
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "trim" ) {   // outer trimming loop
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "hole" ) {   // inner trimming loop
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "scrv" ) {   // special curve
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "sp" ) {     // special point
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "end" ) {    // end statement
			obj_linesUnsupportedTotal++;
		} else
		// Connectivity between free-form surfaces ---------------------------------------------
		if( linePrefix == "con" ) {          // connect
			obj_linesUnsupportedTotal++;
		} else
		// Grouping ----------------------------------------------------------------------------
		if( linePrefix == "g" ) {            // group name
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "s" ) {     // smoothing group
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "mg" ) {    // merging group
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "o" ) {     // object name
			obj_linesUnsupportedTotal++;
		} else
		// Display/render attributes -----------------------------------------------------------
		if( linePrefix == "bevel" ) {             // bevel interpolation
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "c_interp" ) {   // color interpolation
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "d_interp" ) {   // dissolve interpolation
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "lod" ) {        // level of detail
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "usemtl" ) {     // material name
			//obj_linesUnsupportedTotal++;
		} else if( linePrefix == "mtlib" ) {      // material library
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "shadow_obj" ) { // shadow casting
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "trace_obj" ) {  // ray tracing
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "ctech" ) {      // curve approximation technique
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "stech" ) {      // surface approximation technique
			obj_linesUnsupportedTotal++;
		} else if( linePrefix == "mtllib") {      // mtl libs. Parse directy, to have all materials before using them guarenteed.
			{
				std::string fileName;
				fp >> fileName;

				std::filesystem::path filePath(fileName);

				if(filePath.is_relative())
				{
					fileName = std::filesystem::absolute(filePath).string();
				}

				MtlParser parser;
				parser.parseFile(fileName);

				materials.insert(parser.getMaterialsRef().begin(), parser.getMaterialsRef().end());
			}
		} else
		// Comments ----------------------------------------------------------------------------
		if(  linePrefix == "#" ) {                 // comments
			obj_commentsTotal++;
		} else {
		// lines to ignore - in the ideal case there are none ... -----------------------------
			obj_linesIgnoredTotal++;
		}
		// fetch the rest of the line - even if it is just the end-of-line character:
		getline( fp, line );
		// may cause problems at the end of the file if not cleared:
		line.clear();
		linePrefix.clear();
	}

	// what we just found:
	cout << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesTotal            << " lines read.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << obj_verticesTotal         << " vertices found.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << objVerticesNormalsTotal   << " vertex normals found.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << objTexCoordsTotal         << " texture coordinates found.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << objFacesTotal             << " faces found.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << obj_commentsTotal         << " lines of comments.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesUnsupportedTotal << " lines not supported.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesIgnoredTotal     << " lines ignored.\n";
	cout << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";

	// allocate memory:
	rVertexProps.resize( obj_verticesTotal );
	rFaceProps.resize( objFacesTotal );

	std::vector<float> textureCoordinates(objTexCoordsTotal * 2);	//temporary buffer for s t coordinates
	size_t texCoordIndex = 0;

	MeshReader::getModelMetaDataRef().setHasTextureCoordinates(objTexCoordsTotal > 0);

	std::unordered_map<std::string, unsigned char> textureToIdMap;
	unsigned char texId = 0;
	for(const auto & material : materials)
	{
		if(!(material.second.map_Kd.empty()) )
		{
			if(textureToIdMap.find(material.second.map_Kd) == textureToIdMap.end())
			{
				textureToIdMap[material.second.map_Kd] = texId++;
			}
		}
	}

	MtlMaterial* currentMaterial = nullptr;

	// init vertex coordinates and function value
	for( size_t i=0; i<obj_verticesTotal; i++ ) {
		rVertexProps.at( i ).mCoordX  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( i ).mCoordY  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( i ).mCoordZ  = _NOT_A_NUMBER_DBL_;
		// init function value with zero as NaN results in performance issues by repeatingly checking min/max values in MeshGL::getFuncValMinMaxUser.
		rVertexProps.at( i ).mFuncVal = 0.0;
		// init colors
		rVertexProps.at( i ).mColorRed = 187;
		rVertexProps.at( i ).mColorGrn = 187;
		rVertexProps.at( i ).mColorBle = 187;
		rVertexProps.at( i ).mColorAlp = 255;
	}

	// fetching the data by parsing the data the 2nd time
	int vertexIdx       = 0;
	int faceIdx         = 0;

	// rewind file and fetch data:
	timeStart = clock();

	fp.clear();
	fp.seekg( ios_base::beg );
	uint64_t lineNr = 0;
	while( fp.good() ) {
		string someLine;
		getline( fp, someLine );
		lineNr++;
		if( someLine.length() == 0 ) {
			continue; // because of empty line OR EndOfFile.
		}
		istringstream iss( someLine );
		vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
		if( tokens.empty() ) {
			// cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: in line( " << lineNr-1 << " ) of length " << someLine.length() << "\n";
			continue; // Essentially an empty lines with just a \cr.
		}
		string firstToken = tokens.at( 0 );
		if( firstToken.compare( 0, 1, "#" ) == 0 ) { // Comments
			// Meta-Data strings stored as comments -------------------------------------------------------
			if( tokens.size() < 2 ) {
				continue;
			}
			string possibleMetaDataName = tokens.at( 1 );
			ModelMetaData::eMetaStrings foundMetaId;
			if( MeshReader::getModelMetaDataRef().getModelMetaStringId( possibleMetaDataName, foundMetaId ) ) {
				uint64_t preMetaLen = 3 + possibleMetaDataName.size(); // 1 for comment sign '#' plus 2x space.
				string metaContent = someLine.substr( preMetaLen );
				cout << "[ObjReader::" << __FUNCTION__ << "] Meta-Data: " << possibleMetaDataName << " (" << foundMetaId << ") = " << metaContent << "\n";
				if( !MeshReader::getModelMetaDataRef().setModelMetaString( foundMetaId, metaContent ) ) {
					cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: Meta-Data not set!\n";
				}
			}
			continue; // because of line with comment.
			// --------------------------------------------------------------------------------------------
		} else if( firstToken == "v" ) { // Vertex
			//cout << "someLine Vertex: " << someLine << "\n";
			auto valueCount = tokens.size()-1;
			if( ( valueCount < 3 ) || ( valueCount == 5 ) || ( valueCount > 7 ) ) {
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: Wrong token count: " << tokens.size()-1 << "!"  << "\n";
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: " << someLine << "\n";
				continue;
			}

			rVertexProps.at( vertexIdx ).mCoordX = stod( tokens.at( 1 ) );
			rVertexProps.at( vertexIdx ).mCoordY = stod( tokens.at( 2 ) );
			rVertexProps.at( vertexIdx ).mCoordZ = stod( tokens.at( 3 ) );
			if( ( valueCount == 4 ) || ( valueCount == 7 ) ) { // There is function value at the end of the line
				rVertexProps.at( vertexIdx ).mFuncVal = stod( tokens.at( valueCount ) );
			}
			if( valueCount >= 6 ) { // Strict: if( ( valueCount == 6 ) || ( valueCount == 7 ) ) {
				rVertexProps.at( vertexIdx ).mColorRed = stoi( tokens.at( 4 ) );
				rVertexProps.at( vertexIdx ).mColorGrn = stoi( tokens.at( 5 ) );
				rVertexProps.at( vertexIdx ).mColorBle = stoi( tokens.at( 6 ) );
			}
			vertexIdx++;
		} else if( firstToken == "f" ) { // faces i.e. triangles, quadtriangles and trianglestrips
			if(!parseFaceProperty(tokens, faceIdx, rFaceProps, textureCoordinates, currentMaterial, textureToIdMap))
			{
				continue;
			}

		} else if( firstToken == "vn" ) { // Vertex normals
			if( tokens.size() != 4 ) {
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: Wrong token count: " << tokens.size() << "!"  << "\n";
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: " << someLine << "\n";
				continue;
			}

			// addNormal( stod( tokens.at( 1 ) ), stod( tokens.at( 2 ) ), stod( tokens.at( 3 ) ) );
		} else if( firstToken == "l" ) { // Polygonal lines
			auto somePolyLinesIndices = new vector<int>;
			for( size_t i=1; i<tokens.size(); i++ ) { //! \todo test the import of polylines from OBJs.
				int vertIndex = stoi( tokens.at( i ) )-1; // OBJs start with ONE, while GigaMesh's indices start wit ZERO
				somePolyLinesIndices->push_back( vertIndex );
			}
			rMeshSeed.getPolyLineVertIndicesRef().push_back( somePolyLinesIndices );			
		} else if ( firstToken == "vt" ) { //texture Coordinats
			if(tokens.size() != 3)
			{
				continue;
			}
			if(texCoordIndex < textureCoordinates.size() - 1)
			{
				textureCoordinates[texCoordIndex++] = stof( tokens.at(1));
				textureCoordinates[texCoordIndex++] = stof( tokens.at(2));
			}
		} else if ( firstToken == "usemtl")
		{
			if(tokens.size() != 2)
			{
				continue;
			}
			currentMaterial = &materials[tokens[1]];
		} else {
			cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: line ignored: " << someLine << "\n";
		}
	}

	fp.close();
	timeStop  = clock();
	cout << "[ObjReader::" << __FUNCTION__ << "] fetch data from file:       " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << "\n";
	std::setlocale( LC_NUMERIC, oldLocale );
	std::filesystem::current_path(prevRootPath);
	return true;
}
