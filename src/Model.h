#pragma once
#include "Geometry.h"
#include "Surface.h"
#include <vector>

class Model
{
	typedef struct
	{
		unsigned int indexStart;
		unsigned int indexCount;

		Surface *surface;
	}sSubModel;
public:
	void draw()
	{
		geom.bindVAO();
		for(unsigned int i=0; i<subModels.size(); ++i)
		{
			if(subModels[i].surface)
				subModels[i].surface->bind();
			glDrawElements(GL_TRIANGLES, subModels[i].indexCount, GL_UNSIGNED_INT, (void*)(subModels[i].indexStart));
		}
		geom.unbindVAO();
	}

	void addGeometryAndSurface(Geometry *g, Surface *s)
	{
		subModels.push_back(sSubModel());
		sSubModel &sm = subModels[subModels.size()-1];
		sm.indexStart = geom.getTriangleSize();
		geom.addGeometry(*g);
		sm.indexCount = 3*(geom.getTriangleSize() - sm.indexStart);
		sm.indexStart = 3*sizeof(unsigned int)*sm.indexStart;

		sm.surface = s;
	}

	void prepare()
	{
		geom.process();
		geom.createStaticBuffers();
	}
private:
	Geometry geom;
	std::vector<sSubModel> subModels;
};