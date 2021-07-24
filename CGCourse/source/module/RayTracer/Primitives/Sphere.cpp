#include "Sphere.h"
#include "../VersionControl.h"

Sphere::Sphere(Vec3f center, float radius, Material* mat)
	:center(center), radius(radius)
{
	this->mat = mat;
}

bool Sphere::intersect(const Ray& r, Hit& h, float tmin)
{
	const Vec3f& origin = r.getOrigin();
	const Vec3f& direction = r.getDirection();

	Vec3f connection = center - origin;
	float distance = connection.Length();

	// Case 1
	// The start-point of the ray is inside the sphere
	// Absolutely 1 intersection would happen
	bool inside = distance < radius;

	// Case 2
	// The start-point of the ray is outside the sphere
	float projection = (center - origin).Dot3(direction);
	// If the sphere is behind the ray, no intersection would happen
	if ((projection < tmin) && (!inside))
		return false;

	// Case 3
	Vec3f projected = r.pointAtParameter(projection);
	// If the sphere is too far away from the ray
	float mindis = (center - projected).Length();
	if ((mindis > radius) && (!inside))
		return false;

	// Case 4
	// Absolutely 2 intersection would happen
	float nearest = (float)sqrt(radius * radius - mindis * mindis);
	float minus = projection - nearest;
	if (minus > tmin)
		nearest = minus;
	else
		nearest = projection + nearest;

	if (nearest < h.getT())
	{
#if(RTVersion==1)
		h.set(nearest, this->mat, r);
#endif
#if(RTVersion==2)
		Vec3f hitpoint = r.pointAtParameter(nearest);
		Vec3f normal = hitpoint - center;
		normal.Normalize();
		h.set(nearest, this->mat, normal, r);
#endif
	}

	// Do intersect
	return true;
}

void Sphere::paint(void)
{

}