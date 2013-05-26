#pragma once
#include "Geometry.h"
#include "ObjLoader.h"

void fillGeometryFromMesh(Geometry *g, Mesh *m)
{
	if(g && m)
	{
		for(unsigned int i=0; i<m->numVertices; ++i)
		{
			Geometry::sVertex v;
			v.position 	= vec3(m->vertices[i].x, m->vertices[i].y, m->vertices[i].z);
			v.normal 	= vec3(m->vertices[i].nx, m->vertices[i].ny, m->vertices[i].nz);
			v.texCoord	= vec2(m->vertices[i].u, m->vertices[i].v);
			g->addVertex(v);
		}

		for(unsigned int i=0; i<m->numFaces; ++i)
		{
			uvec3 t(m->faces[i].v[0], m->faces[i].v[1], m->faces[i].v[2]);
			g->addTriangle(t);
		}

		g->process();
	}

	printf("Bad pointers given to adapter.");
}