#include "Grid.h"
#include <Windows.h>
#include <gl/GL.h>

#include <RayTracer/Core/RayTree.h>
#include <RayTracer/Core/RayTracingStas.h>
#include <RayTracer/Primitives/Transform.h>
#include <RayTracer/Primitives/Plane.h>

void Grid::InsertPlane(DrawItem& plane)
{
	planes.emplace_back(plane);
}

bool MarchingInfo::nextCell(int& surface)
{
	RayTracingStats::IncrementNumGridCellsTraversed();

	float t_min_next = min(t_next_x, min(t_next_y, t_next_z));
	if (t_min_next == t_next_x)
	{
		t_next_x += d_tx;
		gridTMin = t_next_x;
		i += sign_x;
		surface = (sign_x == 1) ? 0 : 2;
		if (i < 0 || i >= maxX)
			return false;
	}
	else if (t_min_next == t_next_y)
	{
		t_next_y += d_ty;
		gridTMin = t_next_y;
		j += sign_y;
		surface = (sign_y == 1) ? 5 : 4;
		if (j < 0 || j >= maxY)
			return false;
	}
	else
	{
		t_next_z += d_tz;
		gridTMin = t_next_z;
		k += sign_z;
		surface = (sign_z == 1) ? 3 : 1;
		if (k < 0 || k >= maxZ)
			return false;
	}
	return true;
}

Material invalid = { Vec3f(0, 0, 0) };
Material materials[] =
{
	{ Vec3f(1,1,1) },
	{ Vec3f(1,0,1) },
	{ Vec3f(0.7,0,1) },
	{ Vec3f(0.3,0,1) },
	{ Vec3f(0,0,1) },
	{ Vec3f(0,0.3,1) },
	{ Vec3f(0,0.7,1) },
	{ Vec3f(0,1,1) },
	{ Vec3f(0,1,0.7) },
	{ Vec3f(0,1,0.3) },
	{ Vec3f(0,1,0) },
	{ Vec3f(0.3,1,0) },
	{ Vec3f(0.7,1,0) },
	{ Vec3f(1,1,0) },
	{ Vec3f(1,0.7,0) },
	{ Vec3f(1,0.3,0) },
	{ Vec3f(1,0,0) },
};

Material* GetMaterial(int i)
{
	if (i < 0)
	{
		std::cout << "bug" << std::endl;
		return nullptr;
	}
	else if (i < 17)
	{
		return &materials[i];
	}
	else
		return &materials[16];
}

Grid::Grid(BoundingBox* bb, int nx, int ny, int nz)
	: mBoundingBox(bb), mX(nx), mY(ny), mZ(nz)
{
	VoxelS = new Voxel[nx * ny * nz];

	startX = bb->getMin().x();
	startY = bb->getMin().y();
	startZ = bb->getMin().z();

	stepX = (bb->getMax().x() - startX) / mX;
	stepY = (bb->getMax().y() - startY) / mY;
	stepZ = (bb->getMax().z() - startZ) / mZ;

	for (int i = 0; i < mX; i++)
		for (int j = 0; j < mY; j++)
			for (int k = 0; k < mZ; k++)
			{
				SetVoxelState(i, j, k, false);
			}
}

Grid::~Grid()
{
	delete[] VoxelS;
}

Vec3f GridNormals[] =
{
	{-1.0, +0.0, +0.0},
	{+0.0, +0.0, +1.0},
	{+1.0, +0.0, +0.0},
	{+0.0, +0.0, -1.0},
	{+0.0, +1.0, +0.0},
	{+0.0, -1.0, +0.0},
};

void intersectDrawItem(DrawItem& item, const Ray& r, Hit& h, float tmin, bool& firstIntersected, bool& onceIntersected, float tbmax)
{
	Object3D* object = item.object;
	if (item.matrix != nullptr)
		object = new Transform(*item.matrix, item.object);
	if (object->intersect(r, h, tmin))
	{
		onceIntersected = true;
		if (h.getT() < tbmax)
			firstIntersected = true;
	}
	if (item.matrix != nullptr)
		delete object;
}

bool Grid::intersect(const Ray& r, Hit& h, float tmin)
{
	RayTree::SetMainSegment(r, 0, 50);

	MarchingInfo mi;
	initializeRayMarch(mi, r, tmin);

	bool planeIntersected = false;
	bool firstIntersected = false;
	bool onceIntersected = false;

	for (int i = 0; i < planes.size(); i++)
	{
		intersectDrawItem(planes[i], r, h, tmin, planeIntersected, onceIntersected, 9999999);
	}

	if (mi.IntersectedBB == false)
	{
		return planeIntersected;
	}

	int surface = -1;
	int index = 0;

	if (GetVoxelState(mi.i, mi.j, mi.k))
	{
		std::vector<DrawItem>& items = Grid::GetVoxelItems(mi.i, mi.j, mi.k);
		for (int i = 0; i < items.size(); i++)
		{
			Object3D* object = items[i].object;
			if (items[i].matrix != nullptr)
				object = new Transform(*items[i].matrix, items[i].object);
			if (object->intersect(r, h, tmin))
			{
				onceIntersected = true;
				firstIntersected = true;
			}
			if (items[i].matrix != nullptr)
				delete object;
		}
	}
	
	while (mi.nextCell(surface))
	{
		index++;
		if (firstIntersected == true)
			break;

		if (GetVoxelState(mi.i, mi.j, mi.k))
		{
			std::vector<DrawItem>& items = Grid::GetVoxelItems(mi.i, mi.j, mi.k);
			for (int i = 0; i < items.size(); i++)
			{
				intersectDrawItem(items[i], r, h, tmin, firstIntersected, onceIntersected, mi.gridBegin + mi.GetMinTNext());
			}
		}
	}

	if (onceIntersected == false)
	{
		return false;
	}

	////GetVoxelItemNum(mi.i, mi.j, mi.k)
	//h.set(gridMin, GetMaterial(GetVoxelItemNum(firstX, firstY, firstZ) - 1), GridNormals[nearestSurface], r);

	return true;
}

bool Grid::intersectVisualize(const Ray& r, Hit& h, float tmin)
{
	RayTree::SetMainSegment(r, 0, 50);

	MarchingInfo mi;
	initializeRayMarch(mi, r, tmin);
	if (mi.IntersectedBB == false)
	{
		return false;
	}

	int surface = -1;
	int index = 0;
	bool hitCell = false;
	float gridMin = 9999999;
	int nearestSurface = -1;
	int firstX = -1, firstY = -1, firstZ = -1;

	addVoxel(mi.i, mi.j, mi.k, GetMaterial(index));
	if (GetVoxelState(mi.i, mi.j, mi.k))
	{
		hitCell = true;
		if (mi.gridTMin < gridMin)
		{
			nearestSurface = mi.firstSurface;
			gridMin = mi.gridTMin;
			firstX = mi.i; firstY = mi.j; firstZ = mi.k;
		}
	}
	addVoxelSurface(mi.i, mi.j, mi.k, mi.firstSurface, GetMaterial(index));
	
	while (mi.nextCell(surface))
	{
		index++;
		addVoxelSurface(mi.i, mi.j, mi.k, surface, GetMaterial(index));
		if (GetVoxelState(mi.i, mi.j, mi.k))
		{
			hitCell = true;
			if (mi.gridTMin < gridMin && nearestSurface == -1)
			{
				nearestSurface = surface;
				gridMin = mi.gridTMin;
				firstX = mi.i; firstY = mi.j; firstZ = mi.k;
			}
		}
		addVoxel(mi.i, mi.j, mi.k, GetMaterial(index));
	}

	if (hitCell == false)
	{
		return false;
	}

	//GetVoxelItemNum(mi.i, mi.j, mi.k)
	h.set(gridMin + mi.gridBegin, GetMaterial(GetVoxelItemNum(firstX, firstY, firstZ) - 1), GridNormals[nearestSurface], r);

	return true;
}

void Grid::paint(void)
{
	materials[0].glSetMaterial();

	for (int i = 0; i < mX; i++)
		for (int j = 0; j < mY; j++)
			for (int k = 0; k < mZ; k++)
			{
				if (GetVoxelState(i, j, k))
				{
					GetMaterial(GetVoxelItemNum(i, j, k) - 1)->glSetMaterial();
					paintVoxel(i, j, k);
				}
			}
}

Vec3f Grid::GetVoxelCenter(int x, int y, int z)
{
	return Vec3f(
		startX + stepX * (x + 0.5),
		startY + stepY * (y + 0.5),
		startZ + stepZ * (z + 0.5));
}

std::vector<DrawItem>& Grid::GetVoxelItems(int x, int y, int z)
{
	return VoxelS[x * mY * mZ + y * mZ + z].Items;
}

Vec3f Grid::GetVoxelSizeHalf()
{
	return Vec3f(stepX * 0.5, stepY * 0.5, stepZ * 0.5);
}

void Grid::SetVoxelState(int x, int y, int z, bool state)
{
	VoxelS[x * mY * mZ + y * mZ + z].IsOpaque = state;
}

void Grid::InsertVoxelItem(int x, int y, int z, DrawItem& item)
{
	VoxelS[x * mY * mZ + y * mZ + z].Items.emplace_back(item);
}

bool Grid::GetVoxelState(int x, int y, int z)
{
	return VoxelS[x * mY * mZ + y * mZ + z].IsOpaque;
}

void Grid::GetGridSegments(int& x, int& y, int& z)
{
	x = mX;
	y = mY;
	z = mZ;
}

void Grid::paintVoxel(int& x, int& y, int& z)
{
	static int indices[] =
	{
		0,1,3,2,
		1,5,7,3,
		5,4,6,7,
		4,0,2,6,
		2,3,7,6,
		5,1,0,4,
	};

	Vec3f center = GetVoxelCenter(x, y, z);
	Vec3f points[] =
	{
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
	};

	static Vec3f normals[6];
	
	for (int i = 0; i < 6; i++)
	{
		glBegin(GL_QUADS);

		Vec3f edge1 = points[indices[i * 4 + 2]] - points[indices[i * 4 + 0]];
		Vec3f edge2 = points[indices[i * 4 + 1]] - points[indices[i * 4 + 0]];
		Vec3f normal;
		Vec3f::Cross3(normal, edge1, edge2);
		normal.Normalize();

		glNormal3f(-normal.x(), -normal.y(), -normal.z());
		glVertex3f(points[indices[i * 4 + 0]].x(), points[indices[i * 4 + 0]].y(), points[indices[i * 4 + 0]].z());
		glVertex3f(points[indices[i * 4 + 1]].x(), points[indices[i * 4 + 1]].y(), points[indices[i * 4 + 1]].z());
		glVertex3f(points[indices[i * 4 + 2]].x(), points[indices[i * 4 + 2]].y(), points[indices[i * 4 + 2]].z());
		glVertex3f(points[indices[i * 4 + 3]].x(), points[indices[i * 4 + 3]].y(), points[indices[i * 4 + 3]].z());
		glEnd();
	}
}

void Grid::addVoxel(int& x, int& y, int& z, Material* mat)
{
	static int indices[] =
	{
		0,1,3,2,
		1,5,7,3,
		5,4,6,7,
		4,0,2,6,
		2,3,7,6,
		5,1,0,4,
	};

	Vec3f center = GetVoxelCenter(x, y, z);
	Vec3f points[] =
	{
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
	};

	static Vec3f normals[6];
	
	for (int i = 0; i < 6; i++)
	{
		Vec3f edge1 = points[indices[i * 4 + 2]] - points[indices[i * 4 + 0]];
		Vec3f edge2 = points[indices[i * 4 + 1]] - points[indices[i * 4 + 0]];
		Vec3f normal;
		Vec3f::Cross3(normal, edge1, edge2);
		normal.Normalize();

		RayTree::AddHitCellFace(
			points[indices[i * 4 + 0]],
			points[indices[i * 4 + 1]],
			points[indices[i * 4 + 2]],
			points[indices[i * 4 + 3]],
			normal, mat);
	}
}

void Grid::addVoxelSurface(int& x, int& y, int& z, int surface, Material* mat)
{
	static int indices[] =
	{
		0,1,3,2,
		1,5,7,3,
		5,4,6,7,
		4,0,2,6,
		2,3,7,6,
		5,1,0,4,
	};

	Vec3f center = GetVoxelCenter(x, y, z);
	Vec3f points[] =
	{
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(-stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,-stepY * 0.5,+stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,-stepZ * 0.5),
		center + Vec3f(+stepX * 0.5,+stepY * 0.5,+stepZ * 0.5),
	};

	static Vec3f normals[6];
	
	int i = surface;
	Vec3f edge1 = points[indices[i * 4 + 2]] - points[indices[i * 4 + 0]];
	Vec3f edge2 = points[indices[i * 4 + 1]] - points[indices[i * 4 + 0]];
	Vec3f normal;
	Vec3f::Cross3(normal, edge1, edge2);
	normal.Normalize();

	RayTree::AddEnteredFace(
		points[indices[i * 4 + 0]],
		points[indices[i * 4 + 1]],
		points[indices[i * 4 + 2]],
		points[indices[i * 4 + 3]],
		normal, mat);
}

bool Between(const Vec3f& min, const Vec3f& max, const Vec3f& value)
{
	if (value.x() > min.x() && value.x() < max.x() &&
		value.y() > min.y() && value.y() < max.y() &&
		value.z() > min.z() && value.z() < max.z())
		return true;
	else
		return false;
}

bool Intersect(const Ray& ray, const Vec3f& min, const Vec3f& max)
{
	Vec3f hitPoint;
	const Vec3f& origin = ray.getOrigin(); 
	const Vec3f& dir = ray.getDirection(); 

	float t;

	float ox = origin.x(); float oy = origin.y(); float oz = origin.z();
	float dx = dir.x(); float dy = dir.y(); float dz = dir.z();
	float tx_min, ty_min, tz_min;
	float tx_max, ty_max, tz_max;

	if (abs(dx) < 0.000001f)
	{
		if (ox < min.x() || ox > max.x())
			return false;
		else
			return true;
	}
	else
	{
		if (dx >= 0)
		{
			tx_min = (min.x() - ox) / dx;
			tx_max = (max.x() - ox) / dx;
		}
		else
		{
			tx_min = (max.x() - ox) / dx;
			tx_max = (min.x() - ox) / dx;
		}

	}

	if (abs(dy) < 0.000001f)
	{
		if (oy < min.y() || oy > max.y())
			return false;
		else
			return true;
	}
	else
	{
		if (dy >= 0)
		{
			ty_min = (min.y() - oy) / dy;
			ty_max = (max.y() - oy) / dy;
		}
		else
		{
			ty_min = (max.y() - oy) / dy;
			ty_max = (min.y() - oy) / dy;
		}

	}

	if (abs(dz) < 0.000001f)
	{
		if (oz < min.z() || oz > max.z())
			return false;
		else
			return true;
	}
	else
	{
		if (dz >= 0)
		{
			tz_min = (min.z() - oz) / dz;
			tz_max = (max.z() - oz) / dz;
		}
		else
		{
			tz_min = (max.z() - oz) / dz;
			tz_max = (min.z() - oz) / dz;
		}

	}

	float t0, t1;
	t0 = max(tz_min, max(tx_min, ty_min));
	t1 = min(tz_max, min(tx_max, ty_max));

	return t0 < t1;
}

void Grid::initializeRayMarch(MarchingInfo& mi, const Ray& r, float tmin) const
{
	Vec3f min = boundingBox->getMin();
	Vec3f max = boundingBox->getMax();
	Vec3f origin = r.getOrigin();
	mi.maxX = mX;
	mi.maxY = mY;
	mi.maxZ = mZ;
	mi.firstSurface = -1;

	// CASE1: The origin is inside the grid
	if (Between(min, max, origin))
	{
		mi.IntersectedBB = true;

		Vec3f offset = origin - min;
		mi.i = floor(offset.x() / stepX);
		mi.j = floor(offset.y() / stepY);
		mi.k = floor(offset.z() / stepZ);

		mi.d_tx = abs(stepX / r.getDirection().x());
		mi.d_ty = abs(stepY / r.getDirection().y());
		mi.d_tz = abs(stepZ / r.getDirection().z());

		mi.sign_x = r.getDirection().x() > 0 ? 1 : -1;
		mi.sign_y = r.getDirection().y() > 0 ? 1 : -1;
		mi.sign_z = r.getDirection().z() > 0 ? 1 : -1;

		int d_next_x = (mi.sign_x == 1) ? ceil(offset.x() / stepX) : floor(offset.x() / stepX);
		int d_next_y = (mi.sign_y == 1) ? ceil(offset.y() / stepY) : floor(offset.y() / stepY);
		int d_next_z = (mi.sign_z == 1) ? ceil(offset.z() / stepZ) : floor(offset.z() / stepZ);

		mi.t_next_x = abs((d_next_x * stepX - offset.x()) / r.getDirection().x());
		mi.t_next_y = abs((d_next_y * stepY - offset.y()) / r.getDirection().y());
		mi.t_next_z = abs((d_next_z * stepZ - offset.z()) / r.getDirection().z());

		Vec3f miNextX = origin + r.getDirection() * mi.t_next_x;
		Vec3f miNextY = origin + r.getDirection() * mi.t_next_y;
		Vec3f miNextZ = origin + r.getDirection() * mi.t_next_z;
	}

	// CASE2: The origin is outside & ray hits the grid
	else if (Intersect(r, min, max))
	{
		mi.IntersectedBB = true;

		Vec3f Nearest(-999, -999, -999);
		Vec3f hitPoint;
		float t, tmin = 999999;
		mi.gridTMin = 999999;
		mi.gridBegin = 999999;
		if (r.getDirection().x() != 0.f)
		{
			if (r.getDirection().x() > 0)
				t = (min.x() - origin.x()) / r.getDirection().x();
			else
				t = (max.x() - origin.x()) / r.getDirection().x();

			if (t > 0.f)
			{
				hitPoint = origin + (t + 0.001) * r.getDirection();
				if (min.y() < hitPoint.y() && hitPoint.y() < max.y() && min.z() < hitPoint.z() && hitPoint.z() < max.z())
				{
					Nearest = hitPoint;
					mi.gridTMin = 0;
					mi.gridBegin = t;
					mi.firstSurface = (r.getDirection().x() > 0) ? 0 : 2;
				}
			}
		}

		if (r.getDirection().y() != 0.f)
		{
			if (r.getDirection().y() > 0)
				t = (min.y() - origin.y()) / r.getDirection().y();
			else
				t = (max.y() - origin.y()) / r.getDirection().y();

			if (t > 0.f)
			{
				hitPoint = origin + (t + 0.001) * r.getDirection();

				if (min.z() < hitPoint.z() && hitPoint.z() < max.z() && min.x() < hitPoint.x() && hitPoint.x() < max.x())
				{
					if (t < mi.gridBegin)
					{
						Nearest = hitPoint;
						mi.gridTMin = 0;
						mi.gridBegin = t;
						mi.firstSurface = (r.getDirection().y() > 0) ? 5 : 4;
					}
				}
			}
		}

		if (r.getDirection().z() != 0.f)
		{
			if (r.getDirection().z() > 0)
				t = (min.z() - origin.z()) / r.getDirection().z();
			else
				t = (max.z() - origin.z()) / r.getDirection().z();

			if (t > 0.f)
			{
				hitPoint = origin + (t + 0.001) * r.getDirection();

				if (min.x() < hitPoint.x() && hitPoint.x() < max.x() && min.y() < hitPoint.y() && hitPoint.y() < max.y())
				{
					if (t < mi.gridBegin)
					{
						Nearest = hitPoint;
						mi.gridTMin = 0;
						mi.gridBegin = t;
						mi.firstSurface = (r.getDirection().z() > 0) ? 3 : 1;
					}
				}
			}
		}

		origin = Nearest;
		Vec3f offset = origin - min;
		mi.i = floor(offset.x() / stepX);
		mi.j = floor(offset.y() / stepY);
		mi.k = floor(offset.z() / stepZ);

		mi.d_tx = abs(stepX / r.getDirection().x());
		mi.d_ty = abs(stepY / r.getDirection().y());
		mi.d_tz = abs(stepZ / r.getDirection().z());

		mi.sign_x = r.getDirection().x() > 0 ? 1 : -1;
		mi.sign_y = r.getDirection().y() > 0 ? 1 : -1;
		mi.sign_z = r.getDirection().z() > 0 ? 1 : -1;

		int d_next_x = (mi.sign_x == 1) ? ceil(offset.x() / stepX) : floor(offset.x() / stepX);
		int d_next_y = (mi.sign_y == 1) ? ceil(offset.y() / stepY) : floor(offset.y() / stepY);
		int d_next_z = (mi.sign_z == 1) ? ceil(offset.z() / stepZ) : floor(offset.z() / stepZ);

		mi.t_next_x = abs((d_next_x * stepX - offset.x()) / r.getDirection().x());
		mi.t_next_y = abs((d_next_y * stepY - offset.y()) / r.getDirection().y());
		mi.t_next_z = abs((d_next_z * stepZ - offset.z()) / r.getDirection().z());

		if (mi.i < 0 || mi.j < 0 || mi.k < 0 || mi.i >= mi.maxX || mi.j >= mi.maxY || mi.k >= mi.maxZ)
		{
			float t_min_next = min(mi.t_next_x, min(mi.t_next_y, mi.t_next_z));
			if (t_min_next == mi.t_next_x)
			{
				mi.gridTMin = mi.t_next_x;
				mi.t_next_x += mi.d_tx;
				mi.i += mi.sign_x;
				mi.firstSurface = (mi.sign_x == 1) ? 0 : 2;
				if (mi.i < 0 || mi.i >= mi.maxX)
					mi.IntersectedBB = false;
			}
			else if (t_min_next == mi.t_next_y)
			{
				mi.gridTMin = mi.t_next_y;
				mi.t_next_y += mi.d_ty;
				mi.j += mi.sign_y;
				mi.firstSurface = (mi.sign_y == 1) ? 5 : 4;
				if (mi.j < 0 || mi.j >= mi.maxY)
					mi.IntersectedBB = false;
			}
			else
			{
				mi.gridTMin = mi.t_next_z;
				mi.t_next_z += mi.d_tz;
				mi.k += mi.sign_z;
				mi.firstSurface = (mi.sign_z == 1) ? 3 : 1;
				if (mi.k < 0 || mi.k >= mi.maxZ)
					mi.IntersectedBB = false;
			}

		}

		if (mi.gridTMin == 999999 || mi.firstSurface == -1)
		{
			mi.IntersectedBB = false;
		}

	}

	// CASE3: The origin is outside & ray misses the grid
	else
	{
		mi.IntersectedBB = false;
	}
}

int Grid::GetVoxelItemNum(int x, int y, int z)
{
	return VoxelS[x * mY * mZ + y * mZ + z].Items.size();
}