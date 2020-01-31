#ifndef MODELMETADATA_H
#define MODELMETADATA_H

#include <string>
#include <array>
#include <vector>

using TextureHandle = unsigned char;

class ModelMetaData
{
	public:
		ModelMetaData();

		// Meta-Data as strings
		enum eMetaStrings {
			META_MODEL_ID,         //!< Given Id. Typically an inventory number.
			META_MODEL_MATERIAL,   //!< Material of the real world object. e.g. 'original, clay' for 3D-scans of cuneiform tablets.
			META_REFERENCE_WEB,    //!< Http URL with name format style '[URL|Name}'.
			META_FILENAME,         //!< Filename, when loaded.
			META_TEXTUREFILE,      //!< Meshlab texturefile stored in ply: e.g. "comment TextureFile texture.png"
			META_STRINGS_COUNT,    //!< Total number of strings for meta-data.
		};

		bool         setModelMetaString( eMetaStrings rMetaStrID, const std::string& rModelMeta );
		std::string  getModelMetaString( eMetaStrings rMetaStrID );
		bool         getModelMetaStringName( eMetaStrings rMetaStrID, std::string& rModelMetaStringName );
		bool         getModelMetaStringId( const std::string& rModelMetaStringName, eMetaStrings& rMetaStrID );
		bool         clearModelMetaStrings();

		[[nodiscard]] bool hasTextureCoordinates() const;
		void setHasTextureCoordinates(bool hasTextureCoordinates);


		TextureHandle addTextureName(const std::string& textureName);
		[[nodiscard]] std::string getTextureName(TextureHandle id) const;
		[[nodiscard]] bool hasTextureFiles() const;
		std::vector<std::string>& getTexturefilesRef();

	private:
		std::array<std::string, META_STRINGS_COUNT> mMetaDataStrings;         //!< Meta-Data contents
		std::array<std::string, META_STRINGS_COUNT> mMetaDataStringNames;     //!< Meta-Data names

		std::vector<std::string> mTextureFiles;                               //!< Stores referenced texture-files
		bool mHasTextureCoordinates = false;                                  //!< Stores if the mesh has texture-coordinates
};

#endif // MODELMETADATA_H