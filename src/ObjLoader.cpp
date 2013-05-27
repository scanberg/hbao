/**
Copyright (C) 2012-2014 Robin Skånberg

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ObjLoader.h"

#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <vector>
#include <list>

typedef struct
{
	int position;
	int normal;
	int texcoord;
	int sg;
	unsigned int index;
}sVertexIndex;

typedef struct
{
	float x,y,z;
}sVec3;

typedef struct
{
	float x,y;
}sVec2;

bool equal(const sVertexIndex &lhs, const sVertexIndex &rhs);

unsigned int insertVertexIndex(	sVertexIndex vertex,
								std::vector<sVertexIndex> &table,
								std::vector<std::list<sVertexIndex> > &existingTable);

void readPosition(	const std::string &line,
					std::vector<sVec3> &table,
					float scale);

void readNormal(	const std::string &line,
					std::vector<sVec3> &table );

void readTexcoord(	const std::string &line,
					std::vector<sVec2> &table );

void readFace(		const std::string &line,
					std::vector<sVertexIndex> &vertexTable,
					std::vector<std::list<sVertexIndex> > &existingTable,
					std::vector<Mesh::sFace> &faceTable,
					int sg);

void readSG(		const std::string &line,
					int &sg);

void readG(			const std::string &line,
					std::string &name);

Mesh createMesh(	const std::string &name,
					std::vector<sVertexIndex> &vertexTable,
					std::vector<Mesh::sFace> &faceTable,
					std::vector<sVec3> &positionTable,
					std::vector<sVec3> &normalTable,
					std::vector<sVec2> &texcoordTable);

Mesh loadMeshFromObj(const char *filename, float scale)
{
	printf("Attempting to load mesh->from %s\n", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	if(filehandle.fail())
	{
		printf("Could not open file.\n");
		return Mesh();
	}

	std::vector<std::list<sVertexIndex> > existingVertexTable;

	std::vector<sVertexIndex>	vertexTable;
	std::vector<Mesh::sFace>	faceTable;

	std::vector<sVec3> positionTable;
	std::vector<sVec3> normalTable;
	std::vector<sVec2> texcoordTable;

	std::string line;
	std::string name(filename);
	int sg = 0;

	clock_t start, end;
	start = clock();

	printf("Reading data... ");

	while( filehandle.good() && !filehandle.eof() )
	{
		std::getline(filehandle, line);
		if(line[0] == 'v')
		{
			if(line[1] == 't')
				readTexcoord(line, texcoordTable);
			else if(line[1] == 'n')
				readNormal(line, normalTable);
			else
				readPosition(line, positionTable, scale);
		}
		else if(line[0] == 'f')
			readFace(line, vertexTable, existingVertexTable, faceTable, sg);
		else if(line[0] == 's')
			readSG(line, sg);
	}

	Mesh m = createMesh(name, vertexTable, faceTable,
						positionTable, normalTable, texcoordTable);

	printf("done!\n");

	printf("total vertex count %i\n", vertexTable.size());
	printf("total face count %i\n", faceTable.size());

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken %3.3fs \n", cpu_time_used);

	return m;
}

std::vector<Mesh> loadMeshesFromObj(const char *filename, float scale)
{
	printf("Attempting to load mesh->from %s\n", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	std::vector<Mesh> meshes;

	if(filehandle.fail())
	{
		printf("Could not open file.\n");
		return meshes;
	}

	std::vector<std::list<sVertexIndex> > existingVertexTable;

	std::vector<sVertexIndex>	vertexTable;
	std::vector<Mesh::sFace>	faceTable;

	std::vector<sVec3> positionTable;
	std::vector<sVec3> normalTable;
	std::vector<sVec2> texcoordTable;

	std::string line;
	std::string name;
	int sg = 0;
	int count = 0;

	clock_t start, end;
	start = clock();

	printf("Reading data... ");

	while( filehandle.good() && !filehandle.eof() )
	{
		std::getline(filehandle, line);
		if(line[0] == 'v')
		{
			if(line[1] == 't')
				readTexcoord(line, texcoordTable);
			else if(line[1] == 'n')
				readNormal(line, normalTable);
			else
				readPosition(line, positionTable, scale);
		}
		else if(line[0] == 'f')
			readFace(line, vertexTable, existingVertexTable, faceTable, sg);
		else if(line[0] == 's')
			readSG(line, sg);
		else if(line[0] == 'g')
		{
			if(count > 0)
			{
				Mesh m = createMesh( name, vertexTable, faceTable,
								positionTable, normalTable, texcoordTable);
				meshes.push_back(m);
			}
			readG(line, name);
			printf("new group %s\n", name.c_str());
			++count;
		}
	}

	if(count > 0)
	{
		Mesh m = createMesh(name, vertexTable, faceTable,
							positionTable, normalTable, texcoordTable);
		meshes.push_back(m);
	}

	printf("done!\n");

	printf("total vertex count %i\n", vertexTable.size());
	printf("total face count %i\n", faceTable.size());

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken %3.3fs \n", cpu_time_used);

    //printf("meshes.size() = %i\n", meshes.size());

	return meshes;
}

bool equal(const sVertexIndex &lhs, const sVertexIndex &rhs)
{
	return 	lhs.position == rhs.position &&
			lhs.normal == rhs.normal &&
			lhs.texcoord == rhs.texcoord &&
			lhs.sg == rhs.sg;
}

unsigned int insertVertexIndex(	sVertexIndex vertex,
								std::vector<sVertexIndex> &table,
								std::vector<std::list<sVertexIndex> > &existingTable)
{
	if(	vertex.position == -1 )
	{
		printf("Bad vertex given: v %i %i %i \n",
			vertex.position, vertex.normal, vertex.texcoord);

		return 0;
	}

	vertex.index = table.size();

	// Check against existing vertices,
	// Uses a hashtable approach with vertexIndex as the
	// hashfunction, and variations of this vertexIndex is
	// stored as a linked list in this slot.

	// Is the 'hashindex' greater than the size of the table,
	// then expand the table.
	if( vertex.position >= (int)existingTable.size() )
	{
		existingTable.resize(vertex.position+1, std::list<sVertexIndex>());
		existingTable[vertex.position].push_back(vertex);
	}
	else
	{
		// Check if the vertex already exists in the list
		std::list<sVertexIndex>::const_iterator it = existingTable[vertex.position].begin();
		while(it != existingTable[vertex.position].end())
		{
			if(equal(vertex, *it))
				return it->index;
			++it;
		}
		existingTable[vertex.position].push_back(vertex);
	}

/*	
	// Brute force solution to check for duplicates 
	for(unsigned int i=0; i<table.size(); ++i)
	{
		if(equal(vertex, table[i]))
			return i;
	}
*/

	// No vertex was found, insert and return the index.
	table.push_back(vertex);
	return vertex.index;
}

void readPosition(	const std::string &line,
					std::vector<sVec3> &table,
					float scale )
{
	sVec3 pos;

	// v 1.0 2.0 3.0
	sscanf(line.c_str(), "%*s %f %f %f", &pos.x, &pos.y, &pos.z);
	pos.x *= scale;
	pos.y *= scale;
	pos.z *= scale;
	table.push_back(pos);
}

void readNormal(const std::string &line,
				std::vector<sVec3> &table )
{
	sVec3 norm;

	// vn 1.0 2.0 3.0
	sscanf(line.c_str(), "%*s %f %f %f", &norm.x, &norm.y, &norm.z);
	table.push_back(norm);
}

void readTexcoord(	const std::string &line,
					std::vector<sVec2> &table )
{
	sVec2 texcoord;

	// vt 1.0 2.0
	sscanf(line.c_str(), "%*s %f %f", &texcoord.x, &texcoord.y);
	table.push_back(texcoord);
}

void readFace(	const std::string &line,
				std::vector<sVertexIndex> &vertexTable,
				std::vector<std::list<sVertexIndex> > &existingTable,
				std::vector<Mesh::sFace> &faceTable,
				int sg )
{
	int position[4] = 	{-1,-1,-1,-1};
	int normal[4] 	= 	{-1,-1,-1,-1};
	int texcoord[4] = 	{-1,-1,-1,-1};
	int params[3] 	= 	{-1,-1,-1};
	int count = 0;

	// f 1 2 3 (4)
	// f 1/2 1/2 1/2 (1/2)
	// f 1/2/3 1/2/3 1/2/3 (1/2/3)

	// shared buffer for chunks, wierd bug if memory was allocated
	// as char c[4][100]
	char *buffer = new char[400];
	char *c[4] = {&buffer[0], &buffer[100], &buffer[200], &buffer[300]};

	count = sscanf(line.c_str(), "%*s %s %s %s %s", c[0], c[1], c[2], c[3]);

	for(int i=0; i<count; ++i)
	{
		int p = sscanf(c[i], "%i/%i/%i", &params[0], &params[1], &params[2]);
		position[i] = (p > 0 && params[0] > 0) ? params[0]-1 : -1;
		normal[i] 	= (p > 1 && params[1] > 0) ? params[1]-1 : -1;
		texcoord[i] = (p > 2 && params[2] > 0) ? params[2]-1 : -1;
	}

	delete[] buffer;

	// Triangle or Quad
	if(count == 3 || count == 4)
	{
		sVertexIndex vertex;
		Mesh::sFace face;
		unsigned int indices[4] = {0,0,0,0};

		for(int i=0; i<count; ++i)
		{
			vertex.position = position[i];
			vertex.normal 	= normal[i];
			vertex.texcoord = texcoord[i];
			indices[i] = insertVertexIndex(vertex, vertexTable, existingTable);
		}

		face.v[0] = indices[0];
		face.v[1] = indices[1];
		face.v[2] = indices[2];

		faceTable.push_back(face);

		if(count == 4)
		{
			face.v[0] = indices[3];
			face.v[1] = indices[0];
			face.v[2] = indices[2];
			faceTable.push_back(face);
		}
	}
}

void readSG(const std::string &line,
			int &sg )
{
	// s 1
	sscanf(line.c_str(), "%*s %i", &sg);
}

void readG(	const std::string &line,
			std::string &name)
{
	char *str = new char[128];
	int count = sscanf(line.c_str(), "%*s %s", str);

	if(count > 0)
		name = std::string(str);

	delete[] str;
}

Mesh createMesh(const std::string &name,
				std::vector<sVertexIndex> &vertexTable,
				std::vector<Mesh::sFace> &faceTable,
				std::vector<sVec3> &positionTable,
				std::vector<sVec3> &normalTable,
				std::vector<sVec2> &texcoordTable )
{
	Mesh mesh;
	mesh.name = name;

	for(unsigned int i=0; i<vertexTable.size(); ++i)
	{
		int pos, norm, tc;
		pos = vertexTable[i].position;
		norm = vertexTable[i].normal;
		tc = vertexTable[i].texcoord;

		mesh.vertices.push_back(Mesh::sVertex());

		if(pos > -1 && pos < (int)positionTable.size())
		{
			mesh.vertices[i].x = positionTable[pos].x;
			mesh.vertices[i].y = positionTable[pos].y;
			mesh.vertices[i].z = positionTable[pos].z;
		}
		else
		{
			mesh.vertices[i].x = 0.0f;
			mesh.vertices[i].y = 0.0f;
			mesh.vertices[i].z = 0.0f;
		}

		if(norm > -1 && norm < (int)normalTable.size())
		{
			mesh.vertices[i].nx = normalTable[norm].x;
			mesh.vertices[i].ny = normalTable[norm].y;
			mesh.vertices[i].nz = normalTable[norm].z;
		}
		else
		{
			mesh.vertices[i].nx = 0.0f;
			mesh.vertices[i].ny = 0.0f;
			mesh.vertices[i].nz = 0.0f;
		}

		if(tc > -1 && tc < (int)texcoordTable.size())
		{
			mesh.vertices[i].u = texcoordTable[norm].x;
			mesh.vertices[i].v = texcoordTable[norm].y;
		}
		else
		{
			mesh.vertices[i].u = 0.0f;
			mesh.vertices[i].v = 0.0f;
		}
	}

	mesh.faces = faceTable;

	return mesh;
}