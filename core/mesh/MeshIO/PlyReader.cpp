#include "PlyReader.h"
#include <fstream>
#include <sstream>
#include <iterator>   // istream_iterator
#include <algorithm>
#include <cctype>
#include <clocale>
#include "PlyEnums.h"

#include <GigaMesh/logging/Logging.h>

using uint = unsigned int;

template <class T>
void READ_IN_PROPER_BYTE_ORDER(std::fstream& filestream, T target, size_t size, bool reverse)
{
	auto data = reinterpret_cast<char*>(target);
	(filestream).read(data , (size) );

	if(reverse && size != 1)
	{
		std::reverse(data, data + size);
	}
}

struct plyContainer {
	std::vector<ePlyProperties> propertyType;
	std::vector<int> propertyDataType;
	std::vector<int> propertyListCountDataType;
	std::vector<int> propertyListDataType;
};

ePlyPropertySize plyParseTypeStr( char* propType );

/*
 * Helper class to set and restore locale in RAII fashion
 */
class LocaleGuard {
	public:
		LocaleGuard() {mOldLocale = std::setlocale(LC_NUMERIC, nullptr); std::setlocale(LC_NUMERIC, "C");}
		~LocaleGuard() {std::setlocale(LC_NUMERIC, mOldLocale.c_str());}

	private:
		std::string mOldLocale;
};

void copyVertexTexCoordsToFaces(const std::vector<float>& vertexTextureCoordinates, std::vector<sFaceProperties>& rFaceProps)
{
	for(auto & prop : rFaceProps)
	{
		prop.textureCoordinates[0] = vertexTextureCoordinates[prop.mVertIdxA * 2];
		prop.textureCoordinates[1] = vertexTextureCoordinates[prop.mVertIdxA * 2 + 1];

		prop.textureCoordinates[2] = vertexTextureCoordinates[prop.mVertIdxB * 2];
		prop.textureCoordinates[3] = vertexTextureCoordinates[prop.mVertIdxB * 2 + 1];

		prop.textureCoordinates[4] = vertexTextureCoordinates[prop.mVertIdxC * 2];
		prop.textureCoordinates[5] = vertexTextureCoordinates[prop.mVertIdxC * 2 + 1];
	}
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
bool PlyReader::readFile(const std::string& rFilename,
				std::vector<sVertexProperties>& rVertexProps,
				std::vector<sFaceProperties>& rFaceProps,
				MeshSeedExt& rMeshSeed) {
	std::fstream filestr;
	std::string  lineToParse;
	std::string  lineToParseOri;
	int     linesComment = 0;
	bool    readASCII = false;

	const LocaleGuard guard;

	int timeStart = clock(); // for performance mesurement

	std::cout << "[PlyReader::" << __FUNCTION__ << "] opening: '" << rFilename << "'\n";
	filestr.open( rFilename.c_str(), std::fstream::in );
	if( !filestr.is_open() ) {
		LOG::error() << "[PlyReader::" << __FUNCTION__ << "] could not open: '" << rFilename << "'!\n";
		return false;
	}

	bool reverseByteOrder   = false;
	bool endOfHeader        = false;
	bool hasVertexTexCoords = false;

	uint64_t plyCurrentSection = PLY_SECTION_UNSUPPORTED;
	uint64_t plyElements[PLY_SECTIONS_COUNT];

	for( uint64_t& plyElement : plyElements ) {
		plyElement = 0;
	}

	plyContainer sectionProps[PLY_SECTIONS_COUNT];

	// the first two lines we expect: "ply" -- maybe add a check?
	getline( filestr, lineToParse );
	// than the header begins, which is typically ASCII:
	while( !filestr.eof() && !endOfHeader) {
		getline( filestr, lineToParseOri );
		lineToParse = lineToParseOri;

		for(char& character : lineToParse)
		{
			character = std::tolower(character);
		}

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
			std::istringstream iss( lineToParseOri );
			std::vector<std::string> lineParts{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
			if( lineParts.size() < 2 ) {
				continue;
			}
			std::string possibleMetaDataName = lineParts[ 1 ];
			ModelMetaData::eMetaStrings foundMetaId;
			if( MeshReader::getModelMetaDataRef().getModelMetaStringId( possibleMetaDataName, foundMetaId ) ) {
				uint64_t preMetaLen = 9 + possibleMetaDataName.size(); // 7 for 'comment' plus 2x space.
				std::string metaContent = lineToParseOri.substr( preMetaLen );
				LOG::info() << "[PlyReader::" << __FUNCTION__ << "] Meta-Data: " << possibleMetaDataName << " (" << foundMetaId << ") = " << metaContent << "\n";

				if( foundMetaId == ModelMetaData::META_TEXTUREFILE)
				{
					MeshReader::getModelMetaDataRef().addTextureName(metaContent);
				}

				else
				{
					if( !MeshReader::getModelMetaDataRef().setModelMetaString( foundMetaId, metaContent ) ) {
						LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Meta-Data not set!" << "\n";
					}
				}
			}
			continue;
		}
		// --------------------------------------------------------------------------------------------

		if( lineToParse.substr( 0, 6 ) == "format" ) {
			if( lineToParse == "format binary_big_endian 1.0" ) {
				LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] File is BIG Endian.\n";
				if( not( mSystemIsBigEndian ) ) {
					reverseByteOrder = true;
				}
			} else if( lineToParse == "format binary_little_endian 1.0" ) {
				LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] File is LITTLE Endian.\n";
				if( mSystemIsBigEndian ) {
					reverseByteOrder = true;
				}
			} else {
				readASCII = true;
				LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] try to read as ASCII.\n";
				LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] Line: " << lineToParse << "\n";
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
			if( sscanf( lineToParse.c_str(), "element vertex %lu", &plyElements[PLY_VERTEX] ) == 1 ) {
				LOG::info() << "[PlyReader::" << __FUNCTION__ << "] Vertices: " << plyElements[PLY_VERTEX] << "\n";
				plyCurrentSection = PLY_VERTEX;
				// allocate memory - assume color per vertex:
				rVertexProps.resize( plyElements[PLY_VERTEX] );

				// init vertex coordinates and function value
				for( auto& vertexProp : rVertexProps) {
					vertexProp.mCoordX = _NOT_A_NUMBER_DBL_;
					vertexProp.mCoordY = _NOT_A_NUMBER_DBL_;
					vertexProp.mCoordZ = _NOT_A_NUMBER_DBL_;
					// init function value with zero as NaN results in performance issues by repeatingly checking min/max values in MeshGL::getFuncValMinMaxUser.
					vertexProp.mFuncVal = 0.0;
					// init colors
					vertexProp.mColorRed = 187;
					vertexProp.mColorGrn = 187;
					vertexProp.mColorBle = 187;
					vertexProp.mColorAlp = 255;
				}
			// Faces
			} else if( sscanf( lineToParse.c_str(), "element face %lu", &plyElements[PLY_FACE] ) == 1 ) {
				LOG::info() << "[PlyReader::" << __FUNCTION__ << "] Faces: " << plyElements[PLY_FACE] << "\n";
				plyCurrentSection = PLY_FACE;
				// allocate memory:
				rFaceProps.resize( plyElements[PLY_FACE] );
			} else if( sscanf( lineToParse.c_str(), "element line %lu", &plyElements[PLY_POLYGONAL_LINE] ) == 1 ) {
				LOG::info() << "[PlyReader::" << __FUNCTION__ << "] Polygonal lines: " << plyElements[PLY_POLYGONAL_LINE] << "\n";
				plyCurrentSection = PLY_POLYGONAL_LINE;
			// Unsupported
			} else {
				char elementName[255];
				int  elementCount;
				sscanf( lineToParse.c_str(), "element %s %i", elementName, &elementCount );
				plyElements[PLY_SECTION_UNSUPPORTED] += elementCount;
				plyCurrentSection = PLY_SECTION_UNSUPPORTED;
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unknown element '" << elementName << "', count: " << elementCount << "!\n";
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
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Error while parsing list properties!\n";
				propertyParsed = false;
			}
			propertyListCountDataType = plyParseTypeStr( propTypeCount );
			propertyListDataType      = plyParseTypeStr( propType );
			if( strcmp( propName, "feature_vector" ) == 0 ) {
				propertyType = PLY_LIST_FEATURE_VECTOR;
			} else if( strcmp( propName, "vertex_indices" ) == 0 || // ... more commonly used keyword
					   strcmp( propName, "vertex_index" ) == 0 ) {  // ... used by some low-cost devices
				propertyType = PLY_LIST_VERTEX_INDICES;
			} else if(strcmp( propName, "texcoord") == 0) {
				propertyType = PLY_LIST_TEXCOORDS;
				getModelMetaDataRef().setHasTextureCoordinates(true);
			} else {
				propertyType = PLY_LIST_IGNORE;
			}
			if( ( propertyListDataType == PLY_SIZE_UNDEF ) || ( propertyListCountDataType == PLY_SIZE_UNDEF ) ) {
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Line: " << lineToParse << "\n";
			}
		} else if( lineToParse.substr( 0, 8 ) == "property" ) {
			propertyParsed = true;
			char propType[255];
			char propName[255];
			if( sscanf( lineToParse.c_str(), "property %s %s", propType, propName ) != 2 ) {
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Error while parsing single properties!\n";
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
			} else if (strcmp( propName, "s") == 0) {
				propertyType = PLY_VERTEX_TEXCOORD_S;
			} else if (strcmp( propName, "t") == 0) {
				propertyType = PLY_VERTEX_TEXCOORD_T;
			} else if (strcmp (propName, "texnumber") == 0) {
				propertyType = PLY_FACE_TEXNUMBER;
			} else {
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unknown property '" << propName << "' in header!\n";
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Line: " << lineToParse << "\n";
				propertyType = PLY_UNSUPPORTED;
			}
			propertyDataType = plyParseTypeStr( propType );
			if( propertyDataType == PLY_SIZE_UNDEF ) {
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Line: " << lineToParse << "\n";
			}
		}

		// when no property was parsed we continue
		if( !propertyParsed ) {
			continue;
		}
		switch( plyCurrentSection ) {
			//! \todo Is this fallthrough intentional?
			case PLY_SECTION_UNSUPPORTED:
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Property of an unsupported type: " << lineToParse << "\n";
				break;
			case PLY_POLYGONAL_LINE:
			case PLY_VERTEX:
				if(propertyType == PLY_VERTEX_TEXCOORD_S || propertyType == PLY_VERTEX_TEXCOORD_T)
				{
					hasVertexTexCoords = true;
				}
			case PLY_FACE:
				sectionProps[plyCurrentSection].propertyType.push_back( propertyType );
				sectionProps[plyCurrentSection].propertyDataType.push_back( propertyDataType );
				sectionProps[plyCurrentSection].propertyListCountDataType.push_back( propertyListCountDataType );
				sectionProps[plyCurrentSection].propertyListDataType.push_back( propertyListDataType );
				break;
			default:
				LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] undefined section type: " << plyCurrentSection << "!\n";
		}
	}

	if( !endOfHeader ) {
		// no end of the header found means some trouble:
		LOG::error() << "[PlyReader::" << __FUNCTION__ << "] failed: no end of header was found!\n";
		filestr.close();
		return( false );
	}


	//--------------------------- END OF HEADER PARSING ------------------------------------------------------------------------------------------------

	unsigned char listNrChar;
	uint64_t verticesRead      = 0;
	uint64_t facesRead         = 0;
	uint64_t polyLinesRead     = 0;
	uint64_t unsupportedRead   = 0;
	char  charProp;
	long  bytesIgnored = 0;
	long  extraBytesIgnored = 0;
	long  posInFile;
	float someFloat;
	int   someInt;

	std::vector<int>::iterator plyProp;
	std::vector<int>::iterator plyPropSize;
	std::vector<int>::iterator plyPropListCountSize;
	std::vector<int>::iterator plyPropListSize;

	std::vector<float> vertexTextureCoordinates;
	if(hasVertexTexCoords)
	{
		getModelMetaDataRef().setHasTextureCoordinates(true);
		vertexTextureCoordinates.resize(rVertexProps.size() * 2);
	}
	//---------------------------------- PARSE ASCII FILES----------------------------------------------------------------

	if( readASCII ) {
		//! \todo still incomplete: flags and feature vectors not tested and not supported.


		//--------------------------------- PARSE VERTICES --------------------------------------------------------

		for( verticesRead=0; verticesRead<plyElements[PLY_VERTEX]; verticesRead++ ) {
			getline( filestr, lineToParse );
			// Remove trailing '\r' e.g. from files provided from some low-cost scanners
			if( !lineToParse.empty() && lineToParse[lineToParse.size() - 1] == '\r' ) {
				lineToParse.erase( lineToParse.size() - 1 );
			}
			// Break line into tokens:
			std::istringstream iss( lineToParse );
			std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
			if( tokens.size() == 0 ) {
				continue; // Empty line
			}
			// Parse each token of a line:
			for( uint64_t i=0; i<tokens.size(); i++ ) {
				if( i>=sectionProps[PLY_VERTEX].propertyType.size() ) {
					// Sanity check
					LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] verticesRead: " << verticesRead << " of " << plyElements[PLY_VERTEX] << "\n";
					LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] More tokens than properties " << i << " >= " << sectionProps[PLY_VERTEX].propertyType.size() << "!\n";
					continue;
				}
				std::string lineElement = tokens[ i ];
				ePlyProperties currProperty = sectionProps[PLY_VERTEX].propertyType[ i ];
				switch( currProperty ) {
					case PLY_COORD_X:
						rVertexProps[ verticesRead ].mCoordX = atof( lineElement.c_str() );
						break;
					case PLY_COORD_Y:
						rVertexProps[ verticesRead ].mCoordY = atof( lineElement.c_str() );
						break;
					case PLY_COORD_Z:
						rVertexProps[ verticesRead ].mCoordZ = atof( lineElement.c_str() );
						break;
					case PLY_VERTEX_NORMAL_X:
						rVertexProps[ verticesRead ].mNormalX = atof(lineElement.c_str() );
						break;
					case PLY_VERTEX_NORMAL_Y:
						rVertexProps[ verticesRead ].mNormalY = atof(lineElement.c_str() );
						break;
					case PLY_VERTEX_NORMAL_Z:
						rVertexProps[ verticesRead ].mNormalZ = atof(lineElement.c_str() );
						break;
					case PLY_VERTEX_TEXCOORD_S:
						if(!vertexTextureCoordinates.empty())
							vertexTextureCoordinates[ verticesRead * 2] = atof(lineElement.c_str());
						break;
					case PLY_VERTEX_TEXCOORD_T:
						if(!vertexTextureCoordinates.empty())
							vertexTextureCoordinates[ verticesRead * 2 + 1] = atof(lineElement.c_str());
						break;
					case PLY_FLAGS:
						rVertexProps[ verticesRead ].mFlags = static_cast<unsigned long>(atoi( lineElement.c_str() ));
						break;
					case PLY_LABEL:
						rVertexProps[ verticesRead ].mLabelId = static_cast<unsigned long>(atoi( lineElement.c_str() ));
						break;
					case PLY_VERTEX_QUALITY:
						rVertexProps[ verticesRead ].mFuncVal = atof( lineElement.c_str() );
						break;
					case PLY_VERTEX_INDEX:
						break;
					case PLY_COLOR_RED:
						rVertexProps[ verticesRead ].mColorRed = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_GREEN:
						rVertexProps[ verticesRead ].mColorGrn = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_BLUE:
						rVertexProps[ verticesRead ].mColorBle = atoi( lineElement.c_str() );
						break;
					case PLY_COLOR_ALPHA:
						//! \todo alpha is read but ignored later on.
						rVertexProps[ verticesRead ].mColorAlp = atoi( lineElement.c_str() );
						break;
					case PLY_LIST_IGNORE:
					case PLY_LIST_VERTEX_INDICES:
						listNrChar = atoi( lineElement.c_str() );
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							//! \todo implement PLY_LIST_VERTEX_INDICES for ASCII PLYs
							// ...
							++i;
						}
						break;
					case PLY_LIST_TEXCOORDS:
						//! \todo texture coordinates for ascii-files
						listNrChar = atoi(lineElement.c_str() );
						for(uint j=0; j<static_cast<uint>(listNrChar); ++j)
						{
							//...
							++i;
						}

						break;
					case PLY_LIST_FEATURE_VECTOR:
						listNrChar = atoi( lineElement.c_str() );
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							//! \todo implement PLY_LIST_FEATURE_VECTOR for ASCII PLYs
							// someFloat = atof( lineElement.c_str() );
							// featureVecVertices[verticesRead*featureVecVerticesLen+j] = (double)someFloat;
						}
						//LOG::info() << "\n";
						break;
					case PLY_UNSUPPORTED:
					default:
						// Read but ignore
						LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Unknown property in vertex section " << currProperty << " !\n";
				}
			}
		}
		std::cout << "[PlyReader::" << __FUNCTION__ << "] Reading vertices done.\n";


		//--------------------------------- PARSE FACES --------------------------------------------------------

		// Read faces
		for( facesRead=0; facesRead<plyElements[PLY_FACE]; facesRead++ ) {
			getline( filestr, lineToParse );
			std::string strLine( lineToParse );
			// Remove trailing '\r' e.g. from files provided from some low-cost scanners
			if( !lineToParse.empty() && lineToParse[lineToParse.size() - 1] == '\r' ) {
				lineToParse.erase( lineToParse.size() - 1 );
			}
			// Break line into tokens:
			std::istringstream iss( lineToParse );
			std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };
			if( tokens.size() == 0 ) {
				continue; // Empty line
			}
			// Parse each token of a line:
			auto currPropertyIt = sectionProps[PLY_FACE].propertyType.begin();

			for( uint64_t i=0; i<tokens.size(); i++ ) {
				if(currPropertyIt == sectionProps[PLY_FACE].propertyType.end())
				{
					LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Unknown property in face section!\n";
					continue;
				}

				std::string lineElement = tokens[ i ];
				ePlyProperties currProperty = *currPropertyIt;
				switch( currProperty ) {
					case PLY_LIST_VERTEX_INDICES: {
							uint64_t elementCount = atoi( lineElement.c_str() );
							if( elementCount != 3 ) {
								i += elementCount; // These have to be skipped by the outer loop.
								LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unsupported number (" << listNrChar << ") of elements within the list!\n";
								continue;
							}
							std::string listElementInLine = tokens[ i+1 ];
							uint64_t vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxA = static_cast<int>(vertexIndexNr);
							listElementInLine = tokens[ i+2 ];
							vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxB = static_cast<int>(vertexIndexNr);
							listElementInLine = tokens[ i+3 ];
							vertexIndexNr = atoi( listElementInLine.c_str() );
							rFaceProps[facesRead].mVertIdxC = static_cast<int>(vertexIndexNr);
							i += elementCount; // These have to be skipped by the outer loop.
						} break;
					case PLY_LIST_TEXCOORDS: {
							auto elementCount = atoi(lineElement.c_str());
							if(elementCount != 6) {
								i += elementCount; // these have to be skipped by the outer loop
								LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unsupported number (" << listNrChar << ") of elements within the list!\n";
								continue;
							}
							for(int j = 0; j < elementCount; ++j)
							{
								rFaceProps[facesRead].textureCoordinates[j] = static_cast<float>(atof(tokens[++i].c_str()));
							}
						} break;
					case PLY_FACE_TEXNUMBER:
							rFaceProps[facesRead].textureId = atoi(lineElement.c_str());
							++i;
						break;
					default:
						// Read but ignore
						LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] Unknown property in face section " << currProperty << " !\n";
				}
				++currPropertyIt;
			}
		}
		std::cout << "[PlyReader::" << __FUNCTION__ << "] Reading faces done.\n";
		//! \todo ASCII: add support for selected vertices.
		//! \todo ASCII: add support for polylines.
		//! \todo ASCII: add support for unsupported lines.
		filestr.close();

		if(hasVertexTexCoords && !vertexTextureCoordinates.empty())
		{
			copyVertexTexCoordsToFaces(vertexTextureCoordinates, rFaceProps);
		}

		return( true );
	}

	//---------------------------------- PARSE BINARY FILES----------------------------------------------------------------
	// ReOpen file, when binary!
	if( !readASCII ) {
		filestr.close();

		filestr.open( rFilename.c_str(), std::fstream::in | std::ios_base::binary );
		if( !filestr.is_open() ) {
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "] could not open: '" << rFilename << "'!\n";
			return false;
		}
		filestr.seekg( 0, filestr.end );
		uint64_t fileLength = filestr.tellg();
		filestr.seekg( 0, filestr.beg );
		LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] File length for reading binary: " << fileLength << " Bytes.\n";

		//! \todo buffer can not allocate sufficent memory for very large 3D-models. Anyway it makes no sense to load the whole file jus to search for the end of the header. The latter is relativly small.
		std::vector<char> buffer;
		try{
			buffer.resize( fileLength );
		} catch( const std::exception& e ) {
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "] A standard exception was caught, with message '"
				 << e.what() << "'\n";
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "]        File length: " << fileLength << "\n";
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "]        Size of chars for the buffer: " << fileLength*sizeof( char ) << "\n";
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "]        Maximum buffer size (theoretical, but not guaranteed): " << buffer.max_size() << "\n";
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "] ...... Trying again!\n";

			try{
				// try to truncate to 10 MB
				fileLength = 10*1024*1024;
				buffer.resize( fileLength );
			} catch( const std::exception& e ) {
				LOG::error() << "[PlyReader::" << __FUNCTION__ << "] A standard exception was caught, with message '"
					 << e.what() << "'\n";
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
			LOG::error() << "[PlyReader::" << __FUNCTION__ << "] 'end_header' not found!\n";
			// Close file and return
			filestr.close();
			return( false );
		}
		int posfound = ( it - buffer.begin() );
		LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] 'end_header' found at position " << posfound << "\n";

		char newLine = buffer[ posfound+seq.size() ];
		int forwardToPos;
		if( newLine == 0x0A ) {
			forwardToPos = posfound+seq.size()+1;
			LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] one byte line break.\n";
		} else {
			newLine = buffer[ posfound+seq.size()+1 ];
			if( newLine == 0x0A ) {
				forwardToPos = posfound+seq.size()+2;
				LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] two byte line break.\n";
			} else {
				LOG::error() << "[PlyReader::" << __FUNCTION__ << "] end of header new line not found!\n";
				filestr.close();
				return false;
			}
		}
		filestr.seekg( forwardToPos );
	}

	while( filestr ) {

		//------------------------------------------- PARSE VERTICES -------------------------------------------

		if( verticesRead < plyElements[PLY_VERTEX] ) {
			plyPropSize          = sectionProps[PLY_VERTEX].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_VERTEX].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_VERTEX].propertyListDataType.begin();
			double vertNormalX = _NOT_A_NUMBER_DBL_;
			double vertNormalY = _NOT_A_NUMBER_DBL_;
			double vertNormalZ = _NOT_A_NUMBER_DBL_;
			for(auto currProperty : sectionProps[PLY_VERTEX].propertyType) {
					switch( currProperty ) {
					case PLY_COORD_X:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mCoordX = static_cast<double>(someFloat);
						if( filestr.tellg() < 0 ) {
							LOG::warn() << "insufficient coordinates provided for position vector\n";
						}
						break;
					case PLY_COORD_Y:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mCoordY = static_cast<double>(someFloat);
						if( filestr.tellg() < 0 ) {
							LOG::warn() << "insufficient coordinates provided for position vector\n";
						}
						break;
					case PLY_COORD_Z:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mCoordZ = static_cast<double>(someFloat);
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
						rVertexProps[ verticesRead ].mFlags = someFlags;
						} break;
					case PLY_LABEL: {
						unsigned int someLabelID;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someLabelID, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mLabelId = someLabelID;
						} break;
					case PLY_VERTEX_QUALITY:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mFuncVal = static_cast<double>(someFloat);
						break;
					case PLY_COLOR_RED:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mColorRed = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_GREEN:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mColorGrn = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_BLUE:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						rVertexProps[ verticesRead ].mColorBle = static_cast<unsigned char>(charProp);
						break;
					case PLY_COLOR_ALPHA:
						READ_IN_PROPER_BYTE_ORDER( filestr, &charProp, (*plyPropSize), reverseByteOrder );
						//! \todo Alpha is read but ignored later on.
						rVertexProps[ verticesRead ].mColorAlp = static_cast<unsigned char>(charProp);
						break;
					case PLY_LIST_IGNORE:
					case PLY_LIST_VERTEX_INDICES:
					case PLY_LIST_TEXCOORDS:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						posInFile = filestr.tellg();
						filestr.seekg( posInFile+static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize)) );
						bytesIgnored += static_cast<long>(static_cast<uint>(listNrChar)*static_cast<uint>(*plyPropListSize));
						break;
					case PLY_LIST_FEATURE_VECTOR:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						//LOG::debug() << "listNrChar: " << (unsigned short)listNrChar << "\n";
						if( filestr.tellg() < 0 ) {
							LOG::warn() << "feature vector data is missing\n";
						}
						// When we have no memory allocated yet - this is the time (and we assume that all vertices have feature vectors of the same length or shorter.
						if( rMeshSeed.getFeatureVecVerticesRef().size() == 0 ) {
							rMeshSeed.setFeatureVecVerticesLen(static_cast<uint64_t>(listNrChar));
							rMeshSeed.getFeatureVecVerticesRef().assign( listNrChar*plyElements[PLY_VERTEX], _NOT_A_NUMBER_DBL_ );
						}
						for( uint j=0; j<static_cast<uint>(listNrChar); j++ ) {
							READ_IN_PROPER_BYTE_ORDER( filestr, &someFloat, (*plyPropListSize), reverseByteOrder );
							if( filestr.tellg() < 0 ) {
								LOG::warn() << "feature vector data is missing\n";
							}
							rMeshSeed.getFeatureVecVerticesRef()[ verticesRead*listNrChar+j ] = static_cast<double>(someFloat);
						}
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

			verticesRead++;

		//------------------------------------------- PARSE FACES -------------------------------------------
		} else if( facesRead < plyElements[PLY_FACE] ) {
			plyPropSize          = sectionProps[PLY_FACE].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_FACE].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_FACE].propertyListDataType.begin();
			for(const ePlyProperties currProperty : sectionProps[PLY_FACE].propertyType) {
					switch( currProperty ) {
					case PLY_LIST_VERTEX_INDICES:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						if( listNrChar != 3 ) {
							LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unsupported number (" << static_cast<uint>(listNrChar) << ") of elements within the list!\n";
						}
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						rFaceProps[facesRead].mVertIdxA = someInt;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						rFaceProps[facesRead].mVertIdxB = someInt;
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropListSize), reverseByteOrder );
						rFaceProps[facesRead].mVertIdxC = someInt;
						break;
					case PLY_LIST_TEXCOORDS:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
						if( listNrChar != 6) {
							LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unsupported number (" << static_cast<uint>(listNrChar) << ") of elements within the list!\n";
						}
						else {
							for(unsigned char j = 0; j<listNrChar; ++j)
							{
								float texCoord;
								READ_IN_PROPER_BYTE_ORDER( filestr, &texCoord,(*plyPropListSize), reverseByteOrder);
								rFaceProps[facesRead].textureCoordinates[j] = texCoord;
							}
						}
						break;
					case PLY_FACE_TEXNUMBER:
						READ_IN_PROPER_BYTE_ORDER( filestr, &someInt, (*plyPropSize), reverseByteOrder);
						rFaceProps[facesRead].textureId = static_cast<unsigned char>(someInt);
						break;
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_IGNORE:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
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

			facesRead++;
		//------------------------------------------- PARSE POLYLINES -------------------------------------------
		} else if( polyLinesRead < plyElements[PLY_POLYGONAL_LINE] ) {
			plyPropSize          = sectionProps[PLY_POLYGONAL_LINE].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_POLYGONAL_LINE].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_POLYGONAL_LINE].propertyListDataType.begin();
			PrimitiveInfo primInfo;
			for(const ePlyProperties currProperty : sectionProps[PLY_POLYGONAL_LINE].propertyType) {
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
						rMeshSeed.getPolyLabelIDRef().push_back(labelID);
						} break;
					case PLY_LIST_VERTEX_INDICES: {
						// Read a list of polyline indices
						int nrIndices;
						READ_IN_PROPER_BYTE_ORDER( filestr, &nrIndices, (*plyPropListCountSize), reverseByteOrder );
						std::vector<int>* somePolyLinesIndices = new std::vector<int>;
						for( int j=0; j<nrIndices; j++ ) {
							int vertIndex;
							READ_IN_PROPER_BYTE_ORDER( filestr, &vertIndex, (*plyPropListSize), reverseByteOrder );
							somePolyLinesIndices->push_back( vertIndex );
						}
						rMeshSeed.getPolyLineVertIndicesRef().push_back( somePolyLinesIndices );
						} break;
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_TEXCOORDS:
					case PLY_LIST_IGNORE:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
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
			rMeshSeed.getPolyPrimInfoRef().push_back( primInfo );
			polyLinesRead++;
			//------------------------------------------- PARSE UNSUPPORTED -------------------------------------------
		} else if( unsupportedRead < plyElements[PLY_SECTION_UNSUPPORTED] ) {
			plyPropSize          = sectionProps[PLY_SECTION_UNSUPPORTED].propertyDataType.begin();
			plyPropListCountSize = sectionProps[PLY_SECTION_UNSUPPORTED].propertyListCountDataType.begin();
			plyPropListSize      = sectionProps[PLY_SECTION_UNSUPPORTED].propertyListDataType.begin();
			for(const ePlyProperties currProperty : sectionProps[PLY_SECTION_UNSUPPORTED].propertyType) {
					switch( currProperty ) {
					case PLY_LIST_IGNORE:
					case PLY_LIST_FEATURE_VECTOR:
					case PLY_LIST_VERTEX_INDICES:
					case PLY_LIST_TEXCOORDS:
						READ_IN_PROPER_BYTE_ORDER( filestr, &listNrChar, (*plyPropListCountSize), reverseByteOrder );
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

	if(hasVertexTexCoords && !vertexTextureCoordinates.empty())
	{
		copyVertexTexCoordsToFaces(vertexTextureCoordinates, rFaceProps);
	}

	LOG::info() << "[PlyReader::" << __FUNCTION__ << "] PLY comment lines: " << linesComment << "\n";
	LOG::info() << "[PlyReader::" << __FUNCTION__ << "] PLY ignored:       " << bytesIgnored << " Byte. (one is OK as it is the EOF byte)\n";
	LOG::info() << "[PlyReader::" << __FUNCTION__ << "] PLY Extra ignored: " << extraBytesIgnored << " Byte. (one is OK as it is the EOF byte)\n";
	LOG::info() << "[PlyReader::" << __FUNCTION__ << "] PLY:               " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds.\n";

	return( true );
}

void PlyReader::setIsBigEndian(bool bigEndian)
{
	mSystemIsBigEndian = bigEndian;
}



//! Maps a parsed string from a PLY-header to a data-type.
//!
//! see also PLY-spec:
//! [Dead link!] http://www.cs.kuleuven.ac.be/~ares/libply/ply-0.1/doc/PLY_FILES.txt
//! working alternative: https://people.sc.fsu.edu/~jburkardt/data/ply/ply.html
ePlyPropertySize plyParseTypeStr( char* propType ) {
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
		LOG::debug() << "[PlyReader::" << __FUNCTION__ << "] assuming 'uint' having 4 byte.\n";
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
		LOG::warn() << "[PlyReader::" << __FUNCTION__ << "] unknown type '" << propType << "' in header!\n";
		propertyDataType = PLY_SIZE_UNDEF;
	}
	return propertyDataType;
}
