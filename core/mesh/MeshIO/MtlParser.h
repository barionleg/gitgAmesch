#ifndef MTLPARSER_H
#define MTLPARSER_H

#include <string>
#include <unordered_map>
#include <array>
#include <filesystem>

struct MtlMaterial {
	//color and illumination
	std::array<float,3> Ka = {0.0F,0.0F,0.0F};
	std::array<float,3> Kd = {0.0F,0.0F,0.0F};
	std::array<float,3> Ks = {0.0F,0.0F,0.0F};
	std::array<float,3> Tf = {0.0F,0.0F,0.0F};

	float d                = 0.0F;
	float Ns               = 0.0F;
	float sharpness        = 0.0F;
	float Ni               = 0.0F;

	//name
	std::string name;

	//Texture map Statements
	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;
	std::string map_Ns;
	std::string map_d;
	std::string disp;
	std::string decal;
	std::string bump;

	//isHalo and illumination, moved here for struct-packing reasons
	bool dIsHalo            = false;
	unsigned char illumModel= 0;
};



class MtlParser
{
	public:
		MtlParser() = default;

		bool parseFile(const std::filesystem::path& fileName);
		bool hasMaterial(const std::string& name);

		MtlMaterial& getMaterial(const std::string& name);
		std::unordered_map<std::string, MtlMaterial>& getMaterialsRef();

	private:
		std::unordered_map<std::string, MtlMaterial> mMtlMaterials;
};

#endif // MTLPARSER_H
