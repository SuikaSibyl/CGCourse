#include "Sphere.h"
#include "../VersionControl.h"
#include <Windows.h>
#include <gl/GL.h>
#include "Grid.h"
#include <matrix.h>
#include <RayTracer/Core/RayTracingStas.h>

int tessx = 0, tessy = 0;
bool gouraud = false;

Sphere::Sphere(Vec3f center, float radius, Material* mat)
	:center(center), radius(radius)
{
	this->mat = mat;
	BuildMesh();

	boundingBox = new BoundingBox(
		center - Vec3f(radius, radius, radius),
		center + Vec3f(radius, radius, radius));
}

Vec3f Sphere::getPoint(float u, float v)
{
	float r = 0.9f;
	float pi = M_PI;
	float y = r * std::cos(pi * u);
	float x = r * std::sin(pi * u) * std::cos(2 * pi * v);
	float z = r * std::sin(pi * u) * std::sin(2 * pi * v);

	x *= radius;
	y *= radius;
	z *= radius;

	return Vec3f(x, y, z) + center;
}

void Sphere::BuildMesh()
{
	int Longitude = tessx;
	int Latitude = tessy;
	float lon_step = 1.0f / Longitude;
	float lat_step = 1.0f / Latitude;
	int offset = 0;

	num = Latitude * Longitude * 6;
	points = new Vec3f[num];
	normals = new Vec3f[num];
	
	for (int lat = 0; lat < Latitude; lat++) {  // 纬线u
		for (int lon = 0; lon < Longitude; lon++) { // 经线v
			// 一次构造4个点，两个三角形，
			Vec3f point1 = getPoint(lat * lat_step, lon * lon_step);
			Vec3f point2 = getPoint((lat + 1) * lat_step, lon * lon_step);
			Vec3f point3 = getPoint((lat + 1) * lat_step, (lon + 1) * lon_step);
			Vec3f point4 = getPoint(lat * lat_step, (lon + 1) * lon_step);

			if (gouraud)
			{
				Vec3f normal1, normal2, normal3, normal4;
				normal1 = (center - point1);  normal1.Normalize();
				normal2 = (center - point2);  normal2.Normalize();
				normal3 = (center - point3);  normal3.Normalize();
				normal4 = (center - point4);  normal4.Normalize();

				memcpy(normals + offset + 0, &normal1, 3 * sizeof(float));
				memcpy(normals + offset + 1, &normal3, 3 * sizeof(float));
				memcpy(normals + offset + 2, &normal4, 3 * sizeof(float));
				memcpy(normals + offset + 3, &normal1, 3 * sizeof(float));
				memcpy(normals + offset + 4, &normal2, 3 * sizeof(float));
				memcpy(normals + offset + 5, &normal3, 3 * sizeof(float));
			}
			else
			{
				Vec3f e1 = point2 - point1;
				Vec3f e2 = point3 - point1;
				Vec3f normal;
				Vec3f::Cross3(normal, e2, e1);

				memcpy(normals + offset + 0, &normal, 3 * sizeof(float));
				memcpy(normals + offset + 1, &normal, 3 * sizeof(float));
				memcpy(normals + offset + 2, &normal, 3 * sizeof(float));
				memcpy(normals + offset + 3, &normal, 3 * sizeof(float));
				memcpy(normals + offset + 4, &normal, 3 * sizeof(float));
				memcpy(normals + offset + 5, &normal, 3 * sizeof(float));
			}

			memcpy(points + offset, &point1, 3 * sizeof(float));
			offset += 1;
			memcpy(points + offset, &point3, 3 * sizeof(float));
			offset += 1;
			memcpy(points + offset, &point4, 3 * sizeof(float));
			offset += 1;

			memcpy(points + offset, &point1, 3 * sizeof(float));
			offset += 1;
			memcpy(points + offset, &point2, 3 * sizeof(float));
			offset += 1;
			memcpy(points + offset, &point3, 3 * sizeof(float));
			offset += 1;
		}
	}
}

bool Sphere::intersect(const Ray& r, Hit& h, float tmin)
{
	RayTracingStats::IncrementNumIntersections();

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
#if(RTVersion>=2)
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
	mat->glSetMaterial();
	glBegin(GL_TRIANGLES);

	for (int i = 0; i < num; i += 3)
	{
		glNormal3f(normals[i + 0].x(), normals[i + 0].y(), normals[i + 0].z());
		glVertex3f(points[i + 0].x(), points[i + 0].y(), points[i + 0].z());
		glNormal3f(normals[i + 1].x(), normals[i + 1].y(), normals[i + 1].z());
		glVertex3f(points[i + 1].x(), points[i + 1].y(), points[i + 1].z());
		glNormal3f(normals[i + 2].x(), normals[i + 2].y(), normals[i + 2].z());
		glVertex3f(points[i + 2].x(), points[i + 2].y(), points[i + 2].z());
	}
	glEnd();
}

void Sphere::insertIntoGrid(Grid* g, Matrix* m)
{
	int x, y, z;
	g->GetGridSegments(x, y, z);

	for (int i = 0; i < x; i++)
		for (int j = 0; j < y; j++)
			for (int k = 0; k < z; k++)
			{
				Vec3f center = this->center;
				if (m != nullptr) m->Transform(center);
				Vec3f offset = g->GetVoxelCenter(i, j, k) - center;

				float half = g->GetVoxelSizeHalf().Length();
				float radius = this->radius;
				if (m != nullptr)
				{
					Vec3f dir = offset;
					dir.Normalize();
					dir = radius * dir;

					m->TransformDirection(dir);
					radius = dir.Length();
				}
				if (offset.Length() - half < radius)
				{
					g->SetVoxelState(i, j, k, true);
					DrawItem item = { (Object3D*)this, m };
					g->InsertVoxelItem(i, j, k, item);
				}
			}
}