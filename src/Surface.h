#pragma once
#include "Types.h"

class Surface
{
public:
	Surface() :
		diffuseTexture(0),
		specularTexture(0),
		normalTexture(0)
		{}

	~Surface()
	{
		if(diffuseTexture && glIsTexture(diffuseTexture))
			glDeleteTextures(1, &diffuseTexture);
		if(specularTexture && glIsTexture(specularTexture))
			glDeleteTextures(1, &specularTexture);
		if(normalTexture && glIsTexture(normalTexture))
			glDeleteTextures(1, &normalTexture);
	}

	void loadDiffuseTexture(const char *filename);
	void loadMaskTexture(const char *filename);
	void loadSpecularTexture(const char *filename);
	void loadNormalTexture(const char *filename);

private:
	vec3 diffuseColor;
	vec4 specularColor;

	GLuint diffuseTexture;
	GLuint specularTexture;
	GLuint normalTexture;
};