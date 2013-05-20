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

#include <fstream>
#include <cstdio>
#include <cstring>
#include "Shader.h"
#include "Log.h"

#define BUFFER_SIZE 2048

Shader *Shader::boundShader = NULL;

/**
 * readTextFile - reads a simple textfile specified by filename,
 * if everything is ok, a pointer to a null-terminated string is returned,
 * otherwise NULL is returned.
 */

char* readTextFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL)
    {
      logError("Cannot open file %s", filename);
      return 0;
    }

    fseek(file, 0, SEEK_END);

    char *buffer = NULL;
    int bytesinfile = ftell(file);
    rewind(file);

    if(bytesinfile > 0)
    {
        buffer = (char*)malloc(bytesinfile+1);
        int bytesread = fread( buffer, 1, bytesinfile, file);
        buffer[bytesread] = '\0'; // Terminate the string with a null character
    }

    fclose(file);

    return buffer;
}

Shader::Shader()
{
    vertexShader    = 0;
    fragmentShader  = 0;
    program         = 0;
    compiled        = 0;

    positionLoc     = -1;
    normalLoc       = -1;
    tangentLoc      = -1;
    texCoordLoc     = -1;
    viewMatrixLoc   = -1;
    projMatrixLoc   = -1;

    vertexFile      = NULL;
    fragmentFile    = NULL;
}

Shader::Shader( const char *vertFile, const char *fragFile )
{
    vertexShader    = 0;
    fragmentShader  = 0;
    program         = 0;
    compiled        = 0;

    positionLoc     = -1;
    normalLoc       = -1;
    tangentLoc      = -1;
    texCoordLoc     = -1;
    viewMatrixLoc   = -1;
    projMatrixLoc   = -1;

    vertexFile      = NULL;
    fragmentFile    = NULL; 

    setVertexFile(vertFile);
    setFragmentFile(fragFile);
    loadAndCompile();
}

Shader::~Shader()
{
    if(vertexFile)
        delete vertexFile;

    if(fragmentFile)
        delete fragmentFile;

    glDetachShader(program, fragmentShader); // Detach the fragment shader
    glDetachShader(program, vertexShader); // Detach the vertex shader

    glDeleteShader(fragmentShader); // Delete the fragment shader
    glDeleteShader(vertexShader); // Delete the vertex shader
    glDeleteProgram(program); // Delete the shader program
}

void Shader::setVertexFile(const char *vertFile)
{
    if(!vertFile)
    {
        logError("A null-pointer was passed");
        return;
    }

    if(vertexFile)
        delete vertexFile;

    int len = (int)strlen(vertFile);

    vertexFile = new char[len+1];
    strcpy(vertexFile, vertFile);
    vertexFile[len] = '\0';
}

void Shader::setFragmentFile(const char *fragFile)
{
    if(!fragFile)
    {
        logError("A null-pointer was passed");
        return;
    }

    if(fragmentFile)
        delete fragmentFile;

    int len = (int)strlen(fragFile);

    fragmentFile = new char[len+1];
    strcpy(fragmentFile, fragFile);
    fragmentFile[len] = '\0';
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}
 
void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
 
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
 
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

/*
 * loadAndCompile - create, load, compile and link a shader object.
 */
bool Shader::loadAndCompile()
{
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
 
    char *vs = readTextFile(vertexFile);
    char *fs = readTextFile(fragmentFile);
 
    const char * vv = vs;
    const char * ff = fs;
 
    glShaderSource(vertexShader, 1, &vv, NULL);
    glShaderSource(fragmentShader, 1, &ff, NULL);
 
    free(vs);
    free(fs);
 
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
 
    printShaderInfoLog(vertexShader);
    printShaderInfoLog(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);
 
    glBindAttribLocation(program, 0, "in_position");
    glBindAttribLocation(program, 1, "in_normal");
    glBindAttribLocation(program, 2, "in_tangent");
    glBindAttribLocation(program, 3, "in_texCoord");

    glBindFragDataLocation(program, 0, "out_frag0");
    glBindFragDataLocation(program, 1, "out_frag1");
    glBindFragDataLocation(program, 2, "out_frag2");
    glBindFragDataLocation(program, 3, "out_frag3");

    glLinkProgram(program);
    printProgramInfoLog(program);
 
    positionLoc = glGetAttribLocation(program,"in_position");
    normalLoc = glGetAttribLocation(program, "in_normal");
    tangentLoc = glGetAttribLocation(program, "in_tangent");
    texCoordLoc = glGetAttribLocation(program, "in_texCoord");

    logNote("programId: %i, posLoc %i, normLoc %i, tangLoc %i, texLoc %i", program, positionLoc, normalLoc, tangentLoc, texCoordLoc);
 
    projMatrixLoc = glGetUniformLocation(program, "projMatrix");
    viewMatrixLoc = glGetUniformLocation(program, "viewMatrix");
    modelMatrixLoc = glGetUniformLocation(program, "modelMatrix");

    glUseProgram(program);

    char str[10];
    for(int i=0; i<8; ++i)
    {
        sprintf(str, "texture%i",i);
        int textureLoc = glGetUniformLocation(program, str);
        if(textureLoc > -1)
            glUniform1i(textureLoc, i);
    }

    glUseProgram(0);

    compiled = true;
 
    return true;
}

GLint Shader::getAttributeLocation(const char *att)
{
    return glGetAttribLocation(program, att);
}


GLint Shader::getUniformLocation(const char *uni)
{
    return glGetUniformLocation(program, uni);
}

void Shader::bind()
{
    if(!compiled)
        return;

    glUseProgram(program);
    boundShader = this;
}

void Shader::unbind()
{
    glUseProgram(0);
    boundShader = NULL;
}