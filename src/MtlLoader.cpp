#include "MtlLoader.h"

#include <cstdio>
#include <ctime>
#include <fstream>

void readFloat(const std::string &line, float &val);
void readColor(const std::string &line, sColor &val);
void readString(const std::string &line, std::string &val);

std::vector<Material> loadMaterialFromMtl(const char *filename)
{
	printf("Attempting to load materials from %s\n", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	if(filehandle.fail())
	{
		printf("Could not open file.\n");
		return Mesh();
	}

	std::vector<Material> materials;
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
			if(strcmp(str, "newmtl"))
			{
				if(count > 0)
					materials.push_back(mat);

				readString(mat.name);
				++count;
			}
			else if(strcmp(str, "Ns"))
				readFloat(line, mat.specularExponent);
			else if(strcmp(str, "Ka"))
				readColor(line, mat.ambientColor);
			else if(strcmp(str, "Kd"))
				readColor(line, mat.diffuseColor);
			else if(strcmp(str, "Ks"))
				readColor(line, mat.specularColor);
			else if(strcmp(str, "Ke"))
				readColor(line, mat.emissiveColor);
			else if(strcmp(str, "map_Ka"))
				readString(line, mat.ambientMap);
			else if(strcmp(str, "map_Kd"))
				readString(line, mat.diffuseMap);
			else if(strcmp(str, "map_bump"))
				readString(line, mat.normalMap);
			else if(strcmp(str, "map_d"))
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

void readColor(const std::string &line, sVec3 &val)
{
	sscanf(line.c_str(), "%*s %f %f %f", &val.r, &val.g, &val.b);
}

void readString(const std::string &line, std::string &val)
{
	char str [256];
	sscanf(line.c_str(), "%*s %s", str);
	val = std::string(str);
}