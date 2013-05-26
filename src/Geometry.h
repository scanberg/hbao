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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Types.h"
#include <vector>
#include <cassert>
#include <string>

class Geometry
{
public:
    /** Structure of Vertex used in Geometry **/
    typedef struct
    {
        vec3 position;
        vec3 normal;
        vec3 tangent;
        vec2 texCoord;
    }sVertex;

    Geometry();
    ~Geometry();

    /** GET functions **/
    u32 getVertexSize();
    u32 getTriangleSize();

    const vec3 &getVertexPosition(const u32 &vertexIndex) const;
    const vec3 &getVertexNormal(const u32 &vertexIndex) const;
    const vec2 &getVertexTextCoord(const u32 &vertexIndex) const;

    const uvec3 &getTriangleIndices(const u32 &triangleIndex) const;
    const vec3 &getTrianglePoint(const u32 &triangleIndex, const i32 &point) const;

    const f32 *getVertexData();
    const u32 *getTriangleData();

    /** Functions **/
    void addVertex(const sVertex &vertex);
    void addTriangle(const uvec3 &triangle);
    const Geometry& addGeometry(const Geometry &geometry);

    void translate(const vec3 &translation);
    void rotate(const vec3 &rotation);
    void scale(float scale);

    bool createStaticBuffers(GLint posLoc=0, GLint normLoc=1, GLint tangLoc=2, GLint texLoc=3);
    bool createDynamicBuffers();
    inline bool existOnGpu() { return glIsVertexArray(vao); }
    bool updateBuffers();

    void destroyBuffers();

    void process();
    void clear();

    void draw();

private:
    friend bool loadObj(std::vector<Geometry> &geomList,
                        const std::string &filename,
                        float scale, int flags );

    u32 vao;
    u32 vbo_vertex;
    u32 vbo_triangle;

    std::vector<sVertex> vertices;
    std::vector<uvec3> triangles;
};
    
#endif
