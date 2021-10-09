#pragma once

#include "../Core/Object3D.h"

extern int tessx;
extern int tessy;
extern bool gouraud;

class Sphere :public Object3D
{
public:
	Sphere(Vec3f center, float radius, Material* mat);
	virtual void paint(void) override;
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;
	virtual void insertIntoGrid(Grid* g, Matrix* m) override;

private:
	float radius;
	Vec3f center;

	void BuildMesh();
	Vec3f getPoint(float u, float v);
	int num;
	Vec3f* points;
	Vec3f* normals;
};