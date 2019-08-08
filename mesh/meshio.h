#ifndef MESHIO_H
#define MESHIO_H

#include "meshseedext.h"
#include "MeshIO/ModelMetaData.h"

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
			EXPORT_TEXTURE_COORDINATES, //!< Export per face texture coordinates
			EXPORT_FLAG_COUNT   //!< Number of flags available for export.
		};

		// Read:
		virtual bool readFile( const std::string& rFileName, std::vector<sVertexProperties>& rVertexProps, std::vector<sFaceProperties>& rFaceProps );
		virtual bool readIsRegularGrid( bool* rIsGrid );


		bool importTEXMap( const std::string& rFileName, int* rNrLines, uint64_t** rRefToPrimitves, unsigned char** rTexMap );
		bool importNormals( const std::string& rFileName, std::vector<grVector3ID> *rNormals );

		// Write:
		virtual bool setFlagExport( eExportFlags rFlag, bool rSetTo  );
		virtual bool writeFileUserInteract();
		virtual bool writeFile( const std::string& rFileName,
			                std::vector<sVertexProperties>& rVertexProps,
			                std::vector<sFaceProperties>& rFaceProps );

		ModelMetaData& getModelMetaDataRef();

		// Provide Information:
		virtual std::string getFileExtension();
		virtual std::string getBaseName();
		virtual std::string getFileLocation();
		virtual std::string getFullName();

		virtual bool writeIcoNormalSphereData(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, int subdivisions, bool sphereCoordinates = false);

	private:
		std::array<bool, EXPORT_FLAG_COUNT>   mExportFlags; //!< Handles export options.
		bool   mSystemIsBigEndian; //!< Flag for proper Byte ordering during write/read.

		// File properties
		std::string mFileNameFull;      //!< Full name and path of the current file.

		// Meta-Data NEW with vector of strings
		ModelMetaData mModelMetaData;

};
#endif
