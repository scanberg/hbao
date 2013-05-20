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

#ifndef SHADER_H
#define SHADER_H

/*#ifdef __APPLE__
    #define GLFW_INCLUDE_GL3
    #define GLFW_NO_GLU    
#else
    #include <GL/glew.h>
#endif

#include <GL/glfw.h>*/

#include "Types.h"

class Shader
{
private:
    GLuint program;
    GLuint vertexShader, fragmentShader;

    /*
     * Holders for predefined locations in the shader.
     * The predefined variables are:
     *
     * in_position: position data sent to vertex shader.
     * in_normal: normal data sent to vertex shader.
     * in_texCoord: texture coordinate data sent to vertex shader.
     *
     * viewMatrix: actually the model to view matrix
     * projMatrix: the projection matrix
     */
     
    GLint positionLoc, normalLoc, tangentLoc, texCoordLoc;
    GLint modelMatrixLoc, viewMatrixLoc, projMatrixLoc;

    char *vertexFile, *fragmentFile;
    bool compiled;

    static Shader *boundShader;
    
public:
    Shader();
    Shader(const char *vertFile, const char *fragFile);
    ~Shader();

    static Shader *getBoundShader() { return boundShader; }

    bool loadAndCompile();

    void setVertexFile(const char *vertFile);
    void setFragmentFile(const char *fragFile);

    const char *getVertexFile() { return vertexFile; }
    const char *getFragmentFile() { return fragmentFile; }

    GLint getAttributeLocation(const char *att);
    GLint getUniformLocation(const char *uni);

    GLint getPositionLocation() { return positionLoc; }
    GLint getNormalLocation() { return normalLoc; }
    GLint getTangentLocation() { return tangentLoc; }
    GLint getTexCoordLocation() { return texCoordLoc; }

    GLint getModelMatrixLocation() { return modelMatrixLoc; }
    GLint getViewMatrixLocation() { return viewMatrixLoc; }
    GLint getProjMatrixLocation() { return projMatrixLoc; }

    bool isCompiled() { return compiled; }

    void bind();
    void unbind();
};

#endif
