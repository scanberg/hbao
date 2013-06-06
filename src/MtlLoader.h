#pragma once

#include <string>
#include <vector>

class Material
{
public:
	typedef struct
	{
		float r,g,b;
	}sColor;

	float specularExponent;
	
	sColor ambientColor;
	sColor diffuseColor;
	sColor specularColor;
	sColor emissiveColor;

	std::string ambientMap;
	std::string diffuseMap;
	std::string specularMap;
	std::string normalMap;
	std::string maskMap;

	std::string name;
};

std::vector<Material> loadMaterialsFromMtl(const char *filename);