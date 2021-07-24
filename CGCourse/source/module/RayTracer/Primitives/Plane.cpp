#include "Plane.h"

Plane::Plane(Vec3f& normal, float d, Material* m)
	:distance(d)
{
	normal.Normalize();
	this->normal = normal;
	this->mat = m;
}

bool Plane::intersect(const Ray& r, Hit& h, float tmin)
{
	const Vec3f& origin = r.getOrigin();
	const Vec3f& direction = r.getDirection();

	// Case 1/2/3 - No intersection
	// The ray start-point is inside the plane, no intersection 
	// The ray start-point & direction is on the same side of the plane
	// The ray direction is perpendicular to the plane normal
	float projOrigin = origin.Dot3(normal);
	float projDirection = direction.Dot3(normal);
	if (projDirection * (projOrigin - distance) >= 0)
		return false;

	// Case 4 - One intersection
	float distance = (this->distance - projOrigin) / projDirection;
	if (distance < tmin) return false;
	if (distance < h.getT())
	{
#if(RTVersion==2)
		h.set(distance, this->mat, normal, r);
#endif
	}
}

void Plane::paint(void)
{

}