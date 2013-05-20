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

#include "Log.h"
#include "Types.h"

void printErrorsGL(const char *func, int line)
{
//
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
    	printf("%s:%i :\n",func,line);
    	switch(glErr)
    	{
    	
			case GL_INVALID_ENUM:
				printf("glError: Invalid enum \n");
				break;

			case GL_INVALID_VALUE:
				printf("glError: Invalid value\n");
				break;

			case GL_INVALID_OPERATION:
				printf("glError: Invalid operation \n");
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				printf("glError: Invalid framebuffer operation \n");
				break;

			case GL_OUT_OF_MEMORY:
				printf("glError: Out of memory \n");
				break;

			#ifdef GL_STACK_UNDERFLOW
			case GL_STACK_UNDERFLOW:
				printf("glError: Stack underflow \n");
				break;
			#endif

			#ifdef GL_STACK_OVERFLOW
			case GL_STACK_OVERFLOW:
				printf("glError: Stack underflow \n");
				break;
			#endif

			default:
				printf("glError: unrecognized error \n");
    	}
        glErr = glGetError();
    }
}