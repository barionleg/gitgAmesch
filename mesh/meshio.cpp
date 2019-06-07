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

#define uint unsigned int
#define READ_IN_PROPER_BYTE_ORDER( filestream, target, size, reverse ) { \
	if( not( reverse ) || ( (size) == 1 ) ) { \
	    (filestream).read( reinterpret_cast<char*>(target), (size) ); \
	} else { \
		char* tmpBufRev = new char[size]; \
		char* tmpBuf = new char[size]; \
	    (filestream).read( tmpBufRev, size ); \
	    for( int i=0; i<(size); i++ ) { \
	        tmpBuf[i] = tmpBufRev[(size)-1-i]; \
		} \
	    memcpy( target, tmpBuf, size ); \
		delete[] tmpBufRev; \
		delete[] tmpBuf; \
	} \
\
}

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
	if( !clearModelMetaStrings() ) {
		std::cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: clearModelMetaStrings failed!" << std::endl;
	}
}

//! Destructor
MeshIO::~MeshIO() {
//	if( facesMeshed ) {
//		delete[] facesMeshed;
//		facesMeshed  = nullptr;
//	}
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
	if( fileExtension == "obj" ) {
		readStatus = readOBJ( rFileName, rVertexProps, rFaceProps );
	} else if( fileExtension == "txt" ) {
		bool askRegular;
		if( !readIsRegularGrid( &askRegular ) ) {
			cerr << "[MeshIO::" << __FUNCTION__ << "] Read TXT file canceled!" << endl;
			return( false );
		}
		if( askRegular ) {
			readStatus = readTXTRegularGrid( rFileName, rVertexProps, rFaceProps );
		} else {
			readStatus = readTXT( rFileName, rVertexProps, rFaceProps );
		}
	} else if( fileExtension == "xyz" ) {
		readStatus = readTXT( rFileName, rVertexProps, rFaceProps );
	} else if( fileExtension == "ply" ) {
		readStatus = readPLY( rFileName, rVertexProps, rFaceProps );
	} else {
		cerr << "[MeshIO::" << __FUNCTION__ << "] Error: Unknown extension/type '" << fileExtension << "' specified!" << endl;
		return( false );
	}

	if( readStatus != true ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Read failed!" << endl;
		return( false );
	}

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

//! Reads an plain-text Alias Wavefront OBJ file.
//!
//! After reading the file lots of other operations like meshing cleaning
//! are executed. Furthermore functions are tested here.
//!
//! see .OBJ specification: http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
bool MeshIO::readOBJ(
                const string& rFilename,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	// commmon variables for internal use
	string line;
	string linePrefix;

	// Counters for .OBJ elements and lines:
	int  obj_linesTotal            = 0;
	int  obj_linesIgnoredTotal     = 0;
	int  obj_linesUnsupportedTotal = 0;
	uint64_t  obj_verticesTotal         = 0;
	int  objVerticesNormalsTotal   = 0;
	size_t  objFacesTotal             = 0;
	int  obj_commentsTotal         = 0;

	// for performance measurement:
	int timeStart, timeStop;

	// functionality:
	ifstream fp( rFilename.c_str() );
	if( !fp.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'." << endl;
		return false;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ << "] File opened: '" << rFilename << "'." << endl;
	}

	// determine the amount of data by parsing the data for a start:
	while( fp.good() ) {
		fp >> linePrefix;
		// Vertex Data -------------------------------------------------------------------------
		if( linePrefix == "vt" ) {           // texture vertices
			obj_linesUnsupportedTotal++;
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
	cout << "[MeshIO::" << __FUNCTION__ << "] " << obj_linesTotal            << " lines read."                 << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] =========================================================" << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << obj_verticesTotal         << " vertices found."             << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << objVerticesNormalsTotal   << " vertex normals found."       << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << objFacesTotal             << " faces found."           << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] =========================================================" << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << obj_commentsTotal         << " lines of comments."          << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << obj_linesUnsupportedTotal << " lines not supported."        << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] " << obj_linesIgnoredTotal     << " lines ignored."              << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] =========================================================" << endl;

	// allocate memory:
	rVertexProps.resize( obj_verticesTotal );
	rFaceProps.resize( objFacesTotal );

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

	// Check locale formating of floating point numbers.
	bool replaceDots;
	if( !systemTestFloatDotColon( &replaceDots ) ) {
		return false;
	}
	replaceDots = not( replaceDots );

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
			// cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: in line( " << lineNr-1 << " ) of length " << someLine.length() << endl;
			continue; // Essentially an empty lines with just a \cr.
		}
		string firstToken = tokens.at( 0 );
		if( firstToken.compare( 0, 1, "#" ) == 0 ) { // Comments
			// Meta-Data strings stored as comments -------------------------------------------------------
			if( tokens.size() < 2 ) {
				continue;
			}
			string possibleMetaDataName = tokens.at( 1 );
			eMetaStrings foundMetaId;
			if( getModelMetaStringId( possibleMetaDataName, foundMetaId ) ) {
				uint64_t preMetaLen = 3 + possibleMetaDataName.size(); // 1 for comment sign '#' plus 2x space.
				string metaContent = someLine.substr( preMetaLen );
				cout << "[MeshIO::" << __FUNCTION__ << "] Meta-Data: " << possibleMetaDataName << " (" << foundMetaId << ") = " << metaContent << std::endl;
				if( !setModelMetaString( foundMetaId, metaContent ) ) {
					cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Meta-Data not set!" << std::endl;
				}
			}
			continue; // because of line with comment.
			// --------------------------------------------------------------------------------------------
		} else if( firstToken == "v" ) { // Vertex
			//cout << "someLine Vertex: " << someLine << endl;
			int valueCount = tokens.size()-1;
			if( ( valueCount < 3 ) || ( valueCount == 5 ) || ( valueCount > 7 ) ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Wrong token count: " << tokens.size()-1 << "!"  << endl;
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: " << someLine << endl;
				continue;
			}
			if( replaceDots ) {
				std::replace( tokens.at( 1 ).begin(), tokens.at( 1 ).end(), '.', ',' );
				std::replace( tokens.at( 2 ).begin(), tokens.at( 2 ).end(), '.', ',' );
				std::replace( tokens.at( 3 ).begin(), tokens.at( 3 ).end(), '.', ',' );
			}
			rVertexProps.at( vertexIdx ).mCoordX = stod( tokens.at( 1 ) );
			rVertexProps.at( vertexIdx ).mCoordY = stod( tokens.at( 2 ) );
			rVertexProps.at( vertexIdx ).mCoordZ = stod( tokens.at( 3 ) );
			if( ( valueCount == 4 ) || ( valueCount == 7 ) ) { // There is function value at the end of the line
				if( replaceDots ) {
					std::replace( tokens.at( valueCount ).begin(), tokens.at( valueCount ).end(), '.', ',' );
				}
				rVertexProps.at( vertexIdx ).mFuncVal = stod( tokens.at( valueCount ) );
			}
			if( valueCount >= 6 ) { // Strict: if( ( valueCount == 6 ) || ( valueCount == 7 ) ) {
				rVertexProps.at( vertexIdx ).mColorRed = stoi( tokens.at( 4 ) );
				rVertexProps.at( vertexIdx ).mColorGrn = stoi( tokens.at( 5 ) );
				rVertexProps.at( vertexIdx ).mColorBle = stoi( tokens.at( 6 ) );
			}
			vertexIdx++;
		} else if( firstToken == "f" ) { // faces i.e. triangles, quadtriangles and trianglestrips
			int faceCount = tokens.size()-3;
			if( faceCount > 0 ) {
				int idxFirst = stoi( tokens.at( 1 ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
				int idxPrev  = stoi( tokens.at( 2 ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
				for( int i=0; i<faceCount; i++ ) {
					rFaceProps[faceIdx].mVertIdxA = idxFirst;
					rFaceProps[faceIdx].mVertIdxB = idxPrev;
					idxPrev = stoi( tokens.at( 3+i ) )-1; // GigaMesh indices from 0 - while OBJ starts indexing with 1
					rFaceProps[faceIdx].mVertIdxC = idxPrev;
					//cout << "[MeshIO::" << __FUNCTION__ << "] Face: " << facesMeshed[faceIdx*3] << " " << facesMeshed[faceIdx*3+1] << " " << facesMeshed[faceIdx*3+2] << endl;
					faceIdx++;
				}
				//----------------------------
				// Set normal references and texture coordinates
				//----------------------------
				// Allowed variants in OBJ:
				//----------------------------
				// f v1/vt1 v2/vt2 v3/vt3 ...
				// f v1//vn1 v2//vn2 v3//vn3 ...
				// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
				//cout << "[MeshIO::" << __FUNCTION__ << "] Line: " << someLine << endl;
				for( int i=0; i<faceCount+2; i++ ) {
					string currentToken = tokens.at( i );
					size_t lastSlash  = currentToken.find_last_of( '/' );
					size_t firstSlash = currentToken.find_first_of( '/' );
					if( lastSlash != string::npos ) {
						//cout << "[MeshIO::" << __FUNCTION__ << "] slashes: " << firstSlash << " " << lastSlash << " " << currentToken.length() << endl;
						string vertIdxStr = currentToken.substr( 0, firstSlash );
						int vertIdx = stoi( vertIdxStr )-1;
						//int textureIdx;
						if( lastSlash == firstSlash ) { // f v1/vt1 v2/vt2 v3/vt3 ...
							cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: f v1/vt1 v2/vt2 v3/vt3 ... not implemented!" << endl;
						}
						if( lastSlash == firstSlash+1 ) { // f v1//vn1 v2//vn2 v3//vn3 ...
							string normalIdxStr = currentToken.substr( lastSlash+1 );
							int normalIdx = stoi( normalIdxStr )-1;
							// addVertNormalIdx( vertIdx, normalIdx );
							//cout << "[MeshIO::" << __FUNCTION__ << "] Normal idx: " << normalIdx << " <- " << vertIdx << endl;
						} else { // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
							cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ... not implemented!" << endl;
						}
					}
				}
			} else {
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Wrong face count: " << faceCount << "!"  << endl;
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: " << someLine << endl;
				continue;
			}
		} else if( firstToken == "vn" ) { // Vertex normals
			if( tokens.size() != 4 ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Wrong token count: " << tokens.size() << "!"  << endl;
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: " << someLine << endl;
				continue;
			}
			if( replaceDots ) {
				std::replace( tokens.at( 1 ).begin(), tokens.at( 1 ).end(), '.', ',' );
				std::replace( tokens.at( 2 ).begin(), tokens.at( 2 ).end(), '.', ',' );
				std::replace( tokens.at( 3 ).begin(), tokens.at( 3 ).end(), '.', ',' );
			}
			// addNormal( stod( tokens.at( 1 ) ), stod( tokens.at( 2 ) ), stod( tokens.at( 3 ) ) );
		} else if( firstToken == "l" ) { // Polygonal lines
			vector<int>* somePolyLinesIndices = new vector<int>;
			for( unsigned int i=1; i<tokens.size(); i++ ) { //! \todo test the import of polylines from OBJs.
				int vertIndex = stoi( tokens.at( i ) )-1; // OBJs start with ONE, while GigaMesh's indices start wit ZERO
				somePolyLinesIndices->push_back( vertIndex );
			}
			mPolyLineVertIndices.push_back( somePolyLinesIndices );
		} else {
			cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: line ignored: " << someLine << endl;
		}
	}

	fp.close();
	timeStop  = clock();
	cout << "[MeshIO::" << __FUNCTION__ << "] fetch data from file:       " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	return true;
}

bool MeshIO::readTXT(
                const string& rFilename,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	//! Reads a simple ASCII file having X,Y,Z and R,G,B per line.

	auto oldLocale = std::setlocale(LC_NUMERIC,"");
	std::setlocale(LC_NUMERIC,"C");


	fstream filestr;
	string  lineToParse;
	int     linesRead     = 1;
	uint64_t     linesVertices = 0;
	bool    rgbFloat = true;

	int timeStart, timeStop; // for performance mesurement

	timeStart = clock();
	filestr.open( rFilename.c_str(), fstream::in );

	int   texR, texG, texB;
	float texRf, texGf, texBf;
	float someCoordX, someCoordY, someCoordZ; // for testing sscanf and nothing else.
	int  varsParsed;

	getline( filestr, lineToParse );
	varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %i %i %i", &someCoordX, &someCoordY, &someCoordZ, &texR, &texG, &texB );
	if( varsParsed == 6 ) {
		rgbFloat = false;
		cout << "[MeshIO::readTXT] RGB is int." << endl;
	} else {
		cout << "[MeshIO::readTXT] RGB is float or not existing." << endl;
	}
	filestr.seekg( 0, ios::beg );

	struct Pos{
		Pos(float X, float Y, float Z) : x(X), y(Y), z(Z){ }
		float x;
		float y;
		float z;
	};

	struct Col {
		Col(unsigned char R, unsigned char G, unsigned char B) : r(R), g(G), b(B) {}
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	std::list<Pos> positions;
	std::list<Col> colors;

	while( !filestr.eof() ) {
		if( rgbFloat ) {
			varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %f %f %f", &someCoordX, &someCoordY, &someCoordZ, &texRf, &texGf, &texBf );
			positions.emplace_back(Pos(someCoordX,someCoordY,someCoordZ));
			colors.emplace_back(Col(texRf * 255.0f, texGf * 255.0f, texBf * 255.0f));
		} else {
			varsParsed = sscanf( lineToParse.c_str(), "%f %f %f %i %i %i", &someCoordX, &someCoordY, &someCoordZ, &texR, &texG, &texB );
			positions.emplace_back(Pos(someCoordX,someCoordY,someCoordZ));
			colors.emplace_back(Col(texR, texG, texB));
		}
		if( ( varsParsed == 3 ) || ( varsParsed == 6 ) ) {
			linesVertices++;
		} else {
			cerr << "[MeshIO] Problem in line " << linesRead << " parsed: " << varsParsed << " - " << lineToParse << endl;
		}
		getline( filestr, lineToParse );
		linesRead++;
	}

	timeStop  = clock();
	cout << "[MeshIO] read XYZ/TXT 1st parse:      " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	cout << "[MeshIO] " << linesRead << " lines read from " << rFilename << "." << endl;

	// allocate memory:
	rVertexProps.resize( linesVertices );
	// init vertex coordinates and function value
	for( uint64_t vertexIdx=0; vertexIdx<linesVertices; vertexIdx++ ) {
		// don't init function value with NaN, as this results in performance issues by repeatingly checking min/max values in MeshGL::getFuncValMinMaxUser.
		rVertexProps.at( vertexIdx ).mCoordX  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( vertexIdx ).mCoordY  = _NOT_A_NUMBER_DBL_;
		rVertexProps.at( vertexIdx ).mCoordZ  = _NOT_A_NUMBER_DBL_;
		// init function value
		rVertexProps.at( vertexIdx ).mFuncVal = 0.0;
		// init colors
		rVertexProps.at( vertexIdx ).mColorRed = 187;
		rVertexProps.at( vertexIdx ).mColorGrn = 187;
		rVertexProps.at( vertexIdx ).mColorBle = 187;
		rVertexProps.at( vertexIdx ).mColorAlp = 255;
	}

	// parse to arrays:
	timeStart = clock();

	uint64_t vertexIdx = 0;
	for( const auto& pos : positions ) {
		rVertexProps.at( vertexIdx ).mCoordX  = pos.x;
		rVertexProps.at( vertexIdx ).mCoordY  = pos.y;
		rVertexProps.at( vertexIdx ).mCoordZ  = pos.z;
		++vertexIdx;
	}
	vertexIdx = 0;
	for( const auto& col : colors ) {
		rVertexProps.at( vertexIdx ).mColorRed = col.r;
		rVertexProps.at( vertexIdx ).mColorGrn = col.g;
		rVertexProps.at( vertexIdx ).mColorBle = col.b;
		++vertexIdx;
	}

	timeStop  = clock();
	std::cout << "[MeshIO] read XYZ/TXT 2nd parse:      " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds. " << std::endl;

	std::setlocale( LC_NUMERIC, oldLocale );

	return( true );
}

//! Read XYZ text file with a regular grid, as exported by GIS files.
bool MeshIO::readTXTRegularGrid(
                const string& rFilename,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	// Functionality:
	ifstream fp( rFilename.c_str() );
	if( !fp.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'." << endl;
		return false;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ << "] File opened: '" << rFilename << "'." << endl;
	}

	// Read header line with tags
	vector<int> headerTags;
	char nextByte = fp.peek();
	while( ( nextByte != '\n' ) && ( nextByte != '\r' ) && ( nextByte != -1 ) ) {
		string headerTag;
		fp >> headerTag;
		std::transform( headerTag.begin(), headerTag.end(), headerTag.begin(), ::toupper );
		//cout << "[MeshIO::" << __FUNCTION__ << "] Header tag: " << headerTag << endl;
		if( headerTag == "POINTID" ) {
			cout << "[MeshIO::" << __FUNCTION__ << "] Header tag known, but ignored: " << headerTag << endl;
			headerTags.push_back( TXT_REGULAR_IGNORE );
		} else if( headerTag == "X_COORD" ) {
			headerTags.push_back( TXT_REGULAR_X );
		} else if( headerTag == "Y_COORD" ) {
			headerTags.push_back( TXT_REGULAR_Y );
		} else if( headerTag == "GRID_CODE" ) {
			headerTags.push_back( TXT_REGULAR_Z );
		} else {
			cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Unknown header tag: " << headerTag << "!" << endl;
			headerTags.push_back( TXT_REGULAR_UNKNOWN );
		}
		nextByte = fp.peek();
	}

	// Read the file:
	vector<double> coordX;
	vector<double> coordY;
	vector<double> coordZ;
	vector<int>::iterator itTag;
	while( fp.good() ) {

		bool nextWhiteSpace = true;
		do {
			nextByte = fp.peek();
			if( ( nextByte == ' ' ) || ( nextByte == '\t' ) ) {
				nextByte = fp.get();
			} else {
				nextWhiteSpace = false;
			}
		} while( nextWhiteSpace );

		string strIgnore;
		for( itTag=headerTags.begin(); itTag!=headerTags.end(); itTag++ ) {
			double someCoord;
			//int someInt;
			switch( *itTag ) {
				case TXT_REGULAR_IGNORE:
					fp >> strIgnore;
					cout << "[" << strIgnore << "] ";
					break;
				case TXT_REGULAR_X:
					fp >> someCoord;
					coordX.push_back( someCoord );
					cout << someCoord << " ";
					break;
				case TXT_REGULAR_Y:
					fp >> someCoord;
					coordY.push_back( someCoord );
					cout << someCoord << " ";
					break;
				case TXT_REGULAR_Z:
					fp >> someCoord;
					coordZ.push_back( someCoord );
					cout << someCoord << " ";
					break;
				default: // ignore
					fp >> strIgnore;
					cout << "{" << strIgnore << "} ";
			}
		}
		cout << endl;
		// Read the rest of the line - typically \n
		getline( fp, strIgnore );
		cout << "-" << strIgnore << "-" << endl;
	}

	cout << "[MeshIO::" << __FUNCTION__ << "] X-coordinates: " << coordX.size() << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] Y-coordinates: " << coordY.size() << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] Z-coordinates: " << coordZ.size() << endl;

	fp.close();
	return true;
}

//! Read binary PLY (as exported from Breuckmann OPTOCAT, which we will face
//! quite often ;)
//!
//! But this means this covers only a fraction of the PLY standard and we
//! assume a certain formating (and can ignore parts of the header).
//!
//! see also: http://en.wikipedia.org/wiki/PLY_(file_format)
//!           http://paulbourke.net/dataformats/ply/
//! [DEAD link] full specification: http://local.wasp.uwa.edu.au/~pbourke/dataformats/ply/
//! [DEAD link] alternate specification document: http://www.cs.kuleuven.ac.be/~ares/libply/ply-0.1/doc/PLY_FILES.txt
bool MeshIO::readPLY(
                const string& rFilename,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	fstream filestr;
	string  lineToParse;
	string  lineToParseOri;
	int     linesComment = 0;
	bool    readASCII = false;

	int timeStart = clock(); // for performance mesurement

	cout << "[MeshIO::" << __FUNCTION__ << "] opening: '" << rFilename << "'" << endl;
	filestr.open( rFilename.c_str(), fstream::in );
	if( !filestr.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] could not open: '" << rFilename << "'!" << endl;
		return false;
	}

	bool reverseByteOrder  = false;
	bool endOfHeader       = false; 

	int plyCurrentSection = PLY_SECTION_UNSUPPORTED;
	int plyElements[PLY_SECTIONS_COUNT];

//	for( int i=0; i<PLY_SECTIONS_COUNT; i++ ) {
//		plyElements[i] = 0;
//	}

	for(int& plyElement : plyElements)
	{
		plyElement = 0;
	}

	plyContainer sectionProps[PLY_SECTIONS_COUNT];

	// the first two lines we expect: "ply" -- maybe add a check?
	getline( filestr, lineToParse );
	// than the header begins, which is typically ASCII:
	while( !filestr.eof() && !endOfHeader) {
		getline( filestr, lineToParseOri );
		lineToParse = lineToParseOri;

		// make the line lower-case as comparing is case-sensitive:
//		for( size_t i=0; i<lineToParse.length(); ++i ) {
//			lineToParse[i] = tolower( lineToParse[i] );
//		}

		for(char& character : lineToParse)
		{
			character = std::tolower(character);
		}

		//cout << lineToParse.length() << " '" << lineToParse << "' " << endl;

		// Remove trailing '\r' e.g. from files provided from some low-cost scanners
		if( !lineToParse.empty() && lineToParse[lineToParse.size() - 1] == '\r' ) {
			lineToParse.erase( lineToParse.size() - 1 );
		}

		// if someday someone require trimming of white space it should work
		// like this: std::remove(astring.begin(), astring.end(), ' ');

		// Find END of header
		if( lineToParse == "end_header" ) {
			endOfHeader = true;
			continue;
		}

		// Meta-Data strings stored as comments -------------------------------------------------------
		if( lineToParse.compare( 0, 7, "comment" ) == 0 ) {
			istringstream iss( lineToParseOri );
			vector<string> lineParts{ istream_iterator<string>{iss}, istream_iterator<string>{} };
			if( lineParts.size() < 2 ) {
				continue;
			}
			string possibleMetaDataName = lineParts.at( 1 );
			eMetaStrings foundMetaId;
			if( getModelMetaStringId( possibleMetaDataName, foundMetaId ) ) {
				uint64_t preMetaLen = 9 + possibleMetaDataName.size(); // 7 for 'comment' plus 2x space.
				string metaContent = lineToParseOri.substr( preMetaLen );
				cout << "[MeshIO::" << __FUNCTION__ << "] Meta-Data: " << possibleMetaDataName << " (" << foundMetaId << ") = " << metaContent << std::endl;
				if( !setModelMetaString( foundMetaId, metaContent ) ) {
					cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Meta-Data not set!" << std::endl;
				}
			}
			continue;
		}
		// --------------------------------------------------------------------------------------------

		if( lineToParse.substr( 0, 6 ) == "format" ) {
			if( lineToParse == "format binary_big_endian 1.0" ) {
				cout << "[MeshIO::" << __FUNCTION__ << "] File is BIG Endian." << endl;
				if( not( mSystemIsBigEndian ) ) {
					reverseByteOrder = true;
				}
			} else if( lineToParse == "format binary_little_endian 1.0" ) {
				cout << "[MeshIO::" << __FUNCTION__ << "] File is LITTLE Endian." << endl;
				if( mSystemIsBigEndian ) {
					reverseByteOrder = true;
				}
			} else {
				readASCII = true;
				cerr << "[MeshIO::" << __FUNCTION__ << "] try to read as ASCII." << endl;
				//cerr << "[MeshIO::" << __FUNCTION__ << "] unsupported format (e.g. ASCII)!" << endl;
				cerr << "[MeshIO::" << __FUNCTION__ << "] Line: " << lineToParse << endl;
				//filestr.close();
				//return false;
			}
		}

		//! \todo	We should look into if it's worth it replacing
		//!			sscanf with regex matches, as sscanf with a statically
		//!			allocated array can result in a stack overflow.
		//!			Consequently, the strcmp() calls could be replaced with
		//!			std::string operator()== calls, which are potentially
		//!			faster, as they can terminate on the first character not
		//!			matching the std::string to which the std::string the
		//!			method of which is called

		// parse element line
		if( lineToParse.substr( 0, 7 ) == "element" ) {
			// Vertices
			if( sscanf( lineToParse.c_str(), "element vertex %i", &plyElements[PLY_VERTEX] ) == 1 ) {
				cout << "[MeshIO::" << __FUNCTION__ << "] Vertices: " << plyElements[PLY_VERTEX] << endl;
				plyCurrentSection = PLY_VERTEX;
				// allocate memory - assume color per vertex:
				rVertexProps.resize( plyElements[PLY_VERTEX] );

				// init vertex coordinates and function value
				for( uint64_t vertexIdx=0; vertexIdx<plyElements[PLY_VERTEX]; vertexIdx++ ) {
					rVertexProps.at( vertexIdx ).mCoordX = _NOT_A_NUMBER_DBL_;
					rVertexProps.at( vertexIdx ).mCoordY = _NOT_A_NUMBER_DBL_;
					rVertexProps.at( vertexIdx ).mCoordZ = _NOT_A_NUMBER_DBL_;
					// init function value with zero as NaN results in performance issues by repeatingly checking min/max values in MeshGL::getFuncValMinMaxUser.
					rVertexProps.at( vertexIdx ).mFuncVal = 0.0;
					// init colors
					rVertexProps.at( vertexIdx ).mColorRed = 187;
					rVertexProps.at( vertexIdx ).mColorGrn = 187;
					rVertexProps.at( vertexIdx ).mColorBle = 187;
					rVertexProps.at( vertexIdx ).mColorAlp = 255;
				}
			// Faces
			} else if( sscanf( lineToParse.c_str(), "element face %i", &plyElements[PLY_FACE] ) == 1 ) {
				cout << "[MeshIO::" << __FUNCTION__ << "] Faces: " << plyElements[PLY_FACE] << endl;
				plyCurrentSection = PLY_FACE;
				// allocate memory:
				rFaceProps.resize( plyElements[PLY_FACE] );
			} else if( sscanf( lineToParse.c_str(), "element line %i", &plyElements[PLY_POLYGONAL_LINE] ) == 1 ) {
				cout << "[MeshIO::" << __FUNCTION__ << "] Polygonal lines: " << plyElements[PLY_POLYGONAL_LINE] << endl;
				plyCurrentSection = PLY_POLYGONAL_LINE;
			// Unsupported
			} else {
				char elementName[255];
				int  elementCount;
				sscanf( lineToParse.c_str(), "element %s %i", elementName, &elementCount );
				plyElements[PLY_SECTION_UNSUPPORTED] += elementCount;
				plyCurrentSection = PLY_SECTION_UNSUPPORTED;
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: unknown element '" << elementName << "', count: " << elementCount << "!" << endl;
			}
		}

		// parse property line
		bool propertyParsed = false;
		ePlyProperties  propertyType   = PLY_UNSUPPORTED;
		int  propertyDataType          = PLY_SIZE_UNDEF;
		int  propertyListCountDataType = PLY_SIZE_UNDEF;
		int  propertyListDataType      = PLY_SIZE_UNDEF;
		if( lineToParse.substr( 0, 13 ) == "property list" ) {
			// e.g: property list uint8 float vertex_features
			propertyParsed = true;
			char propType[255];
			char propName[255];
			char propTypeCount[255];
			if( sscanf( lineToParse.c_str(), "property list %s %s %s", propTypeCount, propType, propName ) != 3 ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] Error while parsing list properties!" << endl;
				propertyParsed = false;
			}
			propertyListCountDataType = plyParseTypeStr( propTypeCount );
			propertyListDataType      = plyParseTypeStr( propType );
			if( strcmp( propName, "feature_vector" ) == 0 ) {
				propertyType = PLY_LIST_FEATURE_VECTOR;
			} else if( strcmp( propName, "vertex_indices" ) == 0 || // ... more commonly used keyword
			           strcmp( propName, "vertex_index" ) == 0 ) {  // ... used by some low-cost devices
				propertyType = PLY_LIST_VERTEX_INDICES;
			} else {
				propertyType = PLY_LIST_IGNORE;
			}
			if( ( propertyListDataType == PLY_SIZE_UNDEF ) || ( propertyListCountDataType == PLY_SIZE_UNDEF ) ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] Line: " << lineToParse << endl;
			}
		} else if( lineToParse.substr( 0, 8 ) == "property" ) {
			propertyParsed = true;
			char propType[255];
			char propName[255];
			if( sscanf( lineToParse.c_str(), "property %s %s", propType, propName ) != 2 ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] Error while parsing single properties!" << endl;
				propertyParsed = false;
			}
			if( strcmp( propName, "x" ) == 0 ) {
				propertyType = PLY_COORD_X;
			} else if( strcmp( propName, "y" ) == 0 ) {
				propertyType = PLY_COORD_Y;
			} else if( strcmp( propName, "z" ) == 0 ) {
				propertyType = PLY_COORD_Z;
			} else if( strcmp( propName, "nx" ) == 0 ) {
				propertyType = PLY_VERTEX_NORMAL_X;
			} else if( strcmp( propName, "ny" ) == 0 ) {
				propertyType = PLY_VERTEX_NORMAL_Y;
			} else if( strcmp( propName, "nz" ) == 0 ) {
				propertyType = PLY_VERTEX_NORMAL_Z;
			} else if( strcmp( propName, "labelid" ) == 0 ) {
				propertyType = PLY_LABEL;
			} else if( strcmp( propName, "flags" ) == 0 ) {
				propertyType = PLY_FLAGS;
			} else if( strcmp( propName, "quality" ) == 0 ) {
				propertyType = PLY_VERTEX_QUALITY;
			} else if( strcmp( propName, "red" ) == 0 ) {
				propertyType = PLY_COLOR_RED;
			} else if( strcmp( propName, "green" ) == 0 ) {
				propertyType = PLY_COLOR_GREEN;
			} else if( strcmp( propName, "blue" ) == 0 ) {
				propertyType = PLY_COLOR_BLUE;
			} else if( strcmp( propName, "alpha" ) == 0 ) {
				propertyType = PLY_COLOR_ALPHA;
			// found in PLYs from stanford:
			} else if( strcmp( propName, "diffuse_red" ) == 0 ) {
				propertyType = PLY_COLOR_RED;
			} else if( strcmp( propName, "diffuse_green" ) == 0 ) {
				propertyType = PLY_COLOR_GREEN;
			} else if( strcmp( propName, "diffuse_blue" ) == 0 ) {
				propertyType = PLY_COLOR_BLUE;
			} else if( strcmp( propName, "vertex_index" ) == 0 ) {
				propertyType = PLY_VERTEX_INDEX;
			} else if( strcmp( propName, "label" ) == 0 ) {
				propertyType = PLY_LABEL;
			} else {
				cerr << "[MeshIO::" << __FUNCTION__ << "] unknown property '" << propName << "' in header!" << endl;
				cerr << "[MeshIO::" << __FUNCTION__ << "] Line: " << lineToParse << endl;
				propertyType = PLY_UNSUPPORTED;
			}
			propertyDataType = plyParseTypeStr( propType );
			if( propertyDataType == PLY_SIZE_UNDEF ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] Line: " << lineToParse << endl;
			}
		}

		// when no property was parsed we continue
		if( !propertyParsed ) {
			continue;
		}
		switch( plyCurrentSection ) {
			//! \todo Is this fallthrough intentional?
			case PLY_SECTION_UNSUPPORTED:
				cerr << "[MeshIO::" << __FUNCTION__ << "] Property of an unsupported type: " << lineToParse << endl;
				break;
			case PLY_POLYGONAL_LINE:
			case PLY_VERTEX:
			case PLY_FACE:
				sectionProps[plyCurrentSection].propertyType.push_back( propertyType );
				sectionProps[plyCurrentSection].propertyDataType.push_back( propertyDataType );
				sectionProps[plyCurrentSection].propertyListCountDataType.push_back( propertyListCountDataType );
				sectionProps[plyCurrentSection].propertyListDataType.push_back( propertyListDataType );
				break;
			default:
				cerr << "[MeshIO::" << __FUNCTION__ << "] Error undefined section type: " << plyCurrentSection << "!" << endl;
		}
	}

	if( !endOfHeader ) {
		// no end of the header found means some trouble:
		cerr << "[MeshIO::" << __FUNCTION__ << "] failed: no end of header was found!" << endl;
		filestr.close();
		return( false );
	}

	unsigned char listNrChar;
	uint64_t verticesRead      = 0;
	int   facesRead         = 0;
	int   polyLinesRead     = 0;
	int   unsupportedRead   = 0;
	char  charProp;
	long  bytesIgnored = 0;
	long  extraBytesIgnored = 0;
	long  posInFile;
	float someFloat;
	int   someInt;

	vector<int>::iterator plyProp;
	vector<int>::iterator plyPropSize;
	vector<int>::iterator plyPropListCountSize;
	vector<int>::iterator plyPropListSize;

	if( readASCII ) {
		//! \todo still incomplete: flags and feature vectors not tested and not supported.
		char* oldLocale = std::setlocale( LC_NUMERIC, nullptr );
		std::setlocale( LC_ALL, "C" );
		for( verticesRead=0; verticesRead<plyElements[PLY_VERTEX]; verticesRead++ ) {
			getline( filestr, lineToParse );
			// Remove trailing '\r' e.g. from files provided from some low-cost scanners
			if( !lineToParse.empty() && lineToParse[lineToParse.size() - 1] == '\r' ) {
				lineToParse.erase( lineToParse.size() - 1 );
			}
			// Break line into tokens:
			istringstream iss( lineToParse );
			vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
			if( tokens.size() == 0 ) {
				continue; // Empty line
			}
			// Parse each token of a line:
			for( uint64_t i=0; i<tokens.size(); i++ ) {
				if( i>=sectionProps[PLY_VERTEX].propertyType.size() ) {
					// Sanity check
					cout << "[MeshIO::" << __FUNCTION__ << "] verticesRead: " << verticesRead << " of " << plyElements[PLY_VERTEX] << endl;
					cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: More tokens than properties " << i << " >= " << sectionProps[PLY_VERTEX].propertyType.size() << "!" << endl;
					continue;
				}
				string lineElement = tokens.at( i );
				ePlyProperties currProperty = sectionProps[PLY_VERTEX].propertyType.at( i );
				switch( currProperty ) {
					case PLY_COORD_X:
						rVertexProps.at( verticesRead ).mCoordX = atof( lineElement.c_str() );
						break;
					case PLY_COORD_Y:
						rVertexProps.at( verticesRead ).mCoordY = atof( lineElement.c_str() );
						break;
					case PLY_COORD_Z:
						rVertexProps.at( verticesRead ).mCoordZ = atof( lineElement.c_str() );
						break;
					case PLY_VERTEX_NORMAL_X:
						//! \todo implement parsing of normals in ASCII PLY's.
						break;
					case PLY_VERTEX_NORMAL_Y:
						//! \todo implement parsing of normals in ASCII PLY's.
						break;
					case PLY_VERTEX_NORMAL_Z:
						//! \todo implement parsing of normals in ASCII PLY's.
						break;
					case PLY_FLAGS:
						rVertexProps.at( verticesRead ).mFlags = static_cast<unsigned long>(atoi( lineElement.c_str() ));
						break;
					case PLY_LABEL:
						rVertexProps.at( verticesRead ).mLabelId = static_cast<unsigned long>(atoi( lineElement.c_str() ));
						break;
					case PLY_VERTEX_QUALITY:
						rVertexProps.at( verticesRead ).mFuncVal = atof( lineElement.c_str() );
						break;
					case PLY_VERTEX_INDEX:
						break;
					case PLY_COLOR_RED:
						rVertexProps.at( verticesRead ).mColorRed = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_GREEN:
						rVertexProps.at( verticesRead ).mColorGrn = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_BLUE:
						rVertexProps.at( verticesRead ).mColorBle = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_ALPHA:
						//! \todo alpha is read but ignored later on.
						rVertexProps.at( verticesRead ).mColorAlp = atoi( lineElement.c_str() );
						break;
					case PLY_LIST_IGNORE:
					case PLY_LIST_VERTEX_INDICES:
						listNrChar = atoi( lineElement.c_str() );
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							//! \todo implement PLY_LIST_VERTEX_INDICES for ASCII PLYs
							// ...
						}
						break;
					case PLY_LIST_FEATURE_VECTOR:
						listNrChar = atoi( lineElement.c_str() );
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							//! \todo implement PLY_LIST_FEATURE_VECTOR for ASCII PLYs
							// someFloat = atof( lineElement.c_str() );
							// featureVecVertices[verticesRead*featureVecVerticesLen+j] = (double)someFloat;
						}
						//cout << endl;
						break;
					case PLY_UNSUPPORTED:
					default:
						// Read but ignore
						cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Unknown property in vertex section " << currProperty << " !" << endl;
				}
			}
		}
		cout << "[MeshIO::" << __FUNCTION__ << "] Reading vertices done." << endl;
		// Read faces
		for( facesRead=0; facesRead<plyElements[PLY_FACE]; facesRead++ ) {
			getline( filestr, lineToParse );
			string strLine( lineToParse );
			// Remove trailing '\r' e.g. from files provided from some low-cost scanners
			if( !lineToParse.empty() && lineToParse[lineToParse.size() - 1] == '\r' ) {
				lineToParse.erase( lineToParse.size() - 1 );
			}
			// Break line into tokens:
			istringstream iss( lineToParse );
			vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
			if( tokens.size() == 0 ) {
				continue; // Empty line
			}
			// Parse each token of a line:
			for( uint64_t i=0; i<tokens.size(); i++ ) {
				if( i>=sectionProps[PLY_FACE].propertyType.size() ) {
					// Sanity check
					cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: More tokens than properties!" << endl;
					continue;
				}
				string lineElement = tokens.at( i );
				ePlyProperties currProperty = sectionProps[PLY_FACE].propertyType.at( i );
				switch( currProperty ) {
					case PLY_LIST_VERTEX_INDICES: {
						    uint64_t elementCount = atoi( lineElement.c_str() );
							if( elementCount != 3 ) {
								i += elementCount; // These have to be skipped by the outer loop.
								cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: unsupported number (" << listNrChar << ") of elements within the list!" << endl;
								continue;
							}
							string listElementInLine = tokens.at( i+1 );
							uint64_t vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxA = static_cast<int>(vertexIndexNr);
							listElementInLine = tokens.at( i+2 );
							vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxB = static_cast<int>(vertexIndexNr);
							listElementInLine = tokens.at( i+3 );
							vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxC = static_cast<int>(vertexIndexNr);
							i += elementCount; // These have to be skipped by the outer loop.
						} break;
					default:
						// Read but ignore
						cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Unknown property in face section " << currProperty << " !" << endl;
				}
			}
		}
		cout << "[MeshIO::" << __FUNCTION__ << "] Reading faces done." << endl;
		//! \todo ASCII: add support for selected vertices.
		//! \todo ASCII: add support for polylines.
		//! \todo ASCII: add support for unsupported lines.
		filestr.close();
		std::setlocale( LC_ALL, oldLocale );
		cout << "[MeshIO::" << __FUNCTION__ << "] LC_NUMERIC: " << (*oldLocale) << endl;
		return( true );
	}

	// ReOpen file, when binary!
	if( !readASCII ) {
		filestr.close();

		filestr.open( rFilename.c_str(), fstream::in | ios_base::binary );
		if( !filestr.is_open() ) {
			cerr << "[MeshIO::" << __FUNCTION__ << "] could not open: '" << rFilename << "'!" << endl;
			return false;
		}
		filestr.seekg( 0, filestr.end );
		uint64_t fileLength = filestr.tellg();
		filestr.seekg( 0, filestr.beg );
		cout << "[MeshIO::" << __FUNCTION__ << "] File length for reading binary: " << fileLength << " Bytes." << endl;

		//! \todo buffer can not allocate sufficent memory for very large 3D-models. Anyway it makes no sense to load the whole file jus to search for the end of the header. The latter is relativly small.
		std::vector<char> buffer;
		try{
			buffer.resize( fileLength );
		} catch( const std::exception& e ) {
			cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: A standard exception was caught, with message '"
			     << e.what() << "'" << endl;
			cerr << "[MeshIO::" << __FUNCTION__ << "]        File length: " << fileLength << endl;
			cerr << "[MeshIO::" << __FUNCTION__ << "]        Size of chars for the buffer: " << fileLength*sizeof( char ) << endl;
			cerr << "[MeshIO::" << __FUNCTION__ << "]        Maximum buffer size (theoretical, but not guaranteed): " << buffer.max_size() << endl;
			cerr << "[MeshIO::" << __FUNCTION__ << "] ...... Trying again!" << endl;

			try{
				// try to truncate to 10 MB
				fileLength = 10*1024*1024;
				buffer.resize( fileLength );
			} catch( const std::exception& e ) {
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: A standard exception was caught, with message '"
				     << e.what() << "'" << endl;
				// Close file and return
				filestr.close();
				return( false );
			}
		}

		filestr.read( buffer.data(), fileLength );
		// Search string: 'end_header'
		std::vector<char> seq = { 'e', 'n', 'd', '_', 'h', 'e', 'a', 'd', 'e', 'r' };
		std::vector<char>::iterator it;
		it = std::search( buffer.begin(), buffer.end(), seq.begin(), seq.end() );
		if( it == buffer.end() ) {
			cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: 'end_header' not found!" << endl;
			// Close file and return
			filestr.close();
			return( false );
		}
		int posfound = ( it - buffer.begin() );
		cout << "[MeshIO::" << __FUNCTION__ << "] 'end_header' found at position " << posfound << endl;

		char newLine = buffer.at( posfound+seq.size() );
		int forwardToPos;
		if( newLine == 0x0A ) {
			forwardToPos = posfound+seq.size()+1;
			cout << "[MeshIO::" << __FUNCTION__ << "] one byte line break." << endl;
		} else {
			newLine = buffer.at( posfound+seq.size()+1 );
			if( newLine == 0x0A ) {
				forwardToPos = posfound+seq.size()+2;
				cout << "[MeshIO::" << __FUNCTION__ << "] two byte line break." << endl;
			} else {
				cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: end of header new line not found!" << endl;
				return false;
			}
		}
		filestr.seekg( forwardToPos );
	}

	while( filestr ) {
	//while( !filestr.eof() ) {
		if( verticesRead < plyElements[PLY_VERTEX] ) {
			plyPropSize          = sectionProps[PLY_VERTEX].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_VERTEX].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_VERTEX].propertyListDataType.begin();
			double vertNormalX = _NOT_A_NUMBER_DBL_;
			double vertNormalY = _NOT_A_NUMBER_DBL_;
			double vertNormalZ = _NOT_A_NUMBER_DBL_;
			//int pos = filestr.tellg();
			//cout << ">>> Pos: " << pos << " len: " << fileLength << endl;
			for(auto currProperty : sectionProps[PLY_VERTEX].propertyType) {
				    switch( currProperty ) {
					case PLY_COORD_X:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mCoordX = static_cast<double>(someFloat);
						//filestr.read( reinterpret_cast<char*>(&vertexOriCoords[verticesRead*3]), (*plyPropSize) );
						//cout << "Read X: " << vertexOriCoords[verticesRead*3] << " (Size: " << (*plyPropSize) << ")" << endl;
						if( filestr.tellg() < 0 ) {
							cerr << "ERROR" << endl;
						}
						break;
					case PLY_COORD_Y:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mCoordY = static_cast<double>(someFloat);
						//filestr.read( reinterpret_cast<char*>(&vertexOriCoords[verticesRead*3+1]), (*plyPropSize) );
						if( filestr.tellg() < 0 ) {
							cerr << "ERROR" << endl;
						}
						break;
					case PLY_COORD_Z:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mCoordZ = static_cast<double>(someFloat);
						//filestr.read( reinterpret_cast<char*>(&vertexOriCoords[verticesRead*3+2]), (*plyPropSize) );
						break;
					case PLY_VERTEX_NORMAL_X:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						vertNormalX = static_cast<double>(someFloat);
						break;
					case PLY_VERTEX_NORMAL_Y:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						vertNormalY = static_cast<double>(someFloat);
						break;
					case PLY_VERTEX_NORMAL_Z:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						vertNormalZ = static_cast<double>(someFloat);
						break;
					case PLY_FLAGS: {
						unsigned int someFlags;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFlags, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mFlags = someFlags;
						} break;
					case PLY_LABEL: {
						unsigned int someLabelID;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someLabelID, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mLabelId = someLabelID;
						} break;
					case PLY_VERTEX_QUALITY:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps.at( verticesRead ).mFuncVal = static_cast<double>(someFloat);
						//filestr.read( reinterpret_cast<char*>(&vertexOriCoords[verticesRead*3+2]), (*plyPropSize) );
						break;
					case PLY_COLOR_RED:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						//filestr.read( &charProp, (*plyPropSize) );
						rVertexProps.at( verticesRead ).mColorRed = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_GREEN:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						//filestr.read( &charProp, (*plyPropSize) );
						rVertexProps.at( verticesRead ).mColorGrn = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_BLUE:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						//filestr.read( &charProp, (*plyPropSize) );
						rVertexProps.at( verticesRead ).mColorBle = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_ALPHA:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						//filestr.read( &charProp, (*plyPropSize) );
						//! \todo Alpha is read but ignored later on.
						rVertexProps.at( verticesRead ).mColorAlp = static_cast<unsigned char>(charProp);
						break;
					case PLY_LIST_IGNORE:
					case PLY_LIST_VERTEX_INDICES:
						//cout << "Ignore: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//cout << "Ignore: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize)) );
						bytesIgnored += static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize));
						break;
					case PLY_LIST_FEATURE_VECTOR:
						//cout << "Read: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						//cout << "(*plyPropListCountSize) " << (*plyPropListCountSize) << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//cout << "listNrChar: " << (unsigned short)listNrChar << endl;
						if( filestr.tellg() < 0 ) {
							cerr << "ERROR" << endl;
						}
						//cout << "Read: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						// When we have no memory allocated yet - this is the time (and we assume that all vertices have feature vectors of the same length or shorter.
						if( mFeatureVecVertices.size() == 0 ) {
							mFeatureVecVerticesLen = static_cast<uint64_t>(listNrChar);
							mFeatureVecVertices.assign( mFeatureVecVerticesLen*plyElements[PLY_VERTEX], _NOT_A_NUMBER_DBL_ );
						}
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropListSize), reverseByteOrder );
							if( filestr.tellg() < 0 ) {
								cerr << "ERROR" << endl;
							}
							//cout << someFloat << " ";
							mFeatureVecVertices.at( verticesRead*mFeatureVecVerticesLen+j ) = static_cast<double>(someFloat);
						}
						//cout << endl;
						break;
					default:
						// Read but ignore
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+(*plyPropSize) );
						bytesIgnored += (*plyPropSize);
				}
				plyPropSize++;
				plyPropListCountSize++;
				plyPropListSize++;
				//int pos = filestr.tellg();
				//cout << "Pos: " << pos << " len: " << fileLength << endl;
			}
			// addNormal( vertNormalX, vertNormalY, vertNormalZ );
			// addVertNormalIdx( verticesRead, verticesRead );
			verticesRead++;
			//cout << "<<<<<<<<<<<<<<<<<<<<" << endl;
		} else if( facesRead < plyElements[PLY_FACE] ) {
			plyPropSize          = sectionProps[PLY_FACE].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_FACE].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_FACE].propertyListDataType.begin();
			for(const MeshIO::ePlyProperties currProperty : sectionProps[PLY_FACE].propertyType) {
				    switch( currProperty ) {
					case PLY_LIST_VERTEX_INDICES:
						//cout << "Read: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						if( listNrChar != 3 ) {
							cerr << "[MeshIO::" << __FUNCTION__ << "] unsupported number (" << static_cast<uint>(listNrChar) << ") of elements within the list!" << endl;
						}
						//cout << "Read: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						//cout << someInt << " ";
						rFaceProps[facesRead].mVertIdxA = someInt;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						//cout << someInt << " ";
						rFaceProps[facesRead].mVertIdxB = someInt;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						//cout << someInt << " ";
						rFaceProps[facesRead].mVertIdxC = someInt;
						//cout << endl;
						break;
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_IGNORE:
						//cout << "Ignore: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//cout << "Ignore: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize)) );
						bytesIgnored += static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize));
						break;
					default:
						// Read but ignore
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+(*plyPropSize) );
						bytesIgnored += (*plyPropSize);
				}
				plyPropSize++;
				plyPropListCountSize++;
				plyPropListSize++;
			}
/*
			READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, 1, reverseByteOrder );
			listNr = (int)(unsigned char)listNrChar;
			if( listNr != 3 ) {
				cerr << __PRETTY_FUNCTION__ << " unsupported number (" << listNr << ") of elements within the list!" << endl;
				// ... means no triangles (e.g. quadtriangles) or something else
				// we don't expect.
				//cerr << "[MeshIO] read (binary) PLY failed: something not a triangle was found! Value is: '" << listNr << "' should be: 3. facesRead = " << facesRead << endl;
				//filestr.close();
				return false;
			}
			READ_IN_PROPER_BYTE_ORDER( filestr, &facesMeshed[facesRead*3],   4, reverseByteOrder );  // integer have 4 bytes in a binary PLY
			READ_IN_PROPER_BYTE_ORDER( filestr, &facesMeshed[facesRead*3+1], 4, reverseByteOrder );
			READ_IN_PROPER_BYTE_ORDER( filestr, &facesMeshed[facesRead*3+2], 4, reverseByteOrder );
*/
			//filestr.read( reinterpret_cast<char*>(&facesMeshed[facesRead*3]), 4 ); // integer have 4 bytes in a binary PLY
			//filestr.read( reinterpret_cast<char*>(&facesMeshed[facesRead*3+1]), 4 );
			//filestr.read( reinterpret_cast<char*>(&facesMeshed[facesRead*3+2]), 4 );
			//cout << "[MeshIO] Face[" << facesRead << "/" << (int)listNr << "] " << facesMeshed[facesRead*3] << " - " << facesMeshed[facesRead*3+1] << " - " << facesMeshed[facesRead*3+2] << endl;
			facesRead++;
		} else if( polyLinesRead < plyElements[PLY_POLYGONAL_LINE] ) {
			plyPropSize          = sectionProps[PLY_POLYGONAL_LINE].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_POLYGONAL_LINE].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_POLYGONAL_LINE].propertyListDataType.begin();
			PrimitiveInfo primInfo;
			for(const MeshIO::ePlyProperties currProperty : sectionProps[PLY_POLYGONAL_LINE].propertyType) {
				    switch( currProperty ) {
					case PLY_COORD_X:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mPosX = static_cast<double>(someFloat);
						break;
					case PLY_COORD_Y:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mPosY = static_cast<double>(someFloat);
						break;
					case PLY_COORD_Z:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mPosZ = static_cast<double>(someFloat);
						break;
					case PLY_VERTEX_NORMAL_X:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mNormalX = static_cast<double>(someFloat);
						break;
					case PLY_VERTEX_NORMAL_Y:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mNormalY = static_cast<double>(someFloat);
						break;
					case PLY_VERTEX_NORMAL_Z:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						primInfo.mNormalZ = static_cast<double>(someFloat);
						break;
					case PLY_LABEL: {
						unsigned int labelID;
						READ_IN_PROPER_BYTE_ORDER( filestr, &labelID, (*plyPropSize), reverseByteOrder );
						mPolyLabelID.push_back( labelID );
						} break;
					case PLY_LIST_VERTEX_INDICES: {
						// Read a list of polyline indices
						int nrIndices;
						READ_IN_PROPER_BYTE_ORDER( filestr, &nrIndices, (*plyPropListCountSize), reverseByteOrder );
						vector<int>* somePolyLinesIndices = new vector<int>; 
						for( int j=0; j<nrIndices; j++ ) {
							int vertIndex;
							READ_IN_PROPER_BYTE_ORDER( filestr, &vertIndex, (*plyPropListSize), reverseByteOrder );
							somePolyLinesIndices->push_back( vertIndex );
						}
						mPolyLineVertIndices.push_back( somePolyLinesIndices );
						} break;
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_IGNORE:
						//cout << "Ignore: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//cout << "Ignore: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize)) );
						bytesIgnored += static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize));
						break;
					default:
						// Read but ignore
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+(*plyPropSize) );
						bytesIgnored += (*plyPropSize);
				}
				plyPropSize++;
				plyPropListCountSize++;
				plyPropListSize++;
			}
			mPolyPrimInfo.push_back( primInfo );
			polyLinesRead++;
		} else if( unsupportedRead < plyElements[PLY_SECTION_UNSUPPORTED] ) {
			plyPropSize          = sectionProps[PLY_SECTION_UNSUPPORTED].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_SECTION_UNSUPPORTED].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_SECTION_UNSUPPORTED].propertyListDataType.begin();
			for(const MeshIO::ePlyProperties currProperty : sectionProps[PLY_SECTION_UNSUPPORTED].propertyType) {
				    switch( currProperty ) {
					case PLY_LIST_IGNORE:
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_VERTEX_INDICES:
						//cout << "Ignore: " << (*plyPropListCountSize) << " Byte - " << (*plyPropListSize) << " Byte." << endl;
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//cout << "Ignore: " << (uint)listNrChar << " x " << (uint)(*plyPropListSize) << " Byte." << endl;
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize)) );
						bytesIgnored += static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize));
						break;
					default:
						// Read but ignore
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+(*plyPropSize) );
						bytesIgnored += (*plyPropSize);
				}
				plyPropSize++;
				plyPropListCountSize++;
				plyPropListSize++;
			}
			unsupportedRead++;
		} else {
			// needed to reach EOF!:
 			bytesIgnored++;
			extraBytesIgnored++;
 			READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, 1, reverseByteOrder );
		}
	}

	filestr.close();

	cout << "[MeshIO::" << __FUNCTION__ << "] PLY comment lines: " << linesComment << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] PLY ignored:       " << bytesIgnored << " Byte. (one is OK as it is the EOF byte) " << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] PLY Extra ignored: " << extraBytesIgnored << " Byte. (one is OK as it is the EOF byte) " << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] PLY:               " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	
	return( true );
}

// System checks ------------------------------------------------------------------

//! Test if the system (locale) uses a dot or a colon for string representation floating point numbers.
//! @returns false in case of an error i.e. if neither dot or colon are used.
bool MeshIO::systemTestFloatDotColon( bool* rDotUsed ) {
	double testForDot   = stod( "1.5" );
	double testForColon = stod( "1,5" );
	if( testForDot == 1.5 ) {
		cout << "[MeshIO::" << __FUNCTION__ << "] LOCALE: floats are parsed with DOT." << endl;
		(*rDotUsed) = true;
		return true;
	} else if( testForColon == 1.5 ) {
		cout << "[MeshIO::" << __FUNCTION__ << "] LOCALE: floats are parsed with COLON." << endl;
		(*rDotUsed) = false;
		return true;
	}
	cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: parsing does not use DOT nor COLON!" << endl;
	return false;
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
//	for( size_t i=0; i<fileExtension.length(); ++i ) {
//		fileExtension[i] = tolower( fileExtension[i] );
//	}

	for(char& character : fileExtension)
	{
		character = std::tolower(character);
	}

	// Store Export-flags as some are modified due to presence/absence of elements
	bool exportFlagsSave[EXPORT_FLAG_COUNT];
	for( unsigned int i=0; i<EXPORT_FLAG_COUNT; i++ ) {
		exportFlagsSave[i] = mExportFlags[i];
	}

	cout << "[MeshIO::" << __FUNCTION__ << "] extension: " << fileExtension << endl;
	if( mExportFlags[EXPORT_BINARY] ) {
		cout << "[MeshIO::" << __FUNCTION__ << "] BINARY." << endl;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ << "] ASCII." << endl;
	}
	bool fileWriteOk = false;
	if( fileExtension == "obj" ) {
		fileWriteOk = writeOBJ( rFileName, rVertexProps, rFaceProps );
	}
	if( fileExtension == "wrl" ) {
		fileWriteOk = writeVRML( rFileName, rVertexProps, rFaceProps );
	}
	if( fileExtension == "txt" ) {
		fileWriteOk = writeTXT( rFileName, rVertexProps, rFaceProps );
	}
	if( fileExtension == "xyz" ) {
		fileWriteOk = writeTXT( rFileName, rVertexProps, rFaceProps );
	}
	if( fileExtension == "ply" ) {
		fileWriteOk = writePLY( rFileName, rVertexProps, rFaceProps );
	}

	// Restore flags.
	for( unsigned int i=0; i<EXPORT_FLAG_COUNT; i++ ) {
		mExportFlags[i] = exportFlagsSave[i];
	}

	if( fileWriteOk ) {
		mFileNameFull = rFileName;
		return true;
	}
	cerr << "[MeshIO::" << __FUNCTION__ << "] Unknown extension/type '" << fileExtension << "' specified!" << endl;

	return false;
}

//! Writes a Mesh as Polygon File Format or the Stanford Triangle Format
//! from a given MeshSeed.
//! see also: http://en.wikipedia.org/wiki/PLY_(file_format)
//! and:      http://paulbourke.net/dataformats/ply/
//! \todo add texture per face  - only the texture per vertex is exported
bool MeshIO::writePLY(
                const string& rFilename,
                const std::vector<sVertexProperties>& rVertexProps,
                const std::vector<sFaceProperties>& rFaceProps
) {
	//! Supports:
	fstream filestr;

	// for performance mesurement
	using namespace std::chrono;
	high_resolution_clock::time_point tStart, tEnd;

	// Measure total time:
	tStart = high_resolution_clock::now();
	cout << "[MeshIO::" << __FUNCTION__ << "] ------------------------------------------------------------" << endl;

	filestr.open( rFilename.c_str(), fstream::out | fstream::binary);
	if( !filestr.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ <<"] ERROR: Could not open file: '" << rFilename << "'!" << endl;
		return false;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ <<"] File open for writing: '" << rFilename << "'." << endl;
	}

	// if we got a vertex texture_
	//! \todo Make these hard-coded flags available thru the UI.
	//! \todo Detect if color per vertex is present.
	// old=dead: if( vertexTexture == nullptr ) {
	mExportFlags[EXPORT_VERT_COLOR] = true;
	if( getFeatureVecLenMax( Primitive::IS_VERTEX ) <= 0 ) {
		mExportFlags[EXPORT_VERT_FTVEC]  = false;
	}

	// Header:
	filestr << "ply" << endl;
	if( mExportFlags[EXPORT_BINARY] ) {
		if( mSystemIsBigEndian ) {
			filestr << "format binary_big_endian 1.0" << endl;	
		} else {
			filestr << "format binary_little_endian 1.0" << endl;
		}
	} else {
		filestr << "format ascii 1.0" << endl;	
	}
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	filestr << "comment | PLY file generated by GigaMesh Software Framework                             |" << endl;
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	filestr << "comment | WebSite: https://gigamesh.eu                                                  |" << endl;
	filestr << "comment | EMail:   info@gigamesh.eu                                                     |" << endl;
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	filestr << "comment | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << endl;
	filestr << "comment |          IWR - Heidelberg University, Germany                                 |" << endl;
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	filestr << "comment | GigaMesh compiled                                                             |" << endl;
#ifdef COMP_USER
	filestr << "comment | .... by: " << COMP_USER << endl;
#else
	filestr << "comment | .... by: UNKNOWN                                                              |" << endl;
#endif
#ifdef COMP_DATE
	filestr << "comment | .... at: " << COMP_DATE << endl;
#else
	filestr << "comment | .... at: UNKNOWN                                                              |" << endl;
#endif
#ifdef COMP_EDIT
	filestr << "comment | ... for: " << COMP_EDIT << endl;
#else
	filestr << "comment | ... for: UNKNOWN                                                              |" << endl;
#endif
#ifdef COMP_GITHEAD
	filestr << "comment | ... git-head: " << COMP_GITHEAD << endl;
#else
	filestr << "comment | ... git-head: UNKNOWN                                                         |" << endl;
#endif
#ifdef VERSION_PACKAGE
	filestr << "comment | .... Version: " << VERSION_PACKAGE << endl;
#else
	filestr << "comment | .... Version: UNKNOWN                                                              |" << endl;
#endif
	if( !mExportFlags[EXPORT_BINARY] ) {
		filestr << "comment +-------------------------------------------------------------------------------+" << endl;
		filestr << "comment | ATTENTION: floating point values may be expected to contain either '.' or ',' |" << endl;
		filestr << "comment |            depending on the locale settings.                                  |" << endl;
	}
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	filestr << "comment | Meta information:                                                             |" << endl;
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;
	for( uint64_t i=0; i<META_STRINGS_COUNT; i++ ) {
		eMetaStrings metaId = static_cast<eMetaStrings>( i );
		if( metaId == META_FILENAME ) { // Ignore the filename!
			continue;
		}
		string metaStr = getModelMetaString( metaId );
		if( metaStr.size() == 0 ) { // Ignore empty strings!
			continue;
		}
		string metaName;
		if( getModelMetaStringName( metaId, metaName ) ) {
			filestr << "comment " << metaName << " " << metaStr << endl;
		}
	}
	filestr << "comment +-------------------------------------------------------------------------------+" << endl;

	filestr << "element vertex " << rVertexProps.size() << endl;
	//! .) Vertices: x, y and z coordinates.
	filestr << "property float x" << endl;
	filestr << "property float y" << endl;
	filestr << "property float z" << endl;
	//! .) Vertex quality will be set to the function value.
	filestr << "property float quality" << endl;
	//! .) Vertex flags will be exported.
	if( mExportFlags[EXPORT_VERT_FLAGS] ) {
		filestr << "property int flags" << endl;
	}
	//! .) Texture: Color per vertex, when present.
	if( mExportFlags[EXPORT_VERT_COLOR] ) {
		filestr << "property uint8 red"   << endl;
		filestr << "property uint8 green" << endl;
		filestr << "property uint8 blue"  << endl;
		//! \todo Alpha is ignored and not written to output.
	}
	//! .) Normal per vertex
	if( mExportFlags[EXPORT_VERT_NORMAL] ) {
		filestr << "property float nx" << endl;
		filestr << "property float ny" << endl;
		filestr << "property float nz" << endl;
	}
	//! .) Label ID per vertex
	if( mExportFlags[EXPORT_VERT_LABEL] ) {
		filestr << "property uint32 labelid" << endl;
	}
	//! .) Proprietary support for feature vectors, when present.
	if( mExportFlags[EXPORT_VERT_FTVEC] ) {
		filestr << "property list uint8 float feature_vector" << endl;
	}

	//! .) Faces referencing to Vertices - optional, because they are not required by point clouds.
	if( rFaceProps.size() > 0 ) {
		filestr << "element face " << rFaceProps.size() << endl;
		filestr << "property list uchar int32 vertex_indices" << endl;
	}

	//! .) Proprietary support for polygonal lines (generated by user and/or algorithm), when present.
	if( ( mExportFlags[EXPORT_POLYLINE] ) && ( getPolyLineNr() > 0 ) ) {
		filestr << "element line " << getPolyLineNr() << endl;
		filestr << "property float x" << endl;
		filestr << "property float y" << endl;
		filestr << "property float z" << endl;
		filestr << "property float nx" << endl;
		filestr << "property float ny" << endl;
		filestr << "property float nz" << endl;
		filestr << "property uint32 labelid" << endl;
		filestr << "property list int32 int32 vertex_indices" << endl;
	}

	//! .) End of header tag.
	filestr << "end_header" << endl;

	if( !mExportFlags[EXPORT_BINARY] ) {
		//! \todo Test ASCII export with and without texture.
		// === ASCII mode ===========================================================
		// --- Vertices -------------------------------------------------------------
		uint64_t featureVecLenMax = getFeatureVecLenMax( Primitive::IS_VERTEX );
		for( size_t i=0; i<rVertexProps.size(); i++ ) {
			filestr << rVertexProps.at( i ).mCoordX  << " ";   // x-coordinate
			filestr << rVertexProps.at( i ).mCoordY  << " ";   // y-coordinate
			filestr << rVertexProps.at( i ).mCoordZ  << " ";   // z-coordinate
			filestr << rVertexProps.at( i ).mFuncVal;          // function value stored within PLY_QUALITY
			if( mExportFlags[EXPORT_VERT_FLAGS] ) {
				filestr << " ";
				filestr << static_cast<unsigned int>(rVertexProps.at( i ).mFlags); // flags lower half (int32 vs long64!)
			}
			if( mExportFlags[EXPORT_VERT_COLOR] ) {
				filestr << " ";
				filestr << static_cast<unsigned int>(rVertexProps.at( i ).mColorRed)   << " ";
				filestr << static_cast<unsigned int>(rVertexProps.at( i ).mColorGrn) << " ";
				filestr << static_cast<unsigned int>(rVertexProps.at( i ).mColorBle);
				//! \todo Alpha is ignored and not written to output.
			}
			if( mExportFlags[EXPORT_VERT_NORMAL] ) {
				filestr << " ";
				filestr << rVertexProps.at( i ).mNormalX << " ";
				filestr << rVertexProps.at( i ).mNormalY << " ";
				filestr << rVertexProps.at( i ).mNormalZ;
			}
			if( mExportFlags[EXPORT_VERT_LABEL] ) {
				filestr << " ";
				filestr << rVertexProps.at( i ).mLabelId;
			}
			if( mExportFlags[EXPORT_VERT_FTVEC] ) {
				//! \todo test: write ASCII PLY with features
				filestr << " ";
				filestr << featureVecLenMax << " ";
				for( uint64_t j=0; j<getFeatureVecLenMax( Primitive::IS_VERTEX ); j++ ) {
					float featureToWrite = static_cast<float>(mFeatureVecVertices.at( featureVecLenMax*i+j ));
					//filestr << getFeatureVertex( i, j ) << " ";
					filestr << featureToWrite << " ";
				}
			}
			filestr << endl;
		}
		// --- Faces ----------------------------------------------------------------
		for( int i=0; i<rFaceProps.size(); i++ ) {
			// PLYs start with ZERO! So no +1 needed (in contrast to OBJ)
			filestr << "3 ";
			filestr << rFaceProps[i].mVertIdxA << " ";
			filestr << rFaceProps[i].mVertIdxB << " ";
			filestr << rFaceProps[i].mVertIdxC << endl;
		}
		// ---- Polygonal lines -----------------------------------------------------
		for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
			PrimitiveInfo primInfo = getPolyLinePrimInfo( i );
			filestr << primInfo.mPosX << endl;
			filestr << primInfo.mPosY << endl;
			filestr << primInfo.mPosZ << endl;
			filestr << primInfo.mNormalX << endl;
			filestr << primInfo.mNormalY << endl;
			filestr << primInfo.mNormalZ << endl;
			// The label ID:                      property uint32 labelid
			unsigned int polyLabelID = getPolyLineLabel( i );
			filestr << polyLabelID;
			filestr << endl;
			// Than a list of vertex references:  property list int32 int32 vertex_indices
			filestr << getPolyLineLength( i );
			// Finally the references to the vertices:
			for( unsigned int j=0; j<getPolyLineLength( i ); j++ ) {
				filestr << " " << getPolyLineVertIdx( i, j );
			}
			filestr << endl;
		}
	} else {
		// === Binary mode - tested and working, but maybe not nicely implemented ===
		// --- Vertices -------------------------------------------------------------
		high_resolution_clock::time_point tStartVertices = high_resolution_clock::now();
		unsigned char texVal;
		unsigned long featureVecLenMax = getFeatureVecLenMax( Primitive::IS_VERTEX );
		for( size_t i=0; i<rVertexProps.size(); i++ ) {
			float someFloat = static_cast<float>( rVertexProps.at( i ).mCoordX );
			filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
			someFloat = static_cast<float>( rVertexProps.at( i ).mCoordY );
			filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 );
			someFloat = static_cast<float>( rVertexProps.at( i ).mCoordZ );
			filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 );
			someFloat = static_cast<float>( rVertexProps.at( i ).mFuncVal );   // function value stored ...
			filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // ... within PLY_QUALITY
			if( mExportFlags[EXPORT_VERT_FLAGS] ) {
				int vertFlags = static_cast<int>(rVertexProps.at( i ).mFlags);
				filestr.write( reinterpret_cast<char*>(&vertFlags), PLY_INT32 );
			}
			if( mExportFlags[EXPORT_VERT_COLOR] ) {
				texVal = rVertexProps.at( i ).mColorRed;
				filestr.write( reinterpret_cast<char*>(&texVal), PLY_UINT8 );
				texVal = rVertexProps.at( i ).mColorGrn;
				filestr.write( reinterpret_cast<char*>(&texVal), PLY_UINT8 );
				texVal = rVertexProps.at( i ).mColorBle;
				filestr.write( reinterpret_cast<char*>(&texVal), PLY_UINT8 ); 
				//! \todo Alpha is ignored and not written.
			}
			if( mExportFlags[EXPORT_VERT_NORMAL] ) {
				float someFloat = static_cast<float>(rVertexProps.at( i ).mNormalX);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(rVertexProps.at( i ).mNormalY);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 );
				someFloat = static_cast<float>(rVertexProps.at( i ).mNormalZ);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 );
			}
			if( mExportFlags[EXPORT_VERT_LABEL] ) {
				unsigned int vertLabelID = rVertexProps.at( i ).mLabelId;
				filestr.write( reinterpret_cast<char*>(&vertLabelID), PLY_UINT32 );
			}
			if( mExportFlags[EXPORT_VERT_FTVEC] ) {
				filestr << static_cast<unsigned char>(featureVecLenMax);
				for( uint64_t j=0; j<featureVecLenMax; j++ ) {
					float featureToWrite = static_cast<float>(mFeatureVecVertices.at( featureVecLenMax*i+j ));
					filestr.write( reinterpret_cast<char*>(&(featureToWrite)), PLY_FLOAT32 ); 
				}
			}
		}
		high_resolution_clock::time_point tEndVertices = high_resolution_clock::now();
		duration<double> time_span_Vertices = duration_cast<duration<double>>( tEndVertices - tStartVertices );
		cout << "[MeshIO::" << __FUNCTION__ << "] write Vertices:         " << time_span_Vertices.count() << " seconds." << std::endl;
		// --- Faces ----------------------------------------------------------------
		{
			high_resolution_clock::time_point tStartFaces = high_resolution_clock::now();
			char numberOfVerticesPerFace = 3;
			for( unsigned long i=0; i<rFaceProps.size(); i++ ) {
				// PLYs start with ZERO!
				filestr.write( &numberOfVerticesPerFace, PLY_UCHAR ); // uchar have 1 byte in a binary PLY
				unsigned long someIdx = rFaceProps[i].mVertIdxA;
				filestr.write( reinterpret_cast<char*>(&someIdx), PLY_INT32 ); // floats have 4 bytes in a binary PLY
				someIdx = rFaceProps[i].mVertIdxB;
				filestr.write( reinterpret_cast<char*>(&someIdx), PLY_INT32 ); // floats have 4 bytes in a binary PLY
				someIdx = rFaceProps[i].mVertIdxC;
				filestr.write( reinterpret_cast<char*>(&someIdx), PLY_INT32 ); // floats have 4 bytes in a binary PLY
			}
			high_resolution_clock::time_point tEndFaces = high_resolution_clock::now();
			duration<double> time_span_Faces = duration_cast<duration<double>>( tEndFaces - tStartFaces );
			cout << "[MeshIO::" << __FUNCTION__ << "] write Faces:            " << time_span_Faces.count() << " seconds." << std::endl;
		}
		// ---- Polygonal lines -----------------------------------------------------
		if( ( mExportFlags[EXPORT_POLYLINE] ) && ( getPolyLineNr() > 0 ) ) {
			high_resolution_clock::time_point tStartPolylines = high_resolution_clock::now();
			for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
				PrimitiveInfo primInfo = getPolyLinePrimInfo( i );
				float someFloat = static_cast<float>(primInfo.mPosX);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(primInfo.mPosY);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(primInfo.mPosZ);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(primInfo.mNormalX);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(primInfo.mNormalY);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				someFloat = static_cast<float>(primInfo.mNormalZ);
				filestr.write( reinterpret_cast<char*>(&someFloat), PLY_FLOAT32 ); // floats have 4 bytes in a binary PLY
				// The label ID:                      property uint32 labelid
				unsigned int polyLabelID = getPolyLineLabel( i );
				filestr.write( reinterpret_cast<char*>(&polyLabelID), PLY_UINT32 );
				// Than a list of vertex references:  property list int32 int32 vertex_indices
				int polyLen = static_cast<int>(getPolyLineLength( i ));
				filestr.write( reinterpret_cast<char*>(&polyLen), PLY_INT32 );
				// Finally the references to the vertices:
				for( unsigned int j=0; j<getPolyLineLength( i ); j++ ) {
					int vertIdx = getPolyLineVertIdx( i, j );
					filestr.write( reinterpret_cast<char*>(&vertIdx), PLY_INT32 );
				}
			}
			high_resolution_clock::time_point tEndPolylines = high_resolution_clock::now();
			duration<double> time_span_Polylines = duration_cast<duration<double>>( tEndPolylines - tStartPolylines );
			cout << "[MeshIO::" << __FUNCTION__ << "] write Polylines:        " << time_span_Polylines.count() << " seconds." << std::endl;
		} else {
			cout << "[MeshIO::" << __FUNCTION__ << "] write Polylines:        None present or export disabled." << std::endl;
		}
	}

	filestr.close();

	tEnd = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>( tEnd - tStart );
	cout << "[MeshIO::" << __FUNCTION__ << "] ------------------------------------------------------------" << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] write TOTAL:            " << time_span.count() << " seconds." << endl;
	cout << "[MeshIO::" << __FUNCTION__ << "] ------------------------------------------------------------" << endl;

	return true;
}

//! Writes a Mesh as Wavefront Alias ASCII file from a given MeshSeed.
//! For the moment no texture will be exported!
bool MeshIO::writeOBJ(
                const string& rFilename,
                const std::vector<sVertexProperties>& rVertexProps,
                const std::vector<sFaceProperties>& rFaceProps
) {
	fstream filestr;
	int timeStart; // for performance mesurement

	timeStart = clock();
	filestr.open( rFilename.c_str(), fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Could not open file: '" << rFilename << "'!" << endl;
		return false;
	} else {
		cout << "[MeshIO::" << __FUNCTION__ << "] File open for writing: '" << rFilename << "'." << endl;
	}
	filestr << "#===============================================================================" << endl;
	filestr << "# File generated by the GigaMesh Software Framework" << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;
	filestr << "# WebSite: https://gigamesh.eu" << endl;
	filestr << "# EMail:   info@gigamesh.eu" << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;
	filestr << "# Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>" << endl;
	filestr << "#          IWR - Heidelberg University, Germany" << endl;
	filestr << "#===============================================================================" << endl;
	filestr << "# GigaMesh compiled" << endl;
#ifdef COMP_USER
	filestr << "# .... by: " << COMP_USER << endl;
#else
	filestr << "# .... by: UNKNOWN" << endl;
#endif
#ifdef COMP_DATE
	filestr << "# .... at: " << COMP_DATE << endl;
#else
	filestr << "# .... at: UNKNOWN" << endl;
#endif
#ifdef COMP_EDIT
	filestr << "# ... for: " << COMP_EDIT << endl;
#else
	filestr << "# ... for: UNKNOWN" << endl;
#endif
#ifdef COMP_GITHEAD
	filestr << "# ... git-head: " << COMP_GITHEAD << endl;
#else
	filestr << "# ... git-head: UNKNOWN" << endl;
#endif
#ifdef VERSION_PACKAGE
	filestr << "# .... Version: " << VERSION_PACKAGE << endl;
#else
	filestr << "# .... Version: UNKNOWN" << endl;
#endif
	filestr << "#===============================================================================" << endl;
	filestr << "# Meta-Data: " << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;
	for( uint64_t i=0; i<META_STRINGS_COUNT; i++ ) {
		eMetaStrings metaId = static_cast<eMetaStrings>( i );
		if( metaId == META_FILENAME ) { // Ignore the filename!
			continue;
		}
		string metaStr = getModelMetaString( metaId );
		if( metaStr.size() == 0 ) { // Ignore empty strings!
			continue;
		}
		string metaName;
		if( getModelMetaStringName( metaId, metaName ) ) {
			filestr << "# " << metaName << " " << metaStr << endl;
		}
	}
	filestr << "#===============================================================================" << endl;
	filestr << "# Vertices: " << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;

	for( size_t i=0; i<rVertexProps.size(); i++ ) {
		filestr << "v ";
		filestr << rVertexProps.at( i ).mCoordX  << " ";
		filestr << rVertexProps.at( i ).mCoordY  << " ";
		filestr << rVertexProps.at( i ).mCoordZ  << " ";
		filestr << static_cast<unsigned short>( rVertexProps.at( i ).mColorRed ) << " ";
		filestr << static_cast<unsigned short>( rVertexProps.at( i ).mColorGrn ) << " ";
		filestr << static_cast<unsigned short>( rVertexProps.at( i ).mColorBle ) << " ";
		filestr << rVertexProps.at( i ).mFuncVal << " ";
		filestr << endl;
	}

	filestr << "#-------------------------------------------------------------------------------" << endl;
	filestr << "# Faces: " << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;

	if( rFaceProps.size() == 0 ) {
		filestr << "# NONE present i.e. point cloud" << endl;
	}
	for( unsigned long i=0; i<rFaceProps.size(); i++ ) {
		// OBJs start with ONE!
		filestr << "f ";
		filestr << rFaceProps[i].mVertIdxA+1 << " ";
		filestr << rFaceProps[i].mVertIdxB+1 << " ";
		filestr << rFaceProps[i].mVertIdxC+1 << endl;
	}

	filestr << "#-------------------------------------------------------------------------------" << endl;

	unsigned int lineCount = getPolyLineNr();
	if( lineCount > 0 ) {
		filestr << "# Lines: " << endl;
		filestr << "#-------------------------------------------------------------------------------" << endl;

		for( unsigned int i=0; i<lineCount; i++ ) {
			filestr << "l";
			for( unsigned int j=0; j<getPolyLineLength( i ); j++ ) {
				// OBJs start with ONE!
				filestr << " " << getPolyLineVertIdx( i, j )+1;
			}
			filestr << endl;
		}

		filestr << "#-------------------------------------------------------------------------------" << endl;
	}

	filestr.close();
	cout << "[MeshIO::" << __FUNCTION__ << "] write ASCII OBJ:      " << static_cast<float>( clock() -timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;

	return true;
}

//! Writes this Mesh as VRML 2.0 as ASCII file.
bool MeshIO::writeVRML(
                const string& rFilename,
                const std::vector<sVertexProperties>& rVertexProps,
                const std::vector<sFaceProperties>& rFaceProps
) {
	fstream filestr;
	filestr.open( rFilename.c_str(), fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[MeshIO] Could not open file: '" << rFilename << "'." << endl;
		return false;
	} else {
		cout << "[MeshIO] File open for writing: '" << rFilename << "'." << endl;
	}

	filestr << "#VRML V2.0 utf8" << endl;
	filestr << "#===============================================================================" << endl;
	filestr << "# File generated by the GigaMesh Software Framework" << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;
	filestr << "# WebSite: http://gigamesh.eu" << endl;
	filestr << "# Email:   info@gigamesh.eu" << endl;
	filestr << "#-------------------------------------------------------------------------------" << endl;
	filestr << "# Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>" << endl;
	filestr << "#          IWR - Heidelberg University, Germany" << endl;
	filestr << "#===============================================================================" << endl;
	filestr << "Shape {" << endl;
	filestr << "\tgeometry IndexedFaceSet {" << endl;
	// old=dead: if( vertexTexture != nullptr ) {
	filestr << "\t\tcolorPerVertex TRUE" << endl;
	//} else {
	//	filestr << "\t\tcolorPerVertex FALSE" << endl;
	//}
	filestr << "\t\tcoord Coordinate {" << endl;
	filestr << "\t\t\tpoint [" << endl;
	// vertexIdx -OBJs start with ONE - VRMLs with ZERO!
	for( size_t vertexIdx=0; vertexIdx<rVertexProps.size(); vertexIdx++ ) {
		filestr << "\t\t\t\t" << rVertexProps.at( vertexIdx ).mCoordX << " "
		        << rVertexProps.at( vertexIdx ).mCoordY << " " << rVertexProps.at( vertexIdx ).mCoordZ << endl;
	}
	filestr << "\t\t\t] # point" << endl;
	filestr << "\t\t} # coord Cordinate" << endl;

	filestr << "\t\tcoordIndex [" << endl;
	for( size_t i=0; i<rFaceProps.size(); i++ ) {
		// VRML index for vertices start with ZERO!!!:
		filestr << "\t\t\t" << rFaceProps[i].mVertIdxA << " "
		        << rFaceProps[i].mVertIdxB << " " << rFaceProps[i].mVertIdxC << " -1" << endl;
	}
	filestr << "\t\t] # coordIndex" << endl;

	// old=dead: if( vertexTexture != nullptr ) {
	filestr << "\t\tcolor Color {" << endl;
	filestr << "\t\t\tcolor [" << endl;
	for( size_t vertexIdx=0; vertexIdx<rVertexProps.size(); vertexIdx++ ) {
		filestr << "\t\t\t\t" << rVertexProps.at( vertexIdx ).mColorRed << ", "
		        << rVertexProps.at( vertexIdx ).mColorGrn << ", "
		        << rVertexProps.at( vertexIdx ).mColorBle  << ", " << endl;
	}
	filestr << "\t\t\t] # color" << endl;
	filestr << "\t\t} # color Color" << endl;
	// old=dead: }
	filestr << "\t} # IndexedFaceSet" << endl;
	filestr << "} # Shape " << endl;
	filestr.close();

	cout << "[MeshIO] VRML written to: " << rFilename << endl;
	return true;
}

//! Writes only the POINT CLOUD as ASCII file.
//! \todo IMPLEMENT this!
bool MeshIO::writeTXT(
                const string& rFilename,
                const std::vector<sVertexProperties>& rVertexProps,
                const std::vector<sFaceProperties>& rFaceProps
) {
	std::cerr << "[MeshIO::" << __FUNCTION__ << "] NOT IMPLEMENTED - File " << rFilename << "not written!" << std::endl;
	std::cerr << "[MeshIO::" << __FUNCTION__ << "] Vertex count:  " << rVertexProps.size() << "not written!" << std::endl;
	std::cerr << "[MeshIO::" << __FUNCTION__ << "] Face count:  " << rFaceProps.size() << "not written!" << std::endl;
	return( false );
}

// Meta-Data Strings -----------------------------------------------------------

//! Set Meta-Data as strings.
//! @returns false in case of an error. True otherwise.
bool MeshIO::setModelMetaString(
                eMetaStrings       rMetaStrID,        //!< Id of the meta-data string.
                const std::string& rModelMeta         //!< Meta-data content as string.
) {
	mMetaDataStrings[rMetaStrID] = rModelMeta;
	return( true );
}

//! Get Meta-Data as strings.
//! @returns nullptr case of an error.
std::string MeshIO::getModelMetaString(
                eMetaStrings rMetaStrID               //!< Id of the meta-data string.
) {
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no content!" << std::endl;
		return( nullptr );
	}
	return( mMetaDataStrings[rMetaStrID] );
}

//! Fetch the name of the Meta-Data strings using an Id.
//! @returns false in case of an error. True otherwise.
bool MeshIO::getModelMetaStringName(
                eMetaStrings rMetaStrID,                   //!< Id of the meta-data string.
                std::string& rModelMetaStringName          //!< Name as string of the meta-data string.
) {
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[MeshIO::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no name!" << std::endl;
		return( false );
	}
	rModelMetaStringName = mMetaDataStringNames[rMetaStrID];
	return( true );
}

//! Fetch the Id of the Meta-Data strings using its name as string.
//! @returns false in case of an error or no Id found. True otherwise.
bool MeshIO::getModelMetaStringId(
                const std::string& rModelMetaStringName,   //!< Id of the meta-data string.
                eMetaStrings& rMetaStrID                   //!< Name as string of the meta-data string.
) {
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
		if( rModelMetaStringName.compare( mMetaDataStringNames[i] ) == 0 ) {
			rMetaStrID = static_cast<eMetaStrings>( i );
			return( true );
		}
	}
	// Nothing found
	return( false );
}

//! Clear and reset meta-data.
//! @returns false in case of an error or no Id found. True otherwise.
bool MeshIO::clearModelMetaStrings() {
	// Initialize strings holding meta-data
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
		mMetaDataStrings[i].clear();
		mMetaDataStringNames[i] = "ERROR: Not Set!";
	}

	// Initialze names of the strings holding meta-data
	mMetaDataStringNames[META_MODEL_ID]         = "ModelID";
	mMetaDataStringNames[META_MODEL_MATERIAL]   = "ModelMaterial";
	mMetaDataStringNames[META_FILENAME]         = "ModelFileName";
	mMetaDataStringNames[META_REFERENCE_WEB]    = "ModelReferenceWeb";

	// Done.
	return( true );
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

// Private -----------------------------------------------------------------

//! Maps a parsed string from a PLY-header to a data-type.
//!
//! see also PLY-spec:
//! [Dead link!] http://www.cs.kuleuven.ac.be/~ares/libply/ply-0.1/doc/PLY_FILES.txt
//! working alternative: https://people.sc.fsu.edu/~jburkardt/data/ply/ply.html
MeshIO::ePlyPropertySize MeshIO::plyParseTypeStr( char* propType ) {
	ePlyPropertySize propertyDataType = PLY_SIZE_UNDEF;
	if( strcmp( propType, "int8" ) == 0 ) {
		propertyDataType = PLY_INT8;
	} else if( strcmp( propType, "uint8" ) == 0 ) {
		propertyDataType = PLY_UINT8;
	} else if( strcmp( propType, "int16" ) == 0 ) {
		propertyDataType = PLY_SHORT_INT;
	} else if( strcmp( propType, "uint16" ) == 0 ) {
		propertyDataType = PLY_USHORT_INT;
	} else if( strcmp( propType, "int" ) == 0 ) {
		propertyDataType = PLY_INT32;
	} else if( strcmp( propType, "int32" ) == 0 ) {
		propertyDataType = PLY_INT32;
	} else if( strcmp( propType, "uint32" ) == 0 ) {
		propertyDataType = PLY_UINT32;
	} else if( strcmp( propType, "uint" ) == 0 ) { // assume 'uint' as 32-bit
		propertyDataType = PLY_UINT32;
		cout << "[MeshIO::" << __FUNCTION__ << "] Warning: assuming 'uint' having 4 byte." << endl;
	} else if( strcmp( propType, "float32" ) == 0 ) {
		propertyDataType = PLY_FLOAT32;
	} else if( strcmp( propType, "float64" ) == 0 ) {
		propertyDataType = PLY_FLOAT64;
	} else if( strcmp( propType, "float" ) == 0 ) {
		propertyDataType = PLY_FLOAT;
	} else if( strcmp( propType, "char" ) == 0 ) {
		propertyDataType = PLY_CHAR;
	} else if( strcmp( propType, "uchar" ) == 0 ) {
		propertyDataType = PLY_UCHAR;
	} else {
		cerr << "[MeshIO::" << __FUNCTION__ << "] unknown type '" << propType << "' in header!" << endl;
		propertyDataType = PLY_SIZE_UNDEF;
	}
	return propertyDataType;
}
