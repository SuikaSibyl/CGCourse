#pragma once

#include "../Core/Object3D.h"

class Triangle :public Object3D
{
public:
	Triangle(Vec3f& a, Vec3f& b, Vec3f& c, Material* m);
	virtual void paint(void) override;
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;

private:
	Vec3f p1;
	Vec3f p2;
	Vec3f p3;

	Vec3f e1;
	Vec3f e2;

	Vec3f normal;
	float distance;
};