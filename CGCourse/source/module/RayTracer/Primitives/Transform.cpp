#include "Transform.h"
#include <Windows.h>
#include <gl/GL.h>

Transform::Transform(Matrix& m, Object3D* o)
	:matrix(m), inverse(m), Object(o)
{
	inverse.Inverse();
	invTranspose = Matrix(inverse);
	invTranspose.Transpose();

	Vec3f min = o->getBoundingBox()->getMin();
	Vec3f max = o->getBoundingBox()->getMax();
	float x[] = { min.x() ,max.x() };
	float y[] = { min.y() ,max.y() };
	float z[] = { min.z() ,max.z() };
	Vec3f minAxis = Vec3f(999, 999, 999);
	Vec3f maxAxis = Vec3f(-999, -999, -999);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			for (int k = 0; k < 2; k++)
			{
				Vec3f point(x[i], y[j], z[k]);
				matrix.Transform(point);
				Vec3f::Min(minAxis, point, minAxis);
				Vec3f::Max(maxAxis, point, maxAxis);
			}

	boundingBox = new BoundingBox(minAxis, maxAxis);
}

bool Transform::intersect(const Ray& r, Hit& h, float tmin)
{
	Vec3f origin = r.getOrigin();
	Vec3f direction = r.getDirection();
	inverse.Transform(origin);
	inverse.TransformDirection(direction);
	float scale = direction.Length();
	direction.Normalize();

	Ray osr(origin, direction);
	Hit hit(99999, nullptr, Vec3f(0, 0, 0));
	if (Object->intersect(osr, hit, tmin / scale))
	{
		float t = hit.getT() / scale;
		if (t > tmin && t < h.getT())
		{
			Vec3f normal = hit.getNormal();
			invTranspose.TransformDirection(normal);
			normal.Normalize();
			h.set(t, hit.getMaterial(), normal, r);
			return true;
		}
	}
	return false;
}

void Transform::insertIntoGrid(Grid* g, Matrix* m)
{
	if (m == nullptr)
	{
		m = new Matrix();
		*m = Matrix::MakeScale(1);
	}
	else
	{
		Matrix* tmp = new Matrix();
		*tmp = *m;
		m = tmp;
	}

	*m *= matrix;
	Object->insertIntoGrid(g, m);
}

void Transform::paint(void)
{
	glPushMatrix();
	GLfloat* glMatrix = matrix.glGet();
	glMultMatrixf(glMatrix);
	Object->paint();
	delete[] glMatrix;
	glPopMatrix();
}