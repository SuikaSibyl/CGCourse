#pragma once

#include "ray.h"

class Camera
{
public:
	virtual Ray generateRay(Vec2f point) = 0;
	virtual float getTMin() const = 0;
	virtual void RefreshCamera() = 0;
	// GL Camera Interface
	virtual void glInit(int w, int h) = 0;
	virtual void glPlaceCamera(void) = 0;
	virtual void dollyCamera(float dist) = 0;
	virtual void truckCamera(float dx, float dy) = 0;
	virtual void rotateCamera(float rx, float ry) = 0;
};

class OrthographicCamera :public Camera
{
public:
	OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size);

	virtual void RefreshCamera();

	virtual Ray generateRay(Vec2f point) override;
	virtual float getTMin() const override;

	virtual void glInit(int w, int h) override;
	virtual void glPlaceCamera(void) override;
	virtual void dollyCamera(float dist) override;
	virtual void truckCamera(float dx, float dy) override;
	virtual void rotateCamera(float rx, float ry) override;

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

	virtual void RefreshCamera();

	virtual Ray generateRay(Vec2f point) override;
	virtual float getTMin() const override;

	virtual void glInit(int w, int h) override;
	virtual void glPlaceCamera(void) override;
	virtual void dollyCamera(float dist) override;
	virtual void truckCamera(float dx, float dy) override;
	virtual void rotateCamera(float rx, float ry) override;

private:
	Vec3f center;
	Vec3f direction;
	Vec3f up;
	Vec3f horizontal;
	Vec3f leftbottom;

	float width;
	float height;
	float angle;
};