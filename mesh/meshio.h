#ifndef MESHIO_H
#define MESHIO_H

#include "meshseedext.h"

//!
//! \brief Class for handling file access. (Layer 0)
//!
//! File access (for meshes). It is also a parent of Mesh.
//!
//! Layer 0
//!

class MeshIO : public MeshSeedExt {

	public:
		MeshIO();
		~MeshIO();

		//! Simple struct for parsing and passing points and vectors in R3
		struct grVector3ID {
			int    mId;  //! Unique identifier
			double mX;   //! x-coordinate
			double mY;   //! y-coordinate
			double mZ;   //! z-coordinate
		};

		enum eExportFlags {
			EXPORT_BINARY,      //!< Binary or ASCII for those available in both formats.
			EXPORT_VERT_COLOR,  //!< Export color per vertex.
			EXPORT_VERT_NORMAL, //!< Export normal per vertex.
			EXPORT_VERT_FLAGS,  //!< Flags (bitarray) per vertex.
			EXPORT_VERT_LABEL,  //!< Label ID of the vertex.
			EXPORT_VERT_FTVEC,  //!< Feature vector per vertex, when present.
			EXPORT_POLYLINE,    //!< Export polylines.
			EXPORT_FLAG_COUNT   //!< Number of flags available for export.
		};

		// Read:
		virtual bool readFile( const std::string& rFileName, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );
		virtual bool readIsRegularGrid( bool* rIsGrid );

	private:
		        bool readOBJ( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );
		        bool readTXT( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps ); // also used for file with extension .xyz - as there are basically the same
		        bool readTXTRegularGrid( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );
		        bool readPLY( const std::string& rFilename, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );

	public:
		        bool systemTestFloatDotColon( bool* rDotUsed );
				bool importTEXMap( const std::string& rFileName, int* rNrLines, uint64_t** rRefToPrimitves, unsigned char** rTexMap );
		        bool importNormals( const std::string& rFileName, std::vector<grVector3ID> *rNormals );

		// Write:
		virtual bool setFlagExport( eExportFlags rFlag, bool rSetTo  );
		virtual bool writeFileUserInteract();
		virtual bool writeFile( const std::string& rFileName,
			                std::vector<sVertexProperties>& rVertexProps,
			                std::vector<sFaceProperties>& rFaceProps );

	private:
		// Internal write methods.
		bool writeOBJ(  const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps );
		bool writePLY(  const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps );
		bool writeVRML( const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps );
		bool writeTXT(  const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps );

	public:
		// Meta-Data as strings
		enum eMetaStrings {
			META_MODEL_ID,         //!< Given Id. Typically an inventory number.
			META_MODEL_MATERIAL,   //!< Material of the real world object. e.g. 'original, clay' for 3D-scans of cuneiform tablets.
			META_REFERENCE_WEB,    //!< Http URL with name format style '[URL|Name}'.
			META_FILENAME,         //!< Filename, when loaded.
			META_STRINGS_COUNT,    //!< Total number of strings for meta-data.
		};
		        bool         setModelMetaString( eMetaStrings rMetaStrID, const std::string& rModelMeta );
		        std::string  getModelMetaString( eMetaStrings rMetaStrID );
		        bool         getModelMetaStringName( eMetaStrings rMetaStrID, std::string& rModelMetaStringName );
		        bool         getModelMetaStringId( const std::string& rModelMetaStringName, eMetaStrings& rMetaStrID );
		        bool         clearModelMetaStrings();

	public:
		// Provide Information:
		virtual std::string getFileExtension();
		virtual std::string getBaseName();
		virtual std::string getFileLocation();
		virtual std::string getFullName();

		virtual bool writeIcoNormalSphereData(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, int subdivisions, bool sphereCoordinates = false);
	private:
		enum ePlySections {
			PLY_VERTEX,
			PLY_FACE,
			PLY_POLYGONAL_LINE,
			PLY_SECTION_UNSUPPORTED,
			PLY_SECTIONS_COUNT
		};

		enum ePlyProperties {
			PLY_UNSUPPORTED,
			PLY_COORD_X,
			PLY_COORD_Y,
			PLY_COORD_Z,
			PLY_COLOR_RED,
			PLY_COLOR_GREEN,
			PLY_COLOR_BLUE,
			PLY_COLOR_ALPHA,
			PLY_FLAGS,
			PLY_VERTEX_QUALITY,
			PLY_VERTEX_INDEX,
			PLY_LABEL,
			PLY_VERTEX_NORMAL_X,
			PLY_VERTEX_NORMAL_Y,
			PLY_VERTEX_NORMAL_Z,
			PLY_LIST_IGNORE,
			PLY_LIST_FEATURE_VECTOR,
			PLY_LIST_VERTEX_INDICES,
			PLY_PROPERTIES_COUNT
		};

		struct plyContainer {
			std::vector<MeshIO::ePlyProperties> propertyType;
			std::vector<int> propertyDataType;
			std::vector<int> propertyListCountDataType;
			std::vector<int> propertyListDataType;
		};

		// from PLY-spec ( http://www.cs.kuleuven.ac.be/~ares/libply/ply-0.1/doc/PLY_FILES.txt )
		//--------------------------------------------------------------------------------------
		// name        type        number of bytes
		// ---------------------------------------
		// int8       character                 1
		// uint8      unsigned character        1
		// int16      short integer             2
		// uint16     unsigned short integer    2
		// int32      integer                   4
		// uint32     unsigned integer          4
		// float32    single-precision float    4
		// float64    double-precision float    8

		enum ePlyPropertySize {
			PLY_SIZE_UNDEF = -1,
			PLY_INT8       =  1,
			PLY_UINT8      =  1,
			PLY_SHORT_INT  =  2,
			PLY_USHORT_INT =  2,
			PLY_INT32      =  4,
			PLY_UINT32     =  4,
			PLY_FLOAT32    =  4,
			PLY_FLOAT64    =  8,
			PLY_FLOAT      =  4, // seems not to be from the official spec
			PLY_CHAR       =  1, // seems not to be from the official spec
			PLY_UCHAR      =  1, // seems not to be from the official spec
		};
		ePlyPropertySize plyParseTypeStr( char* propType );

		enum eTxtRegularTags {
			TXT_REGULAR_IGNORE,  //!< Tags for tags to be ignored
			TXT_REGULAR_X,       //!< Tag for x-coordinate
			TXT_REGULAR_Y,       //!< Tag for y-coordinate
			TXT_REGULAR_Z,       //!< Tag for z-coordinate
			TXT_REGULAR_UNKNOWN, //!< Tag for unknown tags.
			TXT_REGULAR_COUNT
		};

	private:
		bool   mExportFlags[EXPORT_FLAG_COUNT]; //!< Handles export options.
		bool   mSystemIsBigEndian; //!< Flag for proper Byte ordering during write/read.

		// File properties
		std::string mFileNameFull;      //!< Full name and path of the current file.

		// Meta-Data NEW with vector of strings
		std::string mMetaDataStrings[META_STRINGS_COUNT];         //!< Meta-Data contents
		std::string mMetaDataStringNames[META_STRINGS_COUNT];     //!< Meta-Data names

};
#endif
