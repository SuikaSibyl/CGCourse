#pragma once

#include "../Core/Object3D.h"

class Sphere :public Object3D
{
public:
	Sphere(Vec3f center, float radius, Material* mat);
	virtual void paint(void) override;
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;

private:
	float radius;
	Vec3f center;
};