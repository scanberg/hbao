#include "Surface.h"
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Surface::sTexture Surface::defaultDiffuseTexture;
Surface::sTexture Surface::defaultNormalTexture;

typedef enum
{
	RGB,
	A
}Target;

GLuint createTexture(	bool mipmap, GLint internalFormat, GLenum format, GLenum type,
						GLint width, GLint height, unsigned char *data)
{
	if(width == 0 || height == 0)
		printf("Texture width or height is zero\n");

	GLuint texID;
	glGenTextures(1, &texID);

	glBindTexture(GL_TEXTURE_2D, texID);

	//glTexStorage2D(GL_TEXTURE_2D, mipmap ? 4 : 1, internalFormat, width, height);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, data);

	glTexImage2D(	GL_TEXTURE_2D,
 					0,
 					internalFormat,
 					width,
 					height,
 					0,
 			  		format,
 					type,
 					data);

	if(mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if(mipmap)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return texID;
}

void updateTextureRGBA(Target target, unsigned char *data)
{
	//printf("is target RGB? %i\n",(int)(target==RGB));
	int width, height;
	unsigned char *texData;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	texData = new unsigned char [width*height*4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

	for(int y=0; y<height; ++y)
	{
		for(int x=0; x<width; ++x)
		{
			if(target == A)
			{
				int offset = y*width + x;
				texData[4*offset+3] = data[offset];
			}
			else if(target == RGB)
			{
				int offset = (y*width + x);
				texData[4*offset+0] = data[3*offset+0];
				texData[4*offset+1] = data[3*offset+1];
				texData[4*offset+2] = data[3*offset+2];
			}
		}
	}

	//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, texData);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	delete[] texData;
}

void Surface::init()
{
	const int width = 4;
	const int height = 4;

	unsigned char diffData[width*height*3];
	unsigned char normData[width*height*3];

	for(int y=0; y<height; ++y)
	{
		for(int x=0; x<width; ++x)
		{
			int i = 3*(y*width + x);
			diffData[i+0] = 255;
			diffData[i+1] = 255;
			diffData[i+2] = 255;

			normData[i+0] = 0;
			normData[i+1] = 0;
			normData[i+2] = 255;
		}
	}

	defaultDiffuseTexture.handle = createTexture(false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, width, height, diffData);
	defaultNormalTexture.handle = createTexture(false, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, width, height, normData);

	printf("Handle of diffuse default %i \n", defaultDiffuseTexture.handle);
}

void Surface::cleanUp()
{
	if(defaultDiffuseTexture.handle && glIsTexture(defaultDiffuseTexture.handle))
		glDeleteTextures(1, &defaultDiffuseTexture.handle);

	if(defaultNormalTexture.handle && glIsTexture(defaultNormalTexture.handle))
		glDeleteTextures(1, &defaultNormalTexture.handle);
}

void Surface::loadDiffuseTexture(const char *filename)
{
	printf("loading diffuse texture from %s... ", filename);

    stbi_set_flip_vertically_on_load(1);

    int x, y, c;
    unsigned char* data = stbi_load(filename, &x, &y, &c, 4);

    if (!glIsTexture(diffuseTexture.handle))
    {
        diffuseTexture.handle = createTexture(true, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, x, y, data);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, diffuseTexture.handle);
        updateTextureRGBA(RGB, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

	stbi_image_free(data);
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

void Surface::bind()
{
	if(diffuseTexture.handle)
	{
		glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, diffuseTexture.handle);
	}
	else if (defaultDiffuseTexture.handle)
	{
		glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, defaultDiffuseTexture.handle);
	}

	if(normalTexture.handle)
	{
		glActiveTexture(GL_TEXTURE1);
	    glBindTexture(GL_TEXTURE_2D, normalTexture.handle);
	}
	else if (defaultNormalTexture.handle)
	{
		glActiveTexture(GL_TEXTURE1);
	    glBindTexture(GL_TEXTURE_2D, defaultNormalTexture.handle);
	}
}

void Surface::unbind()
{
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}