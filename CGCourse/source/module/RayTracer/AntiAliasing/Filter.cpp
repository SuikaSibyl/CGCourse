#include "Filter.h"
#include "Film.h"

Vec3f Filter::getColor(int i, int j, Film* film)
{
	int spp = film->getNumSamples();
	float weight;
	float wsum = 0;
	Vec3f color(0, 0, 0);

	int radius = getSupportRadius();
	// x : [i-supportRadius, i+supportRadius]
	// y : [j-supportRadius, j+supportRadius]
	for (int rx = -radius; rx <= radius; rx++)
	{
		for (int ry = -radius; ry <= radius; ry++)
		{
			for (int ind = 0; ind < spp; ind++)
			{
				bool outOfBound = false;
				Sample sample = film->getSample(i + rx, j + ry, ind, outOfBound);
				if (!outOfBound)
				{
					Vec2f position = sample.getPosition();
					Vec3f col = sample.getColor();
					weight = getWeight(position.x() - 0.5 + rx, position.y() - 0.5 + ry);
					color += weight * col;
					wsum += weight;
				}
			}
		}
	}

	return color * (1. / wsum);
}

float BoxFilter::getWeight(float x, float y)
{
	if (max(abs(x), abs(y)) <= Radius)
		return 1;
	else
		return 0;
}

float TentFilter::getWeight(float x, float y)
{
	float distance = Vec2f(x, y).Length();
	if (distance < Radius)
	{
		return 1 - distance * 1. / Radius;
	}
	return 0;
}

float GaussianFilter::getWeight(float x, float y)
{
	float distance = Vec2f(x, y).Length();
	//if (distance < 2 * Sigma)
		return exp(-distance * distance / (2 * Sigma * Sigma));
	//else
	//	return 0;
}