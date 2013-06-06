#pragma once

#include <cstdio>
#include <vector>
#include <string>

class Mesh
{
	public:
	typedef struct
	{
		float x,y,z;
		float nx,ny,nz;
		float u,v;
	}sVertex;

	typedef struct
	{
		unsigned int v[3];
	}sFace;

	std::vector<sVertex> vertices;
	std::vector<sFace> faces;

	std::string name;
	std::string material;
};

Mesh loadMeshFromObj(const char* filename, float scale = 1.0f);
std::vector<Mesh> loadMeshesFromObj(const char* filename, float scale = 1.0f);