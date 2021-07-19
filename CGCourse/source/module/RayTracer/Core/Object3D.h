#pragma once

#include "material.h"
#include "ray.h"
#include "hit.h"

class Object3D
{
public:
	virtual bool intersect(const Ray& r, Hit& h, float tmin) = 0;
	Material* mat;
};