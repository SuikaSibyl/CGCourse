#pragma once

#include "material.h"
#include "../Materials/PerlinNoise.h"
#include <matrix.h>

class Noise :public Material
{
public:
	Noise(Matrix* m, Material* mat1, Material* mat2, int octaves)
		:Material(Vec3f()), mMatrix(*m), mMat1(mat1), mMat2(mat2), mOctaves(octaves)
	{

	}

	virtual void glSetMaterial(void) const
	{
		mMat1->glSetMaterial();
	}

	virtual float Nxyz(float x, float y, float z) const
	{
		float N = 0;
		int weight = 1;
		for (int i = 0; i < mOctaves; i++)
		{
			float rand = PerlinNoise::noise(x, y, z);
			N += rand / weight;
			x *= 2;
			y *= 2;
			z *= 2;
			weight *= 2;
		}
		return  0.5 + N;
	}

	virtual Vec3f Shade(const Ray& ray, const Hit& hit, const Vec3f& dirToLight,
		const Vec3f& lightColor) const
	{
		float x, y, z;
		Vec3f point = hit.getIntersectionPoint();
		mMatrix.Transform(point);
		x = point.x();
		y = point.y();
		z = point.z();
		
		float N = Nxyz(x, y, z);

		float w = (N < 0) ? 0 : N;
		w = (w > 1) ? 1 : w;

		Vec3f ret(0, 0, 0);
		ret += w * mMat1->Shade(ray, hit, dirToLight, lightColor);
		ret += (1 - w) * mMat2->Shade(ray, hit, dirToLight, lightColor);

		return ret;
	}



protected:
	Matrix mMatrix;
	Material* mMat1;
	Material* mMat2;
	int mOctaves;
};