#include "MtlLoader.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>

void readFloat(const std::string &line, float &val);
void readColor(const std::string &line, Material::sColor &val);
void readString(const std::string &line, std::string &val);

std::vector<Material> loadMaterialsFromMtl(const char *filename)
{
	printf("Attempting to load materials from %s\n", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	std::vector<Material> materials;

	if(filehandle.fail())
	{
		printf("Could not open file.\n");
		return materials;
	}

	int success;
	int count=0;

	std::string line;
	char str [256];

	clock_t start, end;
	start = clock();

	printf("Reading data... ");

	Material mat;

	while( filehandle.good() && !filehandle.eof() )
	{
		std::getline(filehandle, line);
		success = sscanf(line.c_str(), "%s ", str);
		if(success)
		{
			if(strcmp(str, "newmtl") == 0)
			{
				if(count > 0)
					materials.push_back(mat);

				mat = Material();
				readString(line, mat.name);
				++count;
			}
			else if(strcmp(str, "Ns") == 0)
				readFloat(line, mat.specularExponent);
			else if(strcmp(str, "Ka") == 0)
				readColor(line, mat.ambientColor);
			else if(strcmp(str, "Kd") == 0)
				readColor(line, mat.diffuseColor);
			else if(strcmp(str, "Ks") == 0)
				readColor(line, mat.specularColor);
			else if(strcmp(str, "Ke") == 0)
				readColor(line, mat.emissiveColor);
			else if(strcmp(str, "map_Ka") == 0)
				readString(line, mat.ambientMap);
			else if(strcmp(str, "map_Kd") == 0)
				readString(line, mat.diffuseMap);
			else if(strcmp(str, "map_bump") == 0)
				readString(line, mat.normalMap);
			else if(strcmp(str, "map_d") == 0)
				readString(line, mat.maskMap);
		}
	}

	if(count>0)
		materials.push_back(mat);

	printf("done!\n");

	printf("total material count %i\n", materials.size());

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken %3.3fs \n", cpu_time_used);

	return materials;
}

void readFloat(const std::string &line, float &val)
{
	sscanf(line.c_str(), "%*s %f", &val);
}

void readColor(const std::string &line, Material::sColor &val)
{
	sscanf(line.c_str(), "%*s %f %f %f", &val.r, &val.g, &val.b);
}

void readString(const std::string &line, std::string &val)
{
	char str [256];
	sscanf(line.c_str(), "%*s %s", str);
	val = std::string(str);
}