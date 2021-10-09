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
	if (boundingBox == nullptr && obj->getBoundingBox() != nullptr)
	{
		boundingBox = new BoundingBox(*obj->getBoundingBox());
	}
	else if (obj->getBoundingBox() != nullptr)
	{
		Vec3f min;
		Vec3f max;
		Vec3f minx = boundingBox->getMin();
		Vec3f maxx = boundingBox->getMax();
		Vec3f::Min(min, obj->getBoundingBox()->getMin(), minx);
		Vec3f::Max(max, obj->getBoundingBox()->getMax(), maxx);
		delete boundingBox;
		boundingBox = new BoundingBox(min, max);
	}
}

void Group::paint(void)
{
	for (int i = 0; i < objnum; i++)
	{
		if (list[i] != NULL)
		{
			list[i]->paint();
		}
	}
}

void Group::insertIntoGrid(Grid* g, Matrix* m)
{
	for (int i = 0; i < objnum; i++)
	{
		if (i % 1000 == 0)
		{
			std::cout << i * 100.0 / objnum << "%" << std::endl;
		}
		if (list[i] != NULL)
		{
			list[i]->insertIntoGrid(g, m);
		}
	}
}