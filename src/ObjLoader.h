#pragma once

#include <cstdio>

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

	unsigned int numVertices;
	unsigned int numFaces;
	sVertex * vertices;
	sFace * faces;

	Mesh() :
		numVertices(0),
		numFaces(0),
		vertices(NULL),
		faces(NULL)
	{}

	~Mesh()
	{
		if(vertices)
			delete[] vertices;

		if(faces)
			delete[] faces;
	}
};

bool loadMeshFromObj(const char* filename, Mesh *mesh, float scale = 1.0f);