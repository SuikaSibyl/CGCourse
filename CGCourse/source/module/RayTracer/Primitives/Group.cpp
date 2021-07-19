#include "Group.h"

Group::Group(int num) :
	objnum(num)
{
	list = new Object3D* [num];
}

Group::~Group()
{
	delete[] list;
}

bool Group::intersect(const Ray& r, Hit& h, float tmin)
{
	bool intersected = false;
	for (int i = 0; i < objnum; i++)
	{
		if (list[i] != NULL)
		{
			intersected = list[i]->intersect(r, h, tmin) || intersected;
		}
	}
	return intersected;
}

void Group::addObject(int index, Object3D* obj)
{
	list[index] = obj;
}