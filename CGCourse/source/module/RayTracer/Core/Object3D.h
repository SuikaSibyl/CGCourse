#pragma once

#include "material.h"
#include "ray.h"
#include "hit.h"
#include "BoundingBox.h"

class Grid;

class Object3D
{
public:
	virtual ~Object3D()
	{
		if (boundingBox != nullptr)
			delete boundingBox;
	}
	virtual bool intersect(const Ray& r, Hit& h, float tmin) = 0;
	virtual void paint(void) = 0;
	Material* mat;

	BoundingBox* getBoundingBox() { return boundingBox; }
	void setBoundingBox(BoundingBox* bb) { boundingBox = new BoundingBox(*bb); }
	virtual void insertIntoGrid(Grid* g, Matrix* m) {}

protected:
	BoundingBox* boundingBox = nullptr;
};