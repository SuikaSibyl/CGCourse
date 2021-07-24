#pragma once

#include "../Core/Object3D.h"
#include <vector>

class Group :public Object3D
{
public:
	Group(int num);
	~Group();
	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;
	virtual void paint(void) override;
	void addObject(int index, Object3D* obj);

private:
	int objnum;
	Object3D** list;
};