#pragma once
#include <map>
#include <vector>
#include <string>
#include "MtlLoader.h"
#include "Surface.h"

std::map<std::string, Surface*> createSurfaceFromMaterial(const std::vector<Material> &mat, const std::string &path = "")
{
	std::map<std::string, Surface*> surf;

	for(unsigned int i=0; i<mat.size(); ++i)
	{
		Surface *s = new Surface();

		if(mat[i].diffuseMap != "")
			s->loadDiffuseTexture( (path + mat[i].diffuseMap).c_str() );

		if(mat[i].maskMap != "")
			s->loadMaskTexture( (path + mat[i].maskMap).c_str() );

		if(mat[i].normalMap != "")
			s->loadNormalTexture( (path + mat[i].normalMap).c_str() );

		surf.insert( std::pair<std::string, Surface*> (mat[i].name, s) );
	}

	return surf;
}