#pragma once

#include "../Core/material.h"
#include "../Core/Noise.h"

class Wood :public Noise
{
public:
	Wood(Matrix* m, Material* mat1, Material* mat2, int octaves, float frequency, float amplitude)
		:Noise(m, mat1, mat2, octaves), mFrequency(frequency), mAmplitude(amplitude)
	{

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

		float N = Nxyz(x, 0.1 * y, z) - 0.5;
		float M = N;
		//float M = sin(mFrequency * z + mAmplitude * N) * 0.5 + 0.5f;

		float w = (M < 0) ? 0 : M;
		w = (w > 1) ? 1 : w;

		Vec3f ret(0, 0, 0);
		ret += w * mMat1->Shade(ray, hit, dirToLight, lightColor);
		ret += (1 - w) * mMat2->Shade(ray, hit, dirToLight, lightColor);

		return ret;
	}

private:
	float mFrequency;
	float mAmplitude;
};