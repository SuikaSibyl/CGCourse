#include "Triangle.h"
#include <Windows.h>
#include <gl/GL.h>
#include <matrix.h>
#include "Grid.h"
#include <RayTracer/Core/RayTracingStas.h>

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

	Vec3f minAB, minABC;
	Vec3f maxAB, maxABC;
	Vec3f::Min(minAB, a, b); Vec3f::Min(minABC, c, minAB);
	Vec3f::Max(maxAB, a, b); Vec3f::Max(maxABC, c, maxAB);
	this->boundingBox = new BoundingBox(minABC, maxABC);
}

bool Triangle::intersect(const Ray& r, Hit& h, float tmin)
{
	RayTracingStats::IncrementNumIntersections();

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

#if(RTVersion>=2)
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

#define MAX3(a,b,c) (max(a,max(b,c)))
#define MIN3(a,b,c) (min(a,min(b,c)))

bool Triangle::TriangleAABB(Triangle* triangle, const Vec3f& center, const Vec3f& extents, Matrix* m)
{
	// Get the triangle points as vectors
	Vec3f v0 = triangle->p1;
	Vec3f v1 = triangle->p2;
	Vec3f v2 = triangle->p3;

	if (m != nullptr)
	{
		m->Transform(v0);
		m->Transform(v1);
		m->Transform(v2);
	}

	v0 = v0 - center;
	v1 = v1 - center;
	v2 = v2 - center;

	// Compute the edge vectors of the triangle  (ABC)
	// That is, get the lines between the points as vectors
	Vec3f f0 = v1 - v0; // B - A
	Vec3f f1 = v2 - v1; // C - B
	Vec3f f2 = v0 - v2; // A - C

	// Compute the face normals of the AABB, because the AABB
	// is at center, and of course axis aligned, we know that 
	// it's normals are the X, Y and Z axis.
	Vec3f u0(1.0f, 0.0f, 0.0f);
	Vec3f u1(0.0f, 1.0f, 0.0f);
	Vec3f u2(0.0f, 0.0f, 1.0f);


	// There are a total of 13 axis to test!

	// We first test against 9 axis, these axis are given by
	// cross product combinations of the edges of the triangle
	// and the edges of the AABB. You need to get an axis testing
	// each of the 3 sides of the AABB against each of the 3 sides
	// of the triangle. The result is 9 axis of seperation
	// https://awwapp.com/b/umzoc8tiv/

	// Compute the 9 axis
	Vec3f axis[13];
	Vec3f::Cross3(axis[0], u0, f0); axis[0].Normalize();
	Vec3f::Cross3(axis[1], u0, f1);	axis[1].Normalize();
	Vec3f::Cross3(axis[2], u0, f2);	axis[2].Normalize();
	Vec3f::Cross3(axis[3], u1, f0);	axis[3].Normalize();
	Vec3f::Cross3(axis[4], u1, f1);	axis[4].Normalize();
	Vec3f::Cross3(axis[5], u1, f2);	axis[5].Normalize();
	Vec3f::Cross3(axis[6], u2, f0);	axis[6].Normalize();
	Vec3f::Cross3(axis[7], u2, f1);	axis[7].Normalize();
	Vec3f::Cross3(axis[8], u2, f2);	axis[8].Normalize();

	// Testing axis: axis_u0_f0
	// Project all 3 vertices of the triangle onto the Seperating axis
	axis[9] = Vec3f(1, 0, 0);
	axis[10] = Vec3f(0, 1, 0);
	axis[11] = Vec3f(0, 0, 1);

	// Finally, we have one last axis to test, the face normal of the triangle
	// We can get the normal of the triangle by crossing the first two line segments
	Vec3f::Cross3(axis[12], f0, f1); axis[12].Normalize();

	for (int i = 0; i < 13; i++)
	{
		// Testing axis: axis_u0_f0
		// Project all 3 vertices of the triangle onto the Seperating axis
		float p0 = v0.Dot3(axis[i]);
		float p1 = v1.Dot3(axis[i]);
		float p2 = v2.Dot3(axis[i]);

		float r = extents.x() * abs(u0.Dot3(axis[i])) +
			extents.y() * abs(u1.Dot3(axis[i])) +
			extents.z() * abs(u2.Dot3(axis[i]));
		// Now do the actual test, basically see if either of
		// the most extreme of the triangle points intersects r
		// You might need to write Min & Max functions that take 3 arguments
		if (max(-MAX3(p0, p1, p2), MIN3(p0, p1, p2)) > r + 0.000001) {
			// This means BOTH of the points of the projected triangle
			// are outside the projected half-length of the AABB
			// Therefore the axis is seperating and we can exit
			return false;
		}
	}
	return true;
}

void Triangle::insertIntoGrid(Grid* g, Matrix* m)
{
	int x, y, z;
	g->GetGridSegments(x, y, z);
	Vec3f extent = g->GetVoxelSizeHalf();
	for (int i = 0; i < x; i++)
		for (int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
			{
				Vec3f center = g->GetVoxelCenter(i, j, k);
				if (TriangleAABB(this, center, extent, m))
				{
					g->SetVoxelState(i, j, k, true);
					DrawItem item = { (Object3D*)this, m };
					g->InsertVoxelItem(i, j, k, item);
				}
			}
}