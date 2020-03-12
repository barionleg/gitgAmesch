#include <GigaMesh/mesh/MeshIO/ModelMetaData.h>
#include <iostream>

//! Set Meta-Data as strings.
//! @returns false in case of an error. True otherwise.
ModelMetaData::ModelMetaData()
{
	clearModelMetaStrings();
}

bool ModelMetaData::setModelMetaString(
				eMetaStrings       rMetaStrID,        //!< Id of the meta-data string.
				const std::string& rModelMeta         //!< Meta-data content as string.
) {
	mMetaDataStrings[rMetaStrID] = rModelMeta;
	return( true );
}

//! Get Meta-Data as strings.
//! @returns nullptr case of an error.
std::string ModelMetaData::getModelMetaString(
				eMetaStrings rMetaStrID               //!< Id of the meta-data string.
) const 
{
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[ModelMetaData::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no content!\n" << std::endl;
		return std::string();
	}
	return( mMetaDataStrings[rMetaStrID] );
}

//! Fetch the name of the Meta-Data strings using an Id.
//! @returns false in case of an error. True otherwise.
bool ModelMetaData::getModelMetaStringName(
				eMetaStrings rMetaStrID,                   //!< Id of the meta-data string.
				std::string& rModelMetaStringName          //!< Name as string of the meta-data string.
) const 
{
	if( rMetaStrID == META_STRINGS_COUNT ) {
		std::cerr << "[ModelMetaData::" << __FUNCTION__ << "] ERROR: Id META_STRINGS_COUNT has no name!" << std::endl;
		return( false );
	}
	rModelMetaStringName = mMetaDataStringNames[rMetaStrID];
	return( true );
}

//! Fetch the Id of the Meta-Data strings using its name as string.
//! @returns false in case of an error or no Id found. True otherwise.
bool ModelMetaData::getModelMetaStringId(
				const std::string& rModelMetaStringName,   //!< Id of the meta-data string.
				eMetaStrings& rMetaStrID                   //!< Name as string of the meta-data string.
) const 
{
	for( unsigned i=0; i<META_STRINGS_COUNT ; i++ ) {
		if( rModelMetaStringName == mMetaDataStringNames[i]) {
			rMetaStrID = static_cast<eMetaStrings>( i );
			return( true );
		}
	}
	// Nothing found
	return( false );
}

//! Clear and reset meta-data.
//! @returns false in case of an error or no Id found. True otherwise.
bool ModelMetaData::clearModelMetaStrings() {
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
	mMetaDataStringNames[META_TEXTUREFILE]      = "TextureFile";

	mFileName = "ModelFileName";
	// Done.
	return( true );
}

bool ModelMetaData::hasTextureCoordinates() const
{
	return mHasTextureCoordinates;
}

void ModelMetaData::setHasTextureCoordinates(bool hasTextureCoordinates)
{
	mHasTextureCoordinates = hasTextureCoordinates;
}

TextureHandle ModelMetaData::addTextureName(const std::filesystem::path& textureName)
{
	for(size_t i = 0; i<mTextureFiles.size(); ++i)
	{
		if(mTextureFiles[i] == textureName)
			return i;
	}

	mTextureFiles.push_back(textureName);
	return mTextureFiles.size() - 1;
}

std::filesystem::path ModelMetaData::getTextureName(TextureHandle id) const
{
	if(id < mTextureFiles.size())
		return mTextureFiles[id];

	return std::filesystem::path();
}

bool ModelMetaData::hasTextureFiles() const
{
	return !mTextureFiles.empty();
}

std::vector<std::filesystem::path>& ModelMetaData::getTexturefilesRef()
{
	return mTextureFiles;
}

std::filesystem::path ModelMetaData::getFileName() const
{
	return mFileName;
}

void ModelMetaData::setFileName(const std::filesystem::path& fileName)
{
	mFileName = fileName;
}
