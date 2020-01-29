#ifndef MESHWRITER_H
#define MESHWRITER_H

#include <string>
#include <vector>
#include "../meshseedext.h"
#include "ModelMetaData.h"

class MeshWriter
{
	public:
		MeshWriter();
		virtual ~MeshWriter() = default;
		virtual bool writeFile(const std::string& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) = 0;

		void setModelMetaData(const ModelMetaData& metaData);
		ModelMetaData& getModelMetaDataRef();

		void setExportVertColor(bool exportVertColor);
		void setExportVertFeatureVector(bool exportVertFeatureVector);
		void setExportBinary(bool exportBinary);
		void setIsBigEndian(bool isBigEndian);
		void setExportVertNormal(bool exportVertNormal);
		void setExportVertLabel(bool exportVertLabel);
		void setExportPolyline(bool exportPolyline);
		void setExportTextureCoordinates(bool exportTextureCoordinates);
		void setExportVertFlags(bool exportVertFlags);

	private:
		ModelMetaData mModelMetaData;

	protected:
		bool mExportVertColor = false;
		bool mExportVertFeatureVector = false;
		bool mExportBinary = false;
		bool mIsBigEndian = false;
		bool mExportVertFlags = false;
		bool mExportVertNormal = false;
		bool mExportVertLabel = false;
		bool mExportPolyline = false;
		bool mExportTextureCoordinates = false;
};

#endif // MESHWRITER_H
