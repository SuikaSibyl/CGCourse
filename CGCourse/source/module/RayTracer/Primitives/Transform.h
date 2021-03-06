#pragma once

#include "../Core/Object3D.h"
#include <matrix.h>

class Transform :public Object3D
{
public:
	Transform(Matrix& m, Object3D* o);
	virtual void paint(void) override;
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;
	virtual void insertIntoGrid(Grid* g, Matrix* m) override;

private:
	Matrix matrix;
	Matrix inverse;
	Matrix invTranspose;
	Object3D* Object;
};