#ifndef MESHINFODATA_H
#define MESHINFODATA_H

#include <string>

class MeshInfoData {
	public:
		MeshInfoData();
		~MeshInfoData();

	public:
		enum eMeshPropertyString {
			FILENAME,                      //!< Filename with path and extension.
			MODEL_ID,                      //!< Meta-data: Id of the object e.g. inventory number.
			MODEL_MATERIAL,                //!< Meta-data: Material(s) of the acquired object e.g. clay, original or gypsum, cast copy.
			MODEL_WEBREFERENCE,            //!< Meta-data: Web-Reference e.g. CDLI url for a cuneiform tablet.
			STRING_COUNT                   //!< Number of elements.
		};
	public:
		std::string mStrings[STRING_COUNT];
	private:
		std::string mStringName[STRING_COUNT];

	public:
		enum eMeshPropertyULongCount {
			VERTICES_TOTAL,                //!< Number of vertices.
			VERTICES_NAN,
			VERTICES_NORMAL_LEN_NORMAL,
			VERTICES_SOLO,
			VERTICES_POLYLINE,
			VERTICES_BORDER,
			VERTICES_NONMANIFOLD,
			VERTICES_ON_INVERTED_EDGE,
			VERTICES_PART_OF_ZERO_FACE,
			VERTICES_SYNTHETIC,
			VERTICES_MANUAL,
			VERTICES_CIRCLE_CENTER,
			VERTICES_SELECTED,
			VERTICES_FUNCVAL_FINITE,
			VERTICES_FUNCVAL_LOCAL_MIN,
			VERTICES_FUNCVAL_LOCAL_MAX,
			FACES_TOTAL,                   //!< Number of faces.
			FACES_SOLO,
			FACES_BORDER,
			FACES_BORDER_THREE_VERTICES,
			FACES_BORDER_BRDIGE_TRICONN,
			FACES_BORDER_BRDIGE,
			FACES_BORDER_DANGLING,
			FACES_MANIFOLD,
			FACES_NONMANIFOLD,
			FACES_STICKY,
			FACES_ZEROAREA,
			FACES_INVERTED,
			FACES_SELECTED,
			FACES_WITH_SYNTH_VERTICES,     //!< Number of faces having only synthetic vertices i.e. all three vertices are synthetic.
			ULONG_COUNT,                   //!< Number of elements.
		};
	public:
		unsigned long mCountULong[ULONG_COUNT];
	private:
		std::string mCountULongName[ULONG_COUNT];

	public:
		enum eMeshPropertyDouble {
			BOUNDINGBOX_MIN_X,                  //!< Minimum coordinate in x.
			BOUNDINGBOX_MIN_Y,                  //!< Minimum coordinate in y.
			BOUNDINGBOX_MIN_Z,                  //!< Minimum coordinate in z.
			BOUNDINGBOX_MAX_X,                  //!< Maximum coordinate in x.
			BOUNDINGBOX_MAX_Y,                  //!< Maximum coordinate in y.
			BOUNDINGBOX_MAX_Z,                  //!< Maximum coordinate in z.
			BOUNDINGBOX_WIDTH,                  //!< Bounding box width.
			BOUNDINGBOX_HEIGHT,                 //!< Bounding box height.
			BOUNDINGBOX_THICK,                  //!< Bounding box thickness.
			TOTAL_AREA,                         //!< Total area.
			TOTAL_VOLUME_DX,                    //!< Total volume in dx.
			TOTAL_VOLUME_DY,                    //!< Total volume in dy.
			TOTAL_VOLUME_DZ,                    //!< Total volume in dz.
			DOUBLE_COUNT                        //!< Number of elements.
		};
	public:
		double mCountDouble[DOUBLE_COUNT];
	private:
		std::string mmCountDoubleName[DOUBLE_COUNT];

	public:
		void reset();
		bool getMeshInfoHTML( std::string& rInfoHTML );

		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyString     rPropId, std::string& rPropName );
		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyULongCount rPropId, std::string& rPropName );
		bool getMeshInfoPropertyName( const MeshInfoData::eMeshPropertyDouble     rPropId, std::string& rPropName );
};

#endif // MESHINFODATA_H
