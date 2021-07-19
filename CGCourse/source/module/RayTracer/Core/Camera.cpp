#include "Camera.h"
#include <limits>

#define LARGE_DISTANCE 999

OrthographicCamera::OrthographicCamera(Vec3f center, Vec3f direction, Vec3f up, float size)
	:size(size)
	,center(center)
{
	direction.Normalize();
	up.Normalize();

	this->direction = direction;
	Vec3f::Cross3(this->horizontal, direction, up);
	Vec3f::Cross3(this->up, this->horizontal, direction);

	this->horizontal *= size;
	this->up		 *= size;
	leftbottom = this->center - 0.5 * this->horizontal - 0.5 * this->up;
}

Ray OrthographicCamera::generateRay(Vec2f point)
{
	Vec3f orig = leftbottom
		+ point.x() * this->horizontal
		+ point.y() * this->up;
	return Ray(orig, direction);
}

float OrthographicCamera::getTMin() const
{
	return numeric_limits<float>::lowest();
}