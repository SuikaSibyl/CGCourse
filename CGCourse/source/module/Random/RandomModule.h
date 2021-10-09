#pragma once

#include <random>
#include <vectors.h>

class RandomModule
{
public:
	RandomModule();

	float RandFloat01();
	Vec3f RandVec2f0101();
	Vec2f RandVec2f_0101();

private:
	std::random_device randDev;
	std::uniform_real_distribution<float> UFloatDistribution;
};