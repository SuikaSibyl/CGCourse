#pragma once

#include <vectors.h>
#include <Random/RandomModule.h>

class Sampler
{
public:
	Sampler(int spp) {}
	virtual ~Sampler() = default;

	// Returns the 2D offset for the specified sample
	virtual Vec2f getSamplePosition(int n) = 0;
};

class RandomSampler :public Sampler
{
public:
	RandomSampler(int spp) :Sampler(spp) {}
	virtual ~RandomSampler() = default;

	// Returns the 2D offset for the specified sample
	virtual Vec2f getSamplePosition(int n) override
	{
		return random.RandVec2f_0101();
	}

private:
	RandomModule random;
};

class UniformSampler :public Sampler
{
public:
	UniformSampler(int spp) :Sampler(spp)
	{
		width = sqrt(spp);
		step = 1.0 / (width * 2);
	}
	virtual ~UniformSampler() = default;

	// Returns the 2D offset for the specified sample
	virtual Vec2f getSamplePosition(int n) override
	{
		int i = n / width;
		int j = n % width;

		return Vec2f(step * (1 + i * 2), step * (1 + j * 2));
	}

private:
	int width = 0;
	float step = 0;
};

class JitteredSampler :public Sampler
{
public:
	JitteredSampler(int spp) :Sampler(spp)
	{
		width = sqrt(spp);
		step = 1.0 / width;
	}
	virtual ~JitteredSampler() = default;

	// Returns the 2D offset for the specified sample
	virtual Vec2f getSamplePosition(int n) override
	{
		int i = n / width;
		int j = n % width;

		Vec2f offset = random.RandVec2f_0101();
		offset.Scale(step, step);
		return Vec2f(step * i + offset.x(), step * j + offset.y());
	}

private:
	int width = 0;
	float step = 0;
	RandomModule random;
};