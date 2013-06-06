#pragma once
#include "Geometry.h"
#include "ObjLoader.h"

void fillGeometryFromMesh(Geometry *g, Mesh *m)
{
	if(g && m)
	{
		for(unsigned int i=0; i<m->vertices.size(); ++i)
		{
			Geometry::sVertex v;
			v.position 	= vec3(m->vertices[i].x, m->vertices[i].y, m->vertices[i].z);
			v.normal 	= vec3(m->vertices[i].nx, m->vertices[i].ny, m->vertices[i].nz);
			v.texCoord	= vec2(m->vertices[i].u, m->vertices[i].v);
			g->addVertex(v);
		}

		for(unsigned int i=0; i<m->faces.size(); ++i)
		{
			uvec3 t(m->faces[i].v[0], m->faces[i].v[1], m->faces[i].v[2]);
			g->addTriangle(t);
		}

		g->process();
		
		return;
	}

	printf("Bad pointers given to adapter.");
}

Geometry createGeometryFromMesh(const Mesh &mesh)
{
	Geometry geom;

	for(unsigned int i=0; i<mesh.vertices.size(); ++i)
	{
		Geometry::sVertex v;
		v.position 	= vec3(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z);
		v.normal 	= vec3(mesh.vertices[i].nx, mesh.vertices[i].ny, mesh.vertices[i].nz);
		v.texCoord	= vec2(mesh.vertices[i].u, mesh.vertices[i].v);
		geom.addVertex(v);
	}

	for(unsigned int i=0; i<mesh.faces.size(); ++i)
	{
		uvec3 t(mesh.faces[i].v[0], mesh.faces[i].v[1], mesh.faces[i].v[2]);
		geom.addTriangle(t);
	}

	//geom.process();
	geom.material = mesh.material;
	//printf("Geom material %s\n", geom.material.c_str());
	
	return geom;
}

std::vector<Geometry> createGeometryFromMesh(const std::vector<Mesh> &meshes)
{
	std::vector<Geometry> geom;

	for(unsigned int i=0; i<meshes.size(); ++i)
	{
		geom.push_back(createGeometryFromMesh(meshes[i]));
	}

	return geom;
}