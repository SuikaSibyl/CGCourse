#pragma once

#include "Spline.h"
#include "TriangleMesh.h"

class Surface :public Spline
{
public:
	virtual void set(int i, const Vec3f v)
	{

	}
	// FOR GENERATING TRIANGLES
	virtual TriangleMesh* OutputTriangles(ArgParser* args) { return nullptr; }
};

class SurfaceOfRevolution :public Surface
{
public:
	SurfaceOfRevolution() = default;
	SurfaceOfRevolution(Curve* curve)
		:curve(curve)
	{

	}
	// FOR VISUALIZATION
	virtual void Paint(ArgParser* args)
	{
		curve->Paint(args);
	}

	// FOR CONTROL POINT PICKING
	virtual int getNumVertices() { return curve->getNumVertices(); }
	virtual Vec3f getVertex(int i) { return curve->getVertex(i); }
	// FOR EDITING OPERATIONS
	virtual void moveControlPoint(int selectedPoint, float x, float y)
	{
		curve->moveControlPoint(selectedPoint, x, y);
	}
	virtual void addControlPoint(int selectedPoint, float x, float y)
	{
		curve->addControlPoint(selectedPoint, x, y);
	}
	virtual void OutputBSpline(FILE* file)
	{
		fprintf(file, "\surface_of_revolution");
		curve->OutputBSpline(file);
	}

	virtual void deleteControlPoint(int selectedPoint)
	{
		curve->deleteControlPoint(selectedPoint);
	}
	virtual TriangleMesh* OutputTriangles(ArgParser* args)
	{
		int n = curve->GetN();
		int edge_vertices = args->curve_tessellation * n / 3 + 1;
		int total_vertices = args->revolution_tessellation * edge_vertices;
		int square_num = args->revolution_tessellation * (edge_vertices - 1);
		int triangle_num = square_num * 2;

		TriangleMesh* triangleMesh = new TriangleMesh(total_vertices, triangle_num);

		Vec3f* points = new Vec3f[edge_vertices];
		float step = 1. / (edge_vertices - 1);
		for (int i = 0; i < edge_vertices; i++)
		{
			float alpha = step * i;
			points[i] = curve->GetInterpolation(alpha);
		}

		int triangle_idx = 0;
		float pistep = 2 * 3.1415926 / args->revolution_tessellation;
		for (int revolution = 0; revolution < args->revolution_tessellation; revolution++)
		{
			float theta = pistep * revolution;
			float costheta = cos(theta);
			float sintheta = sin(theta);
			// Insert the edge vertices
			for (int i = 0; i < edge_vertices; i++)
			{
				float alpha = step * i;
				triangleMesh->SetVertex(
					edge_vertices * revolution + i,
					Vec3f(points[i].x() * costheta, points[i].y(), points[i].x() * sintheta));
			}

			if (revolution != 0)
			{
				for (int i = 1; i < edge_vertices; i++)
				{
					int points[] =
					{
						edge_vertices * (revolution - 1) + i - 1,
						edge_vertices * (revolution - 1) + i,
						edge_vertices * revolution + i - 1,
						edge_vertices * revolution + i,
					};
					triangleMesh->SetTriangle(triangle_idx++, points[1], points[0], points[2]);
					triangleMesh->SetTriangle(triangle_idx++, points[1], points[2], points[3]);
				}
			}
			if (revolution == args->revolution_tessellation - 1)
			{
				for (int i = 1; i < edge_vertices; i++)
				{
					int points[] =
					{
						edge_vertices * revolution + i - 1,
						edge_vertices * revolution + i,
						edge_vertices * 0 + i - 1,
						edge_vertices * 0 + i,
					};
					triangleMesh->SetTriangle(triangle_idx++, points[1], points[0], points[2]);
					triangleMesh->SetTriangle(triangle_idx++, points[1], points[2], points[3]);
				}
			}
		}
		//triangleMesh.
		return triangleMesh;
	}

private:
	Curve* curve;
};

class BezierPatch :public Surface
{
public:
	BezierPatch()
	{
		for (int i = 0; i < 4; i++)
		{
			Vertices.push_back(std::vector<Vec3f>(4));
			curve[i] = new BezierCurve(4);
		}
	}

	virtual void set(int i, const Vec3f v)
	{
		int x = i / 4;
		int y = i % 4;
		Vertices[x][y] = v;
		curve[x]->set(y, v);
	}

	// FOR VISUALIZATION
	virtual void Paint(ArgParser* args)
	{
		for (int i = 0; i < 4; i++)
			curve[i]->Paint(args);
	}

	Vec3f GetInterpolation(float u, float v)
	{
		Vec3f interpolated[4];
		BezierCurve interpocurve(4);
		for (int i = 0; i < 4; i++)
		{
			interpolated[i] = curve[i]->GetInterpolation(u);
			interpocurve.set(i, interpolated[i]);
		}
		return interpocurve.GetInterpolation(v);
	}

	int edge_point_num;
	int GetIndex(int i, int j)
	{
		return i * edge_point_num + j;
	}

	virtual TriangleMesh* OutputTriangles(ArgParser* args)
	{
		int tesselation = args->patch_tessellation;
		int edge_point_num = tesselation + 1;
		int point_num = edge_point_num * edge_point_num;
		int triangle_num = tesselation * tesselation * 2;

		TriangleMesh* triangleMesh = new TriangleMesh(point_num, triangle_num);
		float step = 1. / tesselation;
		
		this->edge_point_num = edge_point_num;
		for (int i = 0; i < edge_point_num; i++)
			for (int j = 0; j < edge_point_num; j++)
			{
				triangleMesh->SetVertex(i * edge_point_num + j, GetInterpolation(i * step, j * step));
			}

		int triangleIdx = 0;
		for (int i = 0; i < tesselation; i++)
			for (int j = 0; j < tesselation; j++)
			{
				triangleMesh->SetTriangle(triangleIdx++, GetIndex(i, j), GetIndex(i, j + 1), GetIndex(i + 1, j));
				triangleMesh->SetTriangle(triangleIdx++, GetIndex(i + 1, j), GetIndex(i, j + 1), GetIndex(i + 1, j + 1));
			}

		return triangleMesh;
	}

private:
	std::vector<std::vector<Vec3f>> Vertices;
	BezierCurve* curve[4];
};