#pragma once

#include "Random.h"

class ForceField
{
public:
	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) = 0;

};

class GravityForceField :public ForceField
{
public:
	GravityForceField(Vec3f gravity)
		:gravity(gravity)
	{

	}

	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) override
	{
		return gravity;
	}

private:
	Vec3f gravity;
};

class ConstantForceField :public ForceField
{
public:
	ConstantForceField(Vec3f force)
		:force(force)
	{

	}

	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) override
	{
		return force * (1. / mass);
	}

private:
	Vec3f force;
};

class RadialForceField :public ForceField
{
public:
	RadialForceField(float magnitude)
		:magnitude(magnitude)
	{

	}

	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) override
	{
		Vec3f direction = -1. * position;
		direction.Normalize();
		return (1. / mass) * position.Length() * magnitude * direction;
	}

private:
	float magnitude;
};

class VerticalForceField :public ForceField
{
public:
	VerticalForceField(float magnitude)
		:magnitude(magnitude)
	{

	}

	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) override
	{
		Vec3f direction = position.y() > 0 ? Vec3f(0, -1, 0) : Vec3f(0, 1, 0);
		return (1. / mass) * abs(position.y()) * magnitude * direction;
	}

private:
	float magnitude;
};

class WindForceField :public ForceField
{
public:
	WindForceField(float magnitude)
		:magnitude(magnitude)
	{
		strength = Vec3f(1, 0, 0);
	}

	virtual Vec3f getAcceleration(const Vec3f& position, float mass, float t) override
	{
		static float lasttime = 0;
		if (lasttime != t)
		{
			lasttime = t;
			strength += random.randomVector();
			strength.Normalize();
		}

		return (1. / mass) * magnitude * strength;
	}

protected:
	float magnitude;
	Random random;
	Vec3f strength;
};