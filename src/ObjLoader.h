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

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include "Geometry.h"
#include "Tokenizer.h"

#define LOADOBJ_IGNORE_NORMALS 		0x01
#define LOADOBJ_IGNORE_TEXCOORDS	0x02

// Load the obj-data and pack into a single Geometry
bool loadObj( Geometry &geom, const std::string &filename, float scale = 1.0f , int flags = 0);

// Load the obj-data and fill a vector of Geometry
bool loadObj( std::vector<Geometry> &geomList, const std::string &filename, float scale = 1.0f, int flags = 0);

#endif
