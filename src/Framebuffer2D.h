/**
Copyright (C) 2012-2014 Robin Skånberg

Permission is hereby granted, free of charge,
to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FRAMEBUFFER2D_H
#define FRAMEBUFFER2D_H

// #ifdef __APPLE__
// 	#define GLFW_INCLUDE_GL3
// 	#define GLFW_NO_GLU
// #else
// 	#include <GL/glew.h>
// #endif

// #include <GL/glfw.h>

#include "Types.h"

class Framebuffer2D
{
public:
	Framebuffer2D(int width, int height);
	~Framebuffer2D();

/**
 * Assigns a buffer specified in $buffer with a texture
 *
 * @buffer unsigned char, The buffer to be assigned, valid targets are specified in Table 1
 *
 *
 *
 * @return bool Returns true if the buffer was assigned successfully.
 */
	bool attachBuffer(	unsigned char buffer,
						GLint internalFormat = 		GL_RGBA,
						GLint format = 				GL_RGBA,
						GLint type = 				GL_UNSIGNED_INT,
						GLint textureMinFilter = 	GL_NEAREST,
						GLint textureMagFilter = 	GL_NEAREST,
						GLint textureWrapS = 		GL_CLAMP_TO_EDGE,
						GLint textureWrapT = 		GL_CLAMP_TO_EDGE,
						GLboolean mipMap =			GL_FALSE);

	void destroyBuffers(unsigned char bufferBit);

	void resizeBuffers(unsigned char bufferBit, int width, int height);

	inline int getWidth() { return width; }
	inline int getHeight() { return height; }

	void bind();
	void unbind();

	inline unsigned int getBufferHandle(unsigned char buffer) { return *getTextureHandle(buffer); }

private:
	bool bufferIsAux(unsigned char buffer);
	bool bufferIsDepth(unsigned char buffer);
	bool bufferIsValid(unsigned char buffer);
	unsigned int *getTextureHandle(unsigned char buffer);
	GLenum getGLAttachment(unsigned char buffer);
	void updateAuxBuffers();

	int width, height;

	GLenum *auxBuffers;
	unsigned char numAuxBuffers;

	unsigned int bufferHandle[5];

	unsigned int renderBufferDepthHandle;

	unsigned int fboHandle;
};

/**
 * Table 1: Buffers that can be specified in assignBuffer
 */
#define FBO_AUX0	0x00
#define FBO_AUX1	0x01
#define FBO_AUX2	0x02
#define FBO_AUX3	0x03
#define FBO_DEPTH	0x04
#define FBO_COUNT	0x05

/**
 * Table 2: Buffer bits that can be specified in resizeBuffers
 */
#define FBO_AUX0_BIT 	0x00
#define FBO_AUX1_BIT 	0x01
#define FBO_AUX2_BIT 	0x02
#define FBO_AUX3_BIT 	0x04
#define FBO_DEPTH_BIT	0x08
#define RBO_DEPTH_BIT	0x10


#endif