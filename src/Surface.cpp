#include "Surface.h"

void Surface::loadDiffuseTexture(const char *filename)
{
	GLFWimage img;
	if(GL_TRUE == glfwReadImage(filename, &img, 0))
	{

		GLint internalFormat = GL_RGBA8;
		GLint format = GL_RGBA;
		GLint type = GL_BYTE;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, noise);
	}

	glfwFreeImage(img);
}

void Surface::loadMaskTexture(const char *filename)
{

}

void Surface::loadSpecularTexture(const char *filename)
{

}

void Surface::loadNormalTexture(const char *filename)
{

}