#pragma once
#include "material.h"
#include <matrix.h>

class Checkerboard :public Material
{
public:
	Checkerboard(Matrix* m, Material* mat1, Material* mat2)
		:Material(Vec3f()), mMat1(mat1), mMat2(mat2), mMatrix(*m)
	{

	}

	virtual void glSetMaterial(void) const
	{
		mMat1->glSetMaterial();
	}

	virtual Vec3f Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
		const Vec3f& lightColor) const
	{
		Vec3f point = hit.getIntersectionPoint();
		mMatrix.Transform(point);
		int i = (int(floor(point.x())) % 2 == 0) ? 1 : -1;
		int j = (int(floor(point.y())) % 2 == 0) ? 1 : -1;
		int k = (int(floor(point.z())) % 2 == 0) ? 1 : -1;
		int mul = i * j * k;
		if (mul == 1)
			return mMat1->Shade(ray, hit, dirToLight, lightColor);
		else
			return mMat2->Shade(ray, hit, dirToLight, lightColor);
	}

private:
	Material* mMat1;
	Material* mMat2;
	Matrix mMatrix;
};