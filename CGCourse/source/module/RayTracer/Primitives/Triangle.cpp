#include "Triangle.h"
#include <Windows.h>
#include <gl/GL.h>

Triangle::Triangle(Vec3f& a, Vec3f& b, Vec3f& c, Material* m)
	:p1(a), p2(b), p3(c)
{
	this->mat = m;
	// Calc edge
	e1 = p2 - p1;
	e2 = p3 - p1;
	// Calc normal
	Vec3f::Cross3(this->normal, e1, e2);
	this->normal.Normalize();
	// Calc distance
	this->distance = p1.Dot3(this->normal);
}

bool Triangle::intersect(const Ray& r, Hit& h, float tmin)
{
	const Vec3f& origin = r.getOrigin();
	const Vec3f& direction = r.getDirection();

	// On the other way, intersection point is needed
	Vec3f T = origin - p1;
	Vec3f P, Q;
	Vec3f::Cross3(P, direction, e2);
	Vec3f::Cross3(Q, T, e1);
	float ipe1 = 1. / P.Dot3(e1);
	float t = ipe1 * Q.Dot3(e2);
	// If the intersection point is outside
	if (t < tmin || t > h.getT()) return false;
	// Calc the u & v
	float u = ipe1 * P.Dot3(T);
	float v = ipe1 * Q.Dot3(direction);
	// If the intersction point is outside the triangle
	if (u < 0 || v < 0 || u + v>1)
		return false;

#if(RTVersion==2 | RTVersion==3)
	h.set(t, this->mat, normal, r);
#endif

	return true;
}

void Triangle::paint(void)
{
	mat->glSetMaterial();
	glBegin(GL_TRIANGLES);
	glNormal3f(this->normal.x(), this->normal.y(), this->normal.z());
	glVertex3f(p1.x(), p1.y(), p1.z());
	glVertex3f(p2.x(), p2.y(), p2.z());
	glVertex3f(p3.x(), p3.y(), p3.z());
	glEnd();
}