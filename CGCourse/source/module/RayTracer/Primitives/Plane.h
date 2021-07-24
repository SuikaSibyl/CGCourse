#pragma once

#include "../Core/Object3D.h"

class Plane :public Object3D
{
public:
	Plane(Vec3f& normal, float d, Material* m);
	virtual void paint(void) override;
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;

private:
	Vec3f normal;
	float distance;
};