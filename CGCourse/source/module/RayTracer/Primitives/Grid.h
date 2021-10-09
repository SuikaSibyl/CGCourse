#pragma once

#include "../Core/Object3D.h"
#include <vector>
class Plane;

class MarchingInfo
{
public:
	int i, j, k;
	int maxX, maxY, maxZ;
	int sign_x, sign_y, sign_z;
	float t_next_x, t_next_y, t_next_z;
	float d_tx, d_ty, d_tz;
	float tmin;

	int firstSurface;
	float gridTMin;
	float gridBegin;

	Vec3f Normal;
	bool IntersectedBB = false;

	float GetMinTNext() { return min(t_next_x, min(t_next_y, t_next_z)); }
	bool nextCell(int& surface);
};

struct DrawItem
{
	Object3D* object;
	Matrix* matrix;
};

struct Voxel
{
	bool IsOpaque = false;
	std::vector<DrawItem> Items;
};

class Grid :public Object3D
{
public:
	Grid(BoundingBox* bb, int nx, int ny, int nz);
	~Grid();

	virtual bool intersect(const Ray& r, Hit& h, float tmin) override;
	bool intersectVisualize(const Ray& r, Hit& h, float tmin);
	virtual void paint(void) override;
	void addObject(int index, Object3D* obj);

	Vec3f GetVoxelCenter(int x, int y, int z);
	Vec3f GetVoxelSizeHalf();
	std::vector<DrawItem>& GetVoxelItems(int x, int y, int z);
	void SetVoxelState(int x, int y, int z, bool state);
	void InsertVoxelItem(int x, int y, int z, DrawItem& item);
	bool GetVoxelState(int x, int y, int z);
	void GetGridSegments(int& x, int& y, int& z);

	void initializeRayMarch(MarchingInfo& mi, const Ray& r, float tmin) const;
	int GetVoxelItemNum(int x, int y, int z);

	void InsertPlane(DrawItem& plane);

private:
	Voxel* VoxelS;
	BoundingBox* mBoundingBox;
	int mX, mY, mZ;
	float startX, startY, startZ;
	float stepX, stepY, stepZ;

	std::vector<DrawItem> planes;

	void addVoxel(int& x, int& y, int& z, Material* mat);
	void addVoxelSurface(int& x, int& y, int& z, int surface, Material* mat);
	void paintVoxel(int& x, int& y, int& z);
};