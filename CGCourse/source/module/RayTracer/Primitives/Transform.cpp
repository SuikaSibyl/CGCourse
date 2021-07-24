#include "Transform.h"

Transform::Transform(Matrix& m, Object3D* o)
	:matrix(m), Object(o)
{
	matrix.Inverse();
	invTranspose = Matrix(matrix);
	invTranspose.Transpose();
}

bool Transform::intersect(const Ray& r, Hit& h, float tmin)
{
	Vec3f origin = r.getOrigin();
	Vec3f direction = r.getDirection();
	matrix.Transform(origin);
	matrix.TransformDirection(direction);
	float scale = direction.Length();
	direction.Normalize();

	Ray osr(origin, direction);
	Hit hit((float)numeric_limits<float>::max(), nullptr, Vec3f(0, 0, 0));
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

void Transform::paint(void)
{

}