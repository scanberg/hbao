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

#include "Geometry.h"

#include "Log.h"

Geometry::Geometry()
{
    vao = 0;
    vbo_vertex = 0;
    vbo_triangle = 0;
}

Geometry::~Geometry()
{
    destroyBuffers();
}

u32 Geometry::getVertexSize() { return vertices.size(); }
u32 Geometry::getTriangleSize() { return triangles.size(); }

const vec3 &Geometry::getVertexPosition(const u32 &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].position;
}

const vec3 &Geometry::getVertexNormal(const u32 &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].normal;
}

const vec2 &Geometry::getVertexTextCoord(const u32 &vertexIndex) const
{
    assert(vertexIndex < vertices.size());
    return vertices[vertexIndex].texCoord;
}

const uvec3 &Geometry::getTriangleIndices(const u32 &triangleIndex) const
{
    assert(triangleIndex < triangles.size());
    return triangles[triangleIndex];
}

const vec3 &Geometry::getTrianglePoint(const u32 &triangleIndex, const i32 &point) const
{
    assert(triangleIndex < triangles.size());
    assert(0 < point && point < 3);
    return vertices[ triangles[triangleIndex][point] ].position;
}

const f32 *Geometry::getVertexData()
{
    if(vertices.size() > 0)
        return &vertices[0].position[0];

    return NULL;
}

const u32 *Geometry::getTriangleData()
{
    if(triangles.size() > 0)
        return &triangles[0][0];

    return NULL;
}

void Geometry::addVertex(const sVertex &vertex)
{
    vertices.push_back(vertex);
}

void Geometry::addTriangle(const uvec3 &triangle)
{
    triangles.push_back(triangle);
}

const Geometry& Geometry::addGeometry(const Geometry &geom)
{
    u32 vertexOffset = vertices.size();
    uvec3 tri;

    for(u32 i=0; i<geom.vertices.size(); ++i)
    {
        vertices.push_back(geom.vertices[i]);
    }

    for(u32 i=0; i<geom.triangles.size(); ++i)
    {
        tri = geom.triangles[i];
        tri[0] += vertexOffset;
        tri[1] += vertexOffset;
        tri[2] += vertexOffset;

        triangles.push_back(tri);
    }

    return *this;
}

void Geometry::translate(const vec3 &translation)
{
    for(unsigned int i=0; i<vertices.size(); i++)
    {
        vertices[i].position += translation;
    }
}

void Geometry::rotate(const vec3 &rotation)
{

}

void Geometry::scale(float scale)
{

}

void Geometry::clear()
{
    vertices.clear();
    triangles.clear();
}

vec3 generateTangent(vec3 v1, vec3 v2, vec2 st1, vec2 st2)
{    
    float coef = 1.0f / (st1.x * st2.y - st2.x * st1.y);
    vec3 tangent;

    tangent.x = coef * ((v1.x * st2.y)  + (v2.x * -st1.y));
    tangent.y = coef * ((v1.y * st2.y)  + (v2.y * -st1.y));
    tangent.z = coef * ((v1.z * st2.y)  + (v2.z * -st1.y));
    
    return tangent;
}

void Geometry::process()
{
    glm::vec3 a,b,n,t;
    glm::vec2 sta, stb;

    std::vector<vec3> tempNormal, tempTangent;
    tempNormal.resize(vertices.size(),vec3(0));
    tempTangent.resize(vertices.size(),vec3(0));

    assert(vertices.size() > 0);
    assert(triangles.size() > 0);

    std::vector<i32> sharedFaces;
    sharedFaces.resize(vertices.size(), 0);

    printf("vertices.size() = %i \n", (int)vertices.size());
    printf("triangle.size() = %i \n", (int)triangles.size());

    vec3 center = vec3(0.0);

    // Find geometric center
    for (size_t i=0; i<vertices.size(); ++i)
    {
        center += vertices[i].position;
    }

    center /= (float)vertices.size();

    for (size_t i=0; i<vertices.size(); ++i)
    {
        vertices[i].position -= center;
    }

    for (size_t i=0; i<triangles.size(); ++i)
    {
        assert(i < triangles.size());

        a = vertices[triangles[i][1]].position - vertices[triangles[i][0]].position;
        b = vertices[triangles[i][2]].position - vertices[triangles[i][0]].position;

        n = glm::normalize(glm::cross(a,b));

        sta = vertices[triangles[i][1]].texCoord - vertices[triangles[i][0]].texCoord;
        stb = vertices[triangles[i][2]].texCoord - vertices[triangles[i][0]].texCoord;

        t = generateTangent(a,b,sta,stb);

        for(u32 u=0; u<3; ++u)
        {
            //printf("index is %i \n",triangles[i][u]);
            tempNormal[triangles[i][u]] += n;
            tempTangent[triangles[i][u]] += t;
            sharedFaces[triangles[i][u]]++;
        }
    }
    for (size_t i=0; i<vertices.size(); ++i)
    {
        if(sharedFaces[i]>0)
        {
            tempNormal[i] /= (f32)sharedFaces[i];
            tempNormal[i] = glm::normalize(tempNormal[i]);

            tempTangent[i] /= (f32)sharedFaces[i];
            tempTangent[i] = glm::normalize(tempTangent[i]);
        }
        if(glm::dot(vertices[i].normal, vertices[i].normal) == 0.0f)
        {
            vertices[i].normal = tempNormal[i];
        }

        const vec3 & t = tempTangent[i];
        const vec3 & n = tempNormal[i];

        // Gram-Schmidt orthogonalize
        vertices[i].tangent = glm::normalize(t - n * glm::dot(n, t));
    }
    printf("Done processing \n");
}

bool Geometry::createStaticBuffers(GLint posLoc, GLint normLoc, GLint tangLoc, GLint texLoc)
{
    destroyBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo_vertex);
    glGenBuffers(1, &vbo_triangle);

    // bind buffer for vertices and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(sVertex), &vertices[0].position[0], GL_STATIC_DRAW);

    // Enable specific pointer for Vertex, for compability-mode and attributepointer for shader
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glVertexPointer(3, GL_FLOAT, sizeof(sVertex), (char*)NULL);

    if(posLoc > -1)
    {
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL);
        glEnableVertexAttribArray(posLoc);
    }
    else
    {
        logWarning("posLoc in createStaticBuffer was undefined");
    }

    // Enable specific pointer for Normal, for compability-mode and attributepointer for shader
    //glEnableClientState(GL_NORMAL_ARRAY);
    //glNormalPointer(GL_FLOAT, sizeof(sVertex), (char*)NULL+3*sizeof(f32));

    if(normLoc > -1)
    {
        glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL+3*sizeof(f32));
        glEnableVertexAttribArray(normLoc);
    }
    else
    {
        logWarning("normLoc in createStaticBuffer was undefined");
    }

    if(tangLoc > -1)
    {
        glVertexAttribPointer(tangLoc, 3, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL+6*sizeof(f32));
        glEnableVertexAttribArray(tangLoc);
    }
    else
    {
        logWarning("tangLoc in createStaticBuffer was undefined");
    }

    // Enable specific pointer for TextureCoord, for compability-mode and attributepointer for shader
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glTexCoordPointer(2, GL_FLOAT, sizeof(sVertex), (char*)NULL+6*sizeof(f32));

    if(texLoc > -1)
    {
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(sVertex), (char*)NULL+9*sizeof(f32));
        glEnableVertexAttribArray(texLoc);
    }
    else
    {
        logWarning("texLoc in createStaticBuffer was undefined");
    }

    // Create and bind a BO for index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_triangle);
    // copy data into the buffer object
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size()*sizeof(uvec3), &triangles[0][0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    logNote("createdStaticBuffers: vertices %i, triangles %i",
            (int)vertices.size(), (int)triangles.size());

    return true;
}

void Geometry::destroyBuffers()
{
    if(glIsBuffer(vbo_vertex))
        glDeleteBuffers(1, &vbo_vertex);

    if(glIsBuffer(vbo_triangle))
        glDeleteBuffers(1, &vbo_triangle);

    if(glIsVertexArray(vao))
        glDeleteVertexArrays(1, &vao);

    vao = vbo_vertex = vbo_triangle = 0;
}

void Geometry::draw()
{
    if(vao)
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 3 * triangles.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void Geometry::bindVAO()
{
    if(vao)
        glBindVertexArray(vao);
}

void Geometry::unbindVAO()
{
    glBindVertexArray(0);
}

vec3 calculateTangent(glm::vec3 v1, glm::vec3 v2, glm::vec2 st1, glm::vec2 st2)
{
    vec3 tangent;
    float coef = 1.0 / (st1.x * st2.y - st2.x * st1.y);
    
    return coef * ((v1 * st2.y) + (v2 * -st1.y));
}
