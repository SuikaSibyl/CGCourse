#pragma once

#include "ray.h"

class Camera
{
public:
	virtual Ray generateRay(Vec2f point) = 0;
	virtual float getTMin() const = 0;
};

class OrthographicCamera :public Camera
{
public:
	OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size);

	virtual Ray generateRay(Vec2f point) override;
	virtual float getTMin() const override;

private:
	Vec3f center;
	Vec3f direction;
	Vec3f up;
	Vec3f horizontal;
	Vec3f leftbottom;
	float size;
};


class PerspectiveCamera :public Camera
{
public:
	PerspectiveCamera(Vec3f center, Vec3f& direction, Vec3f& up, float angle);

	virtual Ray generateRay(Vec2f point) override;
	virtual float getTMin() const override;

private:
	Vec3f center;
	Vec3f direction;
	Vec3f up;
	Vec3f horizontal;
	Vec3f leftbottom;
	float angle;
};