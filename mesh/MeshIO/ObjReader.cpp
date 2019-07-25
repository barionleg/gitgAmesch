#include "ObjReader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <clocale>

using namespace std;

ObjReader::ObjReader()
{

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
	int timeStart, timeStop;

	// functionality:
	ifstream fp( rFilename.c_str() );
	if( !fp.is_open() ) {
		cerr << "[ObjReader::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'.\n";
		std::setlocale( LC_NUMERIC, oldLocale );
		return false;
	} else {
		cout << "[ObjReader::" << __FUNCTION__ << "] File opened: '" << rFilename << "'.\n";
	}

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
			obj_linesUnsupportedTotal++;
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

	std::vector<float> textureCoordinates(objTexCoordsTotal * 2);	//temporaroy buffer for s t coordinates
	size_t texCoordIndex = 0;

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
		if( tokens.size() == 0 ) {
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
			auto faceCount = tokens.size()-3;
			if( faceCount > 0 ) {
				int idxFirst = stoi( tokens.at( 1 ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
				int idxPrev  = stoi( tokens.at( 2 ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
				for( int i=0; i<faceCount; i++ ) {
					rFaceProps[faceIdx].mVertIdxA = idxFirst;
					rFaceProps[faceIdx].mVertIdxB = idxPrev;
					idxPrev = stoi( tokens.at( 3+i ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
					rFaceProps[faceIdx].mVertIdxC = idxPrev;
					//cout << "[ObjReader::" << __FUNCTION__ << "] Face: " << facesMeshed[faceIdx*3] << " " << facesMeshed[faceIdx*3+1] << " " << facesMeshed[faceIdx*3+2] << "\n";
					faceIdx++;
				}
				//----------------------------
				// Set normal references and texture coordinates
				//----------------------------
				// Allowed variants in OBJ:
				//----------------------------
				// f v1 v2 v3 ...
				// f v1/vt1 v2/vt2 v3/vt3 ...
				// f v1//vn1 v2//vn2 v3//vn3 ...
				// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
				//cout << "[ObjReader::" << __FUNCTION__ << "] Line: " << someLine << "\n";
				for( int i=0; i<faceCount+2; i++ ) {
					const string& currentToken = tokens.at( i );
					size_t lastSlash  = currentToken.find_last_of( '/' );
					size_t firstSlash = currentToken.find_first_of( '/' );
					if( lastSlash != string::npos ) {
						//int textureIdx;
						if( lastSlash == firstSlash ) { // f v1/vt1 v2/vt2 v3/vt3 ...
							cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: f v1/vt1 v2/vt2 v3/vt3 ... not implemented!\n";
						}
						if( lastSlash == firstSlash+1 ) { // f v1//vn1 v2//vn2 v3//vn3 ...
							string normalIdxStr = currentToken.substr( lastSlash+1 );
							int normalIdx = stoi( normalIdxStr )-1;
							// addVertNormalIdx( vertIdx, normalIdx );
							//cout << "[ObjReader::" << __FUNCTION__ << "] Normal idx: " << normalIdx << " <- " << vertIdx << "\n";
						} else { // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
							cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ... not implemented!\n";
						}
					}
				}
			} else {
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: Wrong face count: " << faceCount << "!"  << "\n";
				cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: " << someLine << "\n";
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
			vector<int>* somePolyLinesIndices = new vector<int>;
			for( unsigned int i=1; i<tokens.size(); i++ ) { //! \todo test the import of polylines from OBJs.
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
		} else {
			cerr << "[ObjReader::" << __FUNCTION__ << "] ERROR: line ignored: " << someLine << "\n";
		}
	}

	fp.close();
	timeStop  = clock();
	cout << "[ObjReader::" << __FUNCTION__ << "] fetch data from file:       " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << "\n";
	std::setlocale( LC_NUMERIC, oldLocale );

	return true;
}
