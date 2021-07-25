#include "Plane.h"
#include <Windows.h>
#include <gl/GL.h>

Plane::Plane(Vec3f& normal, float d, Material* m)
	:distance(d)
{
	normal.Normalize();
	this->normal = normal;
	this->mat = m;

	Vec3f axis1, axis2;
	Vec3f v(1, 0, 0);
	if (normal == Vec3f(1, 0, 0)) v = Vec3f(0, 1, 0);
	Vec3f::Cross3(axis1, v, normal);
	Vec3f::Cross3(axis2, normal, axis1);

	Vec3f zero = normal * distance;

	float big = 9999;
	p1 = zero - big * axis1 - big * axis2;
	p2 = zero - big * axis1 + big * axis2;
	p3 = zero + big * axis1 + big * axis2;
	p4 = zero + big * axis1 - big * axis2;
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
#if(RTVersion==2 | RTVersion==3)
		h.set(distance, this->mat, normal, r);
#endif
	}
}

void Plane::paint(void)
{
	mat->glSetMaterial();
	glBegin(GL_QUADS);
	glNormal3f(this->normal.x(), this->normal.y(), this->normal.z());
	glVertex3f(p1.x(), p1.y(), p1.z());
	glVertex3f(p2.x(), p2.y(), p2.z());
	glVertex3f(p3.x(), p3.y(), p3.z());
	glVertex3f(p4.x(), p4.y(), p4.z());
	glEnd();
}