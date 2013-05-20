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

#include <iostream>
#include <string>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <list>

#include "ObjLoader.h"

#define toFloat(x) atof(x.c_str())
#define toInt(x) atoi(x.c_str())

#define hasVertex tempVertex.size() > 0
#define hasTexCoord tempTexCoord.size() > 0
#define hasNormal tempNormal.size() > 0

//#define DEBUG

typedef struct
{
    unsigned int vertexIndex, texCoordIndex, normalIndex;
}sStoredVertex;

// Helper function to retrive the indices in a face chunk
void getIndices(const std::string &str, int &v, int &t, int &n, bool vertex, bool texCoord, bool normal)
{
    v = t = n = -1;

    int slash[2];
    int counter=0;

    slash[0] = (int)str.length();

    for(int i=0; i<(int)str.length(); ++i)
    {
        if(str[i]=='/')
        {
            slash[counter]=i;
            ++counter;
        }
    }

    if(vertex)
        v = atoi(str.substr(0,slash[0]).c_str())-1;

    if(counter==0)
        return;

    if(slash[0]+1 != slash[1] && texCoord)
        t = atoi(str.substr(slash[0]+1,slash[0]+1+slash[1]).c_str())-1;

    if(normal)
        n = atoi(str.substr(slash[1]+1,str.length()).c_str())-1;
}

bool loadObj(Geometry &geom, const std::string &filename, float scale, int flags)
{
    std::vector<Geometry> geomList;
    loadObj(geomList,filename,scale,flags);

    geom.clear();

    // Pack all Geometry into one.
    for(unsigned int i=0; i<geomList.size(); ++i)
    {
        geom.addGeometry(geomList[i]);
    }

    return 0;
}

class VertexBank
{
public:
    VertexBank()
    {
        indexCounter = 0;
        uniqueVertex.reserve(10000);
    };

    bool isUnique(int v, int n, int t, int &index)
    {
        if(v >= uniqueVertex.size())
        {
            //printf("v is more than size \n");

            uniqueVertex.resize(v+1, std::list<VertexItem>() );
            index = insert(v,n,t);
            return true;
        }
        else
        {
            
            std::list<VertexItem>::const_iterator it = uniqueVertex[v].begin();
            while(it != uniqueVertex[v].end())
            {
                //if(n != it->n && t != it-> t)
                //{
                    index = it->realIndex;
                    return false;
                //}
                ++it;
            }
            index = insert(v,n,t);
            return true;
        }
    }

private:
    int insert(int v, int n, int t)
    {
        //printf("inserting new item \n");
        VertexItem vert;
        vert.v = v;
        vert.n = n;
        vert.t = t;

        //printf("Inserting n: %i, t: %i \n", n, t);
        vert.realIndex = indexCounter++;            

        uniqueVertex[v].push_back(vert);

        return vert.realIndex;
    }

    typedef struct
    {
        int v;
        int n;
        int t;
        int realIndex;
    }VertexItem;

    int indexCounter;

    std::vector< std::list<VertexItem> > uniqueVertex;
};

size_t insertUnique(std::vector<vec2> &vec, const vec2 &item)
{
    for(size_t i=0; i<vec.size(); ++i)
    {
        if(vec[i] == item)
        {
            printf("not unique! \n");
            return i;
        }
    }

    vec.push_back(item);
    return vec.size()-1;
}

size_t insertUnique(std::vector<vec3> &vec, const vec3 &item)
{
    for(size_t i=0; i<vec.size(); ++i)
    {
        if(vec[i] == item)
        {
            printf("not unique! \n");
            return i;
        }
    }

    vec.push_back(item);
    return vec.size()-1;
}

bool loadObj( std::vector<Geometry> &geomList, const std::string &filename, float scale, int flags)
{
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);

    std::cout<<"loading "<<filename<<std::endl;

    if(file.fail())
    {
        std::cout<<"loadObj failed, could not read "<<std::endl;
        return 1;
    }

    VertexBank vb;

    Geometry g;
    std::string line,param;

    std::vector<vec3> tempVertex;
    std::vector<vec3> tempNormal;
    std::vector<vec2> tempTexCoord;

    tempVertex.reserve(10000);
    tempNormal.reserve(10000);
    tempTexCoord.reserve(10000);
    
    std::vector<std::vector<int> > vertexUsed;
    std::vector<int> texCoordUsed;
    int tempSG = 0;

    std::vector<size_t> vertexRemap;
    std::vector<size_t> normalRemap;
    std::vector<size_t> texCoordRemap;

    std::vector<int> resetVector;
    resetVector.resize(1,-1);

    std::string tempName;

    while( !file.eof() && file.good() )
    {
        std::getline(file,line);

        #ifdef DEBUG
        std::cout<<line<<"\n";
        #endif
        Tokenizer token(line);

        param = token.getToken();
        if(param == "v")
        {
            vec3 vertex;

            vertex.x = scale*toFloat(token.getToken());
            vertex.y = scale*toFloat(token.getToken());
            vertex.z = scale*toFloat(token.getToken());

            //tempVertex.push_back(vertex);
            //vertexUsed.push_back(resetVector);
            vertexRemap.push_back( insertUnique(tempVertex, vertex) );
        }
        else if(param == "f")
        {
            ivec4 vdata(-1), tdata(-1), ndata(-1), fdata(-1);

            for(int i=0; i<token.size()-1; ++i)
            {
                param = token.getToken();
                getIndices(param, vdata[i], tdata[i], ndata[i],
                    hasVertex,
                    hasTexCoord,
                    hasNormal);

                int remappedV = (vdata[i] > -1) ? vdata[i] : -1;
                int remappedN = (ndata[i] > -1) ? ndata[i] : -1;
                int remappedT = (tdata[i] > -1) ? tdata[i] : -1;

                int index;
                //printf("Checking vertex uniqueness \n");
                if(vb.isUnique(remappedV, remappedN, remappedT, index))
                {
                    index = g.getVertexSize();

                    Geometry::sVertex tv;

                    assert( remappedV < tempVertex.size() );
                    tv.position = tempVertex[ remappedV ];

                    if(remappedT > -1)
                    {
                        assert( remappedT < tempTexCoord.size() );
                        tv.texCoord = tempTexCoord[ remappedT ];
                    }
                    if(remappedN > -1)
                    {
                        assert( remappedN < tempNormal.size() );
                        tv.normal = tempNormal[ remappedN ];
                    }

                    g.addVertex(tv);
                }

                assert(index < g.getVertexSize());
                fdata[i] = index;

                // if(tempSG > (int)vertexUsed[vdata[i]].size()-1)
                //     vertexUsed[vdata[i]].resize(tempSG+1,-1);

                // if(vertexUsed[vdata[i]][tempSG] > -1)
                //     fdata[i] = vertexUsed[vdata[i]][tempSG];
                // else
                // {
                //     vertexUsed[vdata[i]][tempSG] = (int)g.vertices.size();

                //     fdata[i] = g.getVertexSize();

                //     Geometry::sVertex tv;
                //     tv.position = tempVertex[vdata[i]];
                //     //tv.nx = tv.ny = tv.nz = tv.s = tv.t = 0.0f;

                //     if(vtdata[i]>-1 && !(flags & LOADOBJ_IGNORE_TEXCOORDS))
                //     {
                //         assert( vtdata[i] < tempTexCoord.size() );
                //         tv.texCoord = tempTexCoord[vtdata[i]];
                //     }
                //     if(ndata[i]>-1 && !(flags & LOADOBJ_IGNORE_NORMALS))
                //     {
                //         assert( ndata[i] < tempNormal.size() );
                //         tv.normal = tempNormal[ndata[i]];
                //     }

                //     g.addVertex(tv);
                // }
            }
            // if its a triangle, just insert.
            // However if its a quad, then insert the two triangles forming the quad.
            uvec3 t;
            t[0] = fdata[0];
            t[1] = fdata[1];
            t[2] = fdata[2];

            g.addTriangle(t);

            if(fdata[3] != -1)
            {
                t[0] = fdata[3];
                t[1] = fdata[0];
                t[2] = fdata[2];

                g.addTriangle(t);
            }
        }
        else if(param == "vt")
        {
            vec2 tc;

            tc.x = toFloat(token.getToken());
            tc.y = toFloat(token.getToken());

            //tempTexCoord.push_back(tc);
            texCoordRemap.push_back( insertUnique(tempTexCoord, tc) );
        }
        else if(param == "vn")
        {
            vec3 normal;

            normal.x = toFloat(token.getToken());
            normal.y = toFloat(token.getToken());
            normal.z = toFloat(token.getToken());

            //tempNormal.push_back(normal);
            normalRemap.push_back( insertUnique(tempNormal, normal) );
        }
        else if(param == "s")
            tempSG = toInt(token.getToken());
        else if(param == "g")
        {
            /*if(first)
                first=false;
            else
            {
                g.process();
                geomList.push_back(g);
            }

            for(unsigned int i=0; i<vertexUsed.size(); ++i)
                vertexUsed[i].clear();
            
            g.clear();
            */
        }

        if(file.eof())
            break;
    }
    file.close();
    printf("tempVertex.size() = %i \n", (int)tempVertex.size());
    printf("tempNormal.size() = %i \n", (int)tempNormal.size());
    printf("tempTexCoord.size() = %i \n", (int)tempTexCoord.size());
    printf("Reading is done, gonna process \n");
    g.process();
    geomList.push_back(g);

    std::cout<<"done reading "<<filename<<std::endl;

    return 0;
}
