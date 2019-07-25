#include "MeshWriter.h"

MeshWriter::MeshWriter()
{

}

void MeshWriter::setModelMetaData(const ModelMetaData& metaData)
{
	mModelMetaData = metaData;
}

ModelMetaData& MeshWriter::getModelMetaDataRef()
{
	return mModelMetaData;
}

void MeshWriter::setExportVertColor(bool exportVertColor)
{
	mExportVertColor = exportVertColor;
}

void MeshWriter::setExportVertFeatureVector(bool exportVertFeatureVector)
{
	mExportVertFeatureVector = exportVertFeatureVector;
}

void MeshWriter::setExportBinary(bool exportBinary)
{
	mExportBinary = exportBinary;
}

void MeshWriter::setIsBigEndian(bool isBigEndian)
{
	mIsBigEndian = isBigEndian;
}

void MeshWriter::setExportVertNormal(bool exportVertNormal)
{
	mExportVertNormal = exportVertNormal;
}

void MeshWriter::setExportVertLabel(bool exportVertLabel)
{
	mExportVertLabel = exportVertLabel;
}

void MeshWriter::setExportPolyline(bool exportPolyline)
{
	mExportPolyline = exportPolyline;
}

void MeshWriter::setExportTextureCoordinates(bool exportTextureCoordinates)
{
	mExportTextureCoordinates = exportTextureCoordinates;
}

void MeshWriter::setExportVertFlags(bool exportVertFlags)
{
	mExportVertFlags = exportVertFlags;
}
